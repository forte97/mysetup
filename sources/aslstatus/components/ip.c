/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#if defined(__OpenBSD__)
#	include <sys/types.h>
#	include <sys/socket.h>
#elif defined(__FreeBSD__)
#	include <netinet/in.h>
#	include <sys/socket.h>
#endif

#include "../lib/util.h"

static inline void
ip(char *out, const char *interface, sa_family_t sa_family)
{
	int		s;
	struct ifaddrs *ifa, *ifaddr;
	char		host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) < 0) {
		warn("getifaddrs");
		ERRRET(out);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) { continue; }
		s = getnameinfo(ifa->ifa_addr,
				sizeof(struct sockaddr_in6),
				host,
				NI_MAXHOST,
				NULL,
				0,
				NI_NUMERICHOST);
		if (!strcmp(ifa->ifa_name, interface)
		    && (ifa->ifa_addr->sa_family == sa_family)) {
			freeifaddrs(ifaddr);
			if (s != 0) {
				warnx("getnameinfo: %s", gai_strerror(s));
				ERRRET(out);
			}
			bprintf(out, "%s", host);
			return;
		}
	}
	freeifaddrs(ifaddr);
	ERRRET(out);
}

void
ipv4(char *out, const char *interface, uint32_t __unused _i, void __unused *_p)
{
	ip(out, interface, AF_INET);
}

void
ipv6(char *out, const char *interface, uint32_t __unused _i, void __unused *_p)
{
	ip(out, interface, AF_INET6);
}
