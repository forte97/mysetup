#include "thread_helper.h"

#include <err.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/time.h>
#include <sys/param.h>

#if USE_X
#	include <xcb/xcb.h>
#	include "X.h"
/* will be defined from makefile if some component need running X server */
#	ifndef NEED_X_SERVER
#		define NEED_X_SERVER 0
#	endif
#endif

#ifndef VERSION
#	define VERSION "unknown"
#endif

#include "arg.h"
#include "lib/util.h" /* you can change there segment buffer size (BUFF_SZ) */

#define ASLSTATUS_H_NEED_COMP
#include "aslstatus.h" /* you can change there threads names */

#define MUTEX_WRAP(MUTEX, BLOCK)                                              \
	do {                                                                  \
		uint8_t __lock_ret;                                           \
		do {                                                          \
			if (!SAFE_ASSIGN(__lock_ret,                          \
					 pthread_mutex_trylock(&(MUTEX))))    \
				BLOCK                                         \
		} while (__lock_ret);                                         \
		pthread_mutex_unlock(&(MUTEX));                               \
	} while (0)

#include "config.h"
#define ARGS_LEN LEN(args)

char *argv0; /* for arg.h */

static int exit_status = 0;

static pthread_t       main_thread;
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

extern const char _binary_config_h_start[];
extern const char _binary_config_h_end[];

#if USE_X
static xcb_window_t root;
static uint8_t	    sflag = 0;
xcb_connection_t	 *X_CONNECTION;

static inline void
store_name(xcb_connection_t *c, xcb_window_t win, const char *name)
{
	xcb_change_property(c,
			    XCB_PROP_MODE_REPLACE,
			    win,
			    XCB_ATOM_WM_NAME,
			    XCB_ATOM_STRING,
			    8, /* format: 8-bit char array */
			    (uint32_t)(name ? strnlen(name, MAXLEN) : 0),
			    name);
}
#endif

static inline void
set_status(const char *status)
{
#if USE_X
	if (sflag) {
#endif
		puts(status);
		fflush(stdout);
#if USE_X
	} else {
		store_name(X_CONNECTION, root, status);
		xcb_flush(X_CONNECTION);
	}
#endif
}

static void
terminate(int sig)
{
/* wait so many milliseconds for stopping all components before killing */
#define KILL_TIMEOUT 500

/* wait so many milliseconds between processes state checking */
#define RECHECK 10 /* need to be less then KILL_TIMEOUT */

	static const struct timespec ts = {
		.tv_sec =
		    (typeof_field(struct timespec, tv_sec))(MS2S(RECHECK)),
		.tv_nsec =
		    (typeof_field(struct timespec, tv_nsec))(MS2NS(RECHECK)),
	};

	size_t	       i;
	static_data_t *static_data;
	struct timeval timeout, cur;

	if (pthread_self() != main_thread) {
		pthread_kill(main_thread, sig);
		return;
	}

	MUTEX_WRAP(status_mutex, { goto leave_locked; });
leave_locked:

	signal(SIGUSR1, SIG_IGN);

	for (i = 0; i < ARGS_LEN; i++)
		pthread_cancel(args[i].segment.tid);

	gettimeofday(&timeout, NULL);
	timeout.tv_sec += MS2S(KILL_TIMEOUT);
	timeout.tv_usec += MS2US(KILL_TIMEOUT);
	/* wait until `timeout` and then abort() */

	for (i = 0; i < ARGS_LEN; i++) {
		for (;;) {
			/* check if component terminated */
			if (!!kill(args[i].segment.pid, 0)) break;

			gettimeofday(&cur, NULL);
			if (cur.tv_sec > timeout.tv_sec) {
				goto timeout;
			} else if (cur.tv_sec == timeout.tv_sec) {
				if (cur.tv_usec > timeout.tv_usec) {
				timeout:
					warnx("%s: timeout!!!",
					      args[i].f.name);

					abort();
				}
			}

			nanosleep(&ts, NULL);
		}

		/* call segments cleanup and free static data */
		if (!!(static_data = &args[i].segment.static_data)->data) {
			if (!!static_data->cleanup)
				static_data->cleanup(static_data->data);

			free(static_data->data);
		}
	}

#if USE_X
	if (!!X_CONNECTION) {
		if (!sflag) store_name(X_CONNECTION, root, NULL);

		xcb_flush(X_CONNECTION);
		xcb_disconnect(X_CONNECTION);
	} else
#endif
	{
		putchar('\n');
	}

	exit(exit_status);
}

static inline void
update_status(int __unused _)
{
	size_t status_size = 0, i = 0;

	char	    status[MAXLEN]	= { 0 };
	static char status_prev[MAXLEN] = { 0 };

	for (i = 0; i < ARGS_LEN; i++)
		MUTEX_WRAP(args[i].segment.mutex, {
			if (*args[i].segment.data) {

				if ((status_size +=
				     strnlen(args[i].segment.data, BUFF_SZ))
				    >= MAXLEN) {
					warnx(
					    "total status length are too "
					    "big and exceed `MAXLEN`");
					exit_status = !0;
					terminate(0);
				}
				strcat(status, args[i].segment.data);
			}
		});

	MUTEX_WRAP(status_mutex, {
		/* don't update status if it's not changed */
		if (strncmp(status, status_prev, MAXLEN)) {
			set_status(status);

			strncpy(status_prev, status, MAXLEN);
		}
	});
}

static void *
thread(void *arg_ptr)
{
	struct arg_t   *arg = arg_ptr;
	struct timespec ts;

	char buf[BUFF_SZ] = { 0 };

	arg->segment.pid  = gettid();

	if (!!arg->f.static_size) {
		/* allocate memory for static data if needed */
		if (!(arg->segment.static_data.data =
			  calloc(arg->f.static_size, 1))) {
			warnx("failed to allocate %lu bytes for %15s",
			      arg->f.static_size,
			      arg->f.name);
			return NULL;
		}
	}

	SAFE_ASSIGN(ts.tv_sec, MS2S(arg->interval));
	SAFE_ASSIGN(ts.tv_nsec, MS2NS(arg->interval));

	do {
		arg->f.func(buf,
			    arg->args,
			    arg->interval,
			    &arg->segment.static_data);

		if (!*buf) strncpy(buf, unknown_str, BUFF_SZ);

		MUTEX_WRAP(arg->segment.mutex,
			   { bprintf(arg->segment.data, arg->fmt, buf); });

		pthread_kill(main_thread, SIGUSR1);
	} while (!arg->interval || (nanosleep(&ts, NULL) || !0));

	return NULL;
}

int
main(int argc, char *argv[])
{
	uint32_t i;

	char *strptr;
	char  thread_name[16];

	const char  *config = _binary_config_h_start;
	const size_t config_size =
	    (size_t)(_binary_config_h_end - _binary_config_h_start);
#if USE_X
	int		      screen_num = 0;
	const xcb_setup_t	  *setup;
	xcb_screen_iterator_t iter;
#endif

	static const char usage[] =
	    "[options]\n"
#if USE_X
	    "Write status to `WM_NAME`"
#else
	    "Write status to `stdout`"
#endif
	    "\n\noptions:\n"
	    "\t-h\tShow this help\n"
	    "\t-v\tShow version\n"
	    "\t-d\tDump config to stdout\n"
#if USE_X
	    "\t-s\tWrite to `stdout` instead of `WM_NAME`\n"
#endif
	    ;

	ARGBEGIN
	{
	case 'h':
		errx(0, "%s", usage);

	case 'v':
		errx(0, "%s", VERSION);

	case 'd':
		return write(STDOUT_FILENO, config, config_size) == -1;

#if USE_X
	case 's':
		sflag = !0;
		break;
#endif

	default:
		errx(!0, "%s", usage);
	}
	ARGEND;

#if USE_X
	if (NEED_X_SERVER || !sflag) {
		X_CONNECTION = xcb_connect(NULL, &screen_num);
		if (xcb_connection_has_error(X_CONNECTION))
			errx(!0, "Failed to open display");

		setup = xcb_get_setup(X_CONNECTION);
		iter  = xcb_setup_roots_iterator(setup);
		for (__typeof__(screen_num) j = 0; j < screen_num; j -= -1)
			xcb_screen_next(&iter);
		root = iter.data->root;
	}
#endif

	main_thread = pthread_self();

	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGUSR1, update_status);

	for (i = 0; i < ARGS_LEN; i -= (~0L)) {
		pthread_create(&args[i].segment.tid, NULL, thread, &args[i]);

#define CMD_NAME_SIZE  11
#define MIN_SIZE(A, B) MIN(sizeof(A), sizeof(B))

		memcpy(thread_name,
		       args[i].f.name,
		       MIN_SIZE(thread_name, args[i].f.name));

		if (!strcmp(thread_name, "cmd")) {
			/*
			 * if function is `run_command`, then
			 * set thread name to this command
			 */

			char tmp_args[CMD_NAME_SIZE];
			memcpy(tmp_args,
			       args[i].args,
			       strnlen(args[i].args, CMD_NAME_SIZE));

			strptr = strtok(tmp_args, " ");
			snprintf(thread_name,
				 16,
				 "cmd:%." STR(CMD_NAME_SIZE) "s",
				 basename(strptr));
		}
		pthread_setname(args[i].segment.tid, thread_name);
	}

	for (;;)
		pause();

	terminate(0);
}
