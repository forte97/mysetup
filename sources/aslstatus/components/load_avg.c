/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lib/util.h"

void
load_avg(char		      *out,
	 const char __unused *_a,
	 uint32_t __unused    _i,
	 void __unused       *_p)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		warnx("getloadavg: Failed to obtain load average");
		ERRRET(out);
	}

	bprintf(out, "%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}
