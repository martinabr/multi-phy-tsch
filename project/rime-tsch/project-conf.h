/*
 * Copyright (c) 2016, Inria.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Project config file
 * \author
 *         Simon Duquennoy <simon.duquennoy@inria.fr>
 *
 */

#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#define WITH_MULTIRADIO 1
#define WITH_SINGLE_SENDER 0
#define WITH_SINGLE_CHANNEL 0

/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012

#undef TSCH_CONF_AUTOSELECT_TIME_SOURCE
#define TSCH_CONF_AUTOSELECT_TIME_SOURCE 0

/* TSCH logging. 0: disabled. 1: basic log. 2: with delayed
 * log messages from interrupt */
#undef TSCH_LOG_CONF_LEVEL
#define TSCH_LOG_CONF_LEVEL 2

#undef TSCH_LOG_CONF_ID_FROM_LINKADDR
#define TSCH_LOG_CONF_ID_FROM_LINKADDR node_id_from_linkaddr

/* IEEE802.15.4 PANID */
#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0xabcd

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 0

/* Needed for CC2538 platforms only */
/* For TSCH we have to use the more accurate crystal oscillator
 * by default the RC oscillator is activated */
#undef SYS_CTRL_CONF_OSC32K_USE_XTAL
#define SYS_CTRL_CONF_OSC32K_USE_XTAL 1

#undef TSCH_CONF_HW_FRAME_FILTERING
#define TSCH_CONF_HW_FRAME_FILTERING    0

#if WITH_MULTIRADIO       
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO multiradio_driver
#undef TSCH_CONF_SCANNING_RADIO
#define TSCH_CONF_SCANNING_RADIO cc1200_driver
#undef TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0 /* No 6TiSCH minimal schedule */

//#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_1_2kbps_sp /* sp: short packets */
//#define CC1200_CONF_RF_CFG cc1200_802154g_863_870_2gfsk_50kbps
#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_250kbps
//#define CC1200_CONF_RF_CFG cc1200_868_4gfsk_1000kbps

#else /* WITH_MULTIRADIO */

#undef TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0 /* No 6TiSCH minimal schedule */
#undef  NETSTACK_CONF_RADIO
#define NETSTACK_CONF_RADIO         cc1200_driver
#undef TSCH_CONF_SCANNING_RADIO
#define TSCH_CONF_SCANNING_RADIO NETSTACK_CONF_RADIO
//#define NETSTACK_CONF_RADIO         cc1200_driver
#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_1_2kbps_sp /* sp: short packets */
//#define CC1200_CONF_RF_CFG cc1200_802154g_863_870_2gfsk_50kbps
//#define CC1200_CONF_RF_CFG cc1200_868_4gfsk_1000kbps
#define CC1200_NO_HDR_CHECK         1
#endif /* WITH_MULTIRADIO */

#define TSCH_SCHEDULE_CONF_MAX_LINKS 60
#define TSCH_CONF_SYNC_WITH_LOWER_NODE_ID  1
#define CC1200_CONF_USE_GPIO2       1
#define CC1200_CONF_USE_RX_WATCHDOG 0
#define ANTENNA_SW_SELECT_DEF_CONF  ANTENNA_SW_SELECT_SUBGHZ
#define TSCH_CONF_HOPPING_SEQUENCE_MAX_LEN 34
#if WITH_SINGLE_CHANNEL
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE (uint8_t[]){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#else
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE (uint8_t[]){ 16, 17, 18, 19, 26, 15, 25, 22, 23, 11, 12, 13, 24, 14, 20, 21, 1, 32, 6, 5, 10, 28, 30, 27, 29, 8, 0, 4, 31, 3, 9, 7, 33, 2 }
#endif
#define TSCH_CONF_EB_PERIOD (4 * CLOCK_SECOND)
#define TSCH_CONF_MAX_EB_PERIOD (4 * CLOCK_SECOND)
#define TSCH_CONF_KEEPALIVE_TIMEOUT 0
#define TSCH_CONF_MAX_KEEPALIVE_TIMEOUT 0
#define TSCH_CONF_DESYNC_THRESHOLD (90 * CLOCK_SECOND)
#define TSCH_CONF_ADAPTIVE_TIMESYNC 1

#endif /* __PROJECT_CONF_H__ */
