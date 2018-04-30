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

/* Logging */
#define LOG_CONF_LEVEL_MAC LOG_LEVEL_WARN
#define TSCH_LOG_CONF_PER_SLOT 1
#define LOG_CONF_WITH_COMPACT_ADDR 1

/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0xdf10

/* Use multiradio: both cc1200 and cc2538 */
//#define NETSTACK_CONF_RADIO cc1200_driver
#define NETSTACK_CONF_RADIO         cc1200_driver
#define TSCH_CONF_SCANNING_RADIO      cc1200_driver /* used with CC1200_CONF_RF_CFG */
#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_1_2kbps
//#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_1_2kbps_sp
//#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_8kbps
//#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_50kbps_802154g
//#define CC1200_CONF_RF_CFG cc1200_868_2gfsk_250kbps
//#define CC1200_CONF_RF_CFG cc1200_868_4gfsk_1000kbps
#define TSCH_CONF_SCANNING_CC1200_CFG CC1200_CONF_RF_CFG /* Because few channels */
#define TSCH_CONF_DEFAULT_TIMING CC1200_CONF_RF_CFG.tsch_timing
#define ANTENNA_SW_SELECT_DEF_CONF  ANTENNA_SW_SELECT_SUBGHZ

/* Make room in timeslot template for radio reconfig */
//#define TSCH_WITH_CC1200_RECONF 1600
#define TSCH_WITH_CC1200_RECONF 0
/* Tell TSCH to use multiple radios */
#define TSCH_WITH_MULTIRADIO 1
/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#define TSCH_CONF_AUTOSTART 0
/* No 6TiSCH minimal schedule */
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 1
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH      1
/* We will need more than 32 links */
#define TSCH_SCHEDULE_CONF_MAX_LINKS 180
#define TSCH_SCHEDULE_CONF_MAX_SLOTFRAMES 6
#define TSCH_CONF_SYNC_WITH_LOWER_NODE_ID 1
#define EB_ONLY_COORDINATOR 1

/* EB and KA */
#define TSCH_CONF_EB_PERIOD (8 * CLOCK_SECOND)
#define TSCH_CONF_MAX_EB_PERIOD TSCH_CONF_EB_PERIOD
#define TSCH_CONF_KEEPALIVE_TIMEOUT 0
#define TSCH_CONF_MAX_KEEPALIVE_TIMEOUT TSCH_CONF_KEEPALIVE_TIMEOUT
#define TSCH_CONF_DESYNC_THRESHOLD (90 * CLOCK_SECOND)

/* Use no hopping sequence */
#define TSCH_CONF_NO_HOPPING_SEQUENCE 1
//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE (uint8_t[]){ 0, 1 }

//#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE (uint8_t[]){
  //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
  // 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  // 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  // 30, 31, 32, 33 }

#endif /* __PROJECT_CONF_H__ */
