/* See LICENSE file for copyright and license details. */
#include <sys/utsname.h>
#include <stdio.h>
#include <err.h>

#include "../lib/util.h"

void
kernel_release(char		    *out,
	       const char __unused *_a,
	       uint32_t __unused    _i,
	       void __unused	     *_p)
{
	struct utsname udata;

	if (uname(&udata) < 0) {
		warn("uname");
		ERRRET(out);
	}

	bprintf(out, "%s", udata.release);
}
