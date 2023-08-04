/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "../lib/util.h"

#if defined(CLOCK_BOOTTIME)
#	define UPTIME_FLAG	CLOCK_BOOTTIME
#	define STR_UPTIME_FLAG "CLOCK_BOOTTIME"
#elif defined(CLOCK_UPTIME)
#	define UPTIME_FLAG	CLOCK_UPTIME
#	define STR_UPTIME_FLAG "CLOCK_UPTIME"
#else
#	define UPTIME_FLAG	CLOCK_MONOTONIC
#	define STR_UPTIME_FLAG "CLOCK_MONOTONIC"
#endif

void
uptime(char		    *out,
       const char __unused *_a,
       uint32_t __unused    _i,
       void __unused	     *_p)
{
	uintmax_t	h, m;
	struct timespec uptime;

	if (clock_gettime(UPTIME_FLAG, &uptime) < 0) {
		warnx("clock_gettime(%s)", STR_UPTIME_FLAG);
		ERRRET(out);
	}

	SAFE_ASSIGN(h, uptime.tv_sec / 3600);
	SAFE_ASSIGN(m, uptime.tv_sec % 3600 / 60);

	bprintf(out, "%juh %jum", h, m);
}
