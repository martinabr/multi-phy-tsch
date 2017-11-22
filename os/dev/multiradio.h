#ifndef __MULTIRADIO_H__
#define __MULTIRADIO_H__

#include "dev/radio.h"

extern const struct radio_driver cc1200_driver;
extern const struct radio_driver cc2538_rf_driver;

#ifdef MULTIRADIO_CONF_DRIVERS
#define MULTIRADIO_DRIVERS MULTIRADIO_CONF_DRIVERS
#else /* MULTIRADIO_CONF_RADIOS */
#define MULTIRADIO_DRIVERS { &cc2538_rf_driver, &cc1200_driver }
#endif /* MULTIRADIO_CONF_RADIOS */

extern const struct radio_driver multiradio_driver;
int multiradio_select(const struct radio_driver *radio);

#endif /* __MULTIRADIO_H__ */
