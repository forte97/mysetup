#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h> /* MIN */
#include <sys/sysinfo.h>

#include "../lib/util.h"
#include "../aslstatus.h"
#include "../lib/meminfo.h"

#define DEF_RAM(STRUCT, STATIC, OUT)                                          \
	int*		   fd	  = (STATIC)->data;                           \
	struct meminfo_ram STRUCT = MEMINFO_INIT_RAM;                         \
	if (!(STATIC)->cleanup) (STATIC)->cleanup = fd_cleanup;               \
	if (!MEMINFO_FD(fd)) ERRRET(OUT);                                     \
	if (!get_meminfo_ram(*fd, &STRUCT)) ERRRET(OUT)

static inline memory_t get_used(const struct meminfo_ram* info);

void
ram_free(char*		      out,
	 const char __unused* _a,
	 uint32_t __unused    _i,
	 static_data_t*	      static_data)
{
	DEF_RAM(info, static_data, out);

	fmt_human(
	    out,
	    (info.available ? MIN(info.available, info.total) : info.free)
		* 1024);
}

void
ram_perc(char*		      out,
	 const char __unused* _a,
	 uint32_t __unused    _i,
	 static_data_t*	      static_data)
{
	DEF_RAM(info, static_data, out);

	if (!info.total) ERRRET(out);

	bprintf(out,
		"%" PRIperc,
		(percent_t)(100 * get_used(&info) / info.total));
}

void
ram_total(char*		       out,
	  const char __unused* _a,
	  uint32_t __unused    _i,
	  void __unused*       _p)
{
	struct sysinfo info;
	if (!!sysinfo(&info)) ERRRET(out);

	fmt_human(out, info.totalram * info.mem_unit);
}

void
ram_used(char*		      out,
	 const char __unused* _a,
	 uint32_t __unused    _i,
	 static_data_t*	      static_data)
{
	DEF_RAM(info, static_data, out);

	fmt_human(out, get_used(&info) * 1024);
}

static inline memory_t
get_used(const struct meminfo_ram* info)
{
	/*
	 * see procps(free):
	 * https://gitlab.com/procps-ng/procps/-/blob/master/proc/sysinfo.c
	 *
	 * and htop:
	 * https://github.com/htop-dev/htop/blob/master/linux/LinuxProcessList.c
	 */

	const memory_t diff =
	    info->free
	    + (info->cached + info->reclaimable - info->shared
	       /*
		* Adjustments:
		*  - Shmem in part of Cached
		*  https://lore.kernel.org/patchwork/patch/648763/
		*/
	       )
	    + info->buffers;

	return (info->total >= diff) ? info->total - diff
				     : info->total - info->free;
}
