#ifndef _ASLSTATUS_H
#define _ASLSTATUS_H

#include <stdio.h>    /* FILE */
#include <pthread.h>  /* PTHREAD_MUTEX_INITIALIZER */
#include <inttypes.h> /* uintmax_t in cpu_perc */

#if USE_X
#	include <xcb/xcb.h>
#endif

#include "lib/util.h"

#include "components/cpu.h"
#include "components/wifi.h"
#include "components/volume.h"
#include "components/battery.h"
#include "components/netspeed.h"
#include "components/brightness.h"

#define FUNC_ARGS (char *, const char *, uint32_t, static_data_t *)

#define END                                                                   \
	{                                                                     \
		.pid = -1, .tid = 0, .data = { 0 },                           \
		.static_data = { .cleanup = NULL, .data = NULL },             \
		.mutex	     = PTHREAD_MUTEX_INITIALIZER                      \
	}

typedef void (*cleanup_func_t)(void *);

typedef struct static_data_t {
	cleanup_func_t cleanup;
	void	     *data;
} static_data_t;

typedef void(*func_t) FUNC_ARGS;

typedef struct func_data_t {
	func_t	   func;
	const char name[16];
	size_t	   static_size;
} func_data_t;

struct segment_t {
	pid_t		pid;
	pthread_t	tid;
	char		data[BUFF_SZ];
	static_data_t	static_data;
	pthread_mutex_t mutex;
};

struct arg_t {
	const func_data_t f;
	const char	   *fmt;
	const char	   *args;
	const uint32_t	  interval;
	struct segment_t  segment;
};

#ifdef ASLSTATUS_H_NEED_COMP
/* clang-format off */

/* battery */
void battery_perc FUNC_ARGS;
#define battery_perc {battery_perc, "batt_percentage", sizeof(int)}

void battery_state FUNC_ARGS;
#define battery_state {battery_state, "batt_state", sizeof(int)}

void battery_remaining FUNC_ARGS;
#define battery_remaining \
	{battery_remaining, "batt_remaining", sizeof(struct remaining)}

/* brightness */
void brightness FUNC_ARGS;
#define brightness {brightness, "brightness", sizeof(struct brightness_data)}

#if USE_X
/* bspwm */
void bspwm_ws FUNC_ARGS;
#define bspwm_ws {bspwm_ws, "bspwm", sizeof(int)}
#endif


/* cpu */
void cpu_freq FUNC_ARGS;
#define cpu_freq {cpu_freq, "cpu_freq", sizeof(int)}

void cpu_perc FUNC_ARGS;
#define cpu_perc {cpu_perc, "cpu_percentage", sizeof(struct cpu_data_t)}

/* datetime */
void datetime FUNC_ARGS;
#define datetime {datetime, "datetime", 0}


/* disk */
void disk_free FUNC_ARGS;
#define disk_free {disk_free, "disk_free", 0}

void disk_perc FUNC_ARGS;
#define disk_perc {disk_perc, "disk_percentage", 0}

void disk_total FUNC_ARGS;
#define disk_total {disk_total, "disk_total", 0}

void disk_used FUNC_ARGS;
#define disk_used {disk_used, "disk_used", 0}


/* entropy */
void entropy FUNC_ARGS;
#define entropy {entropy, "entropy", sizeof(int)}


/* hostname */
void hostname FUNC_ARGS;
#define hostname {hostname, "hostname", 0}


/* ip */
void ipv4 FUNC_ARGS;
#define ipv4 {ipv4, "ipv4", 0}

void ipv6 FUNC_ARGS;
#define ipv6 {ipv6, "ipv6", 0}


/* kernel_release */
void kernel_release FUNC_ARGS;
#define kernel_release {kernel_release, "kernel_release", 0}


#if USE_X && USE_XKB
/* keymap */
void keymap FUNC_ARGS;
#define keymap {keymap, "keymap", sizeof(char)}
#endif


/* load_avg */
void load_avg FUNC_ARGS;
#define load_avg {load_avg, "load_avg", 0}


/* netspeeds */
void netspeed_rx FUNC_ARGS;
#define netspeed_rx {netspeed_rx, "netspeed_rx", sizeof(struct netspeed_data)}

void netspeed_tx FUNC_ARGS;
#define netspeed_tx {netspeed_tx, "netspeed_tx", sizeof(struct netspeed_data)}


/* num_files */
void num_files FUNC_ARGS;
#define num_files {num_files, "num_files", 0}


/* ram */
void ram_free FUNC_ARGS;
#define ram_free {ram_free, "ram_free", sizeof(int)}

void ram_perc FUNC_ARGS;
#define ram_perc {ram_perc, "ram_percentage", sizeof(int)}

void ram_total FUNC_ARGS;
#define ram_total {ram_total, "ram_total", 0}

void ram_used FUNC_ARGS;
#define ram_used {ram_used, "ram_used", sizeof(int)}


/* run_command */
void run_command FUNC_ARGS;
#define run_command {run_command, "cmd", sizeof(pid_t)}
/*
 * `cmd` thread name hardcoded to be used to run shell commands
 */


/* swap */
void swap_free FUNC_ARGS;
#define swap_free {swap_free, "swap_free", sizeof(int)}

void swap_perc FUNC_ARGS;
#define swap_perc {swap_perc, "swap_percentage", sizeof(int)}

void swap_total FUNC_ARGS;
#define swap_total {swap_total, "swap_total", 0}

void swap_used FUNC_ARGS;
#define swap_used {swap_used, "swap_used", sizeof(int)}


/* temperature */
void temp FUNC_ARGS;
#define temp {temp, "temperature", sizeof(int)}


/* uptime */
void uptime FUNC_ARGS;
#define uptime {uptime, "uptime", 0}


/* user */
void gid FUNC_ARGS;
#define gid {gid, "gid", 0}

void uid FUNC_ARGS;
#define uid {uid, "uid", 0}

void username FUNC_ARGS;
#define username {username, "username", 0}


/* volume */
void vol_perc FUNC_ARGS;
#define vol_perc {vol_perc, "volume", VOLUME_STATIC_SIZE}


/* wifi */
void wifi_perc FUNC_ARGS;
#define wifi_perc {wifi_perc, "wifi_percentage", sizeof(struct wifi_perc_data)}

void wifi_essid FUNC_ARGS;
#define wifi_essid {wifi_essid, "wifi_essid", sizeof(struct wifi_essid_data)}

/* clang-format on */
#endif /* ASLSTATUS_H_NO_COMP */

#endif /* _ASLSTATUS_H */
