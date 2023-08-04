#ifndef BATTERY_H
#define BATTERY_H

#include "../components_config.h"

#ifndef BATTERY_FULL
#	define BATTERY_FULL "" /* "f" */
#endif

#ifndef BATTERY_UNKNOWN
#	define BATTERY_UNKNOWN "" /* "?" */
#endif

#ifndef BATTERY_CHARGING
#	define BATTERY_CHARGING "+" /* "+" */
#endif

#ifndef BATTERY_DISCHARGING
#	define BATTERY_DISCHARGING "-" /* "-" */
#endif

#ifndef BATTERY_REMAINING_NOT_DISCHARGING
#	define BATTERY_REMAINING_NOT_DISCHARGING "?"
#endif

struct remaining {
	int status, charge, current;
};

#endif /* BATTERY_H */
