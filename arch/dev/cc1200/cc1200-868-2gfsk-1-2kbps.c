/*
 * Copyright (c) 2015, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

#include "cc1200-rf-cfg.h"
#include "cc1200-const.h"
#include "net/mac/tsch/tsch-private.h"

/*
 * This is a setup for the following configuration:
* From smartRf: 50kbps setting (868 MHz) but set at 1.2kbps.
 */

 /* Base frequency in kHz */
 #define RF_CFG_CHAN_CENTER_F0           863125
 /* Channel spacing in Hz */
 #define RF_CFG_CHAN_SPACING             200000
/* The minimum channel */
#define RF_CFG_MIN_CHANNEL              0
/* The maximum channel */
#define RF_CFG_MAX_CHANNEL              33
/* The maximum output power in dBm */
#define RF_CFG_MAX_TXPOWER              CC1200_CONST_TX_POWER_MAX
/* The carrier sense level used for CCA in dBm */
#define RF_CFG_CCA_THRESHOLD            (-91)
/* The RSSI offset in dBm */
#define RF_CFG_RSSI_OFFSET              (-99)
/*---------------------------------------------------------------------------*/
static const char rf_cfg_descriptor[] = "868MHz 2-FSK 1.2 kbps";
/*---------------------------------------------------------------------------*/

/* 1 byte time: 6667 usec */
#define CC1200_TSCH_PREAMBLE_LENGTH             33335 /* 3 bytes + 2 SFD */
#define CC1200_TSCH_CONF_RX_WAIT                 1000
#define CC1200_TSCH_CONF_RX_ACK_WAIT              150

#define CC1200_TSCH_DEFAULT_TS_CCA_OFFSET        1800
#define CC1200_TSCH_DEFAULT_TS_CCA                128
#define CC1200_TSCH_DEFAULT_TS_TX_OFFSET        50000 /* Must be greater than preamble */
#define CC1200_TSCH_DEFAULT_TS_RX_OFFSET          (CC1200_TSCH_DEFAULT_TS_TX_OFFSET - CC1200_TSCH_PREAMBLE_LENGTH - (CC1200_TSCH_CONF_RX_WAIT / 2))
#define CC1200_TSCH_DEFAULT_TS_RX_ACK_DELAY       (CC1200_TSCH_DEFAULT_TS_TX_ACK_DELAY - CC1200_TSCH_PREAMBLE_LENGTH - (CC1200_TSCH_CONF_RX_ACK_WAIT / 2))
#define CC1200_TSCH_DEFAULT_TS_TX_ACK_DELAY     50000 /* Must be greater than preamble */
#define CC1200_TSCH_DEFAULT_TS_RX_WAIT            (CC1200_TSCH_PREAMBLE_LENGTH + CC1200_TSCH_CONF_RX_WAIT)
#define CC1200_TSCH_DEFAULT_TS_ACK_WAIT           (CC1200_TSCH_PREAMBLE_LENGTH + CC1200_TSCH_CONF_RX_ACK_WAIT)
#define CC1200_TSCH_DEFAULT_TS_RX_TX              192
#define CC1200_TSCH_DEFAULT_TS_MAX_ACK          73334 /* 7+1+3 bytes at 50 kbps */
#define CC1200_TSCH_DEFAULT_TS_MAX_TX          866667 /* 126+1+3 bytes at 50 kbps */
/* TSCH_DEFAULT_TS_TX_OFFSET + TSCH_DEFAULT_TS_MAX_TX + TSCH_DEFAULT_TS_TX_ACK_DELAY + TSCH_DEFAULT_TS_MAX_ACK + 550 usec slack */
//#define CC1200_TSCH_DEFAULT_TS_TIMESLOT_LENGTH    1040551
#define CC1200_TSCH_DEFAULT_TS_TIMESLOT_LENGTH     1050000

/* TSCH timeslot timing (in rtimer ticks) */
static rtimer_clock_t cc1200_1_2kbps_tsch_timing[tsch_ts_elements_count] = {
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_CCA_OFFSET),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_CCA),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_TX_OFFSET),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_RX_OFFSET),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_RX_ACK_DELAY),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_TX_ACK_DELAY),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_RX_WAIT),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_ACK_WAIT),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_RX_TX),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_MAX_ACK),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_MAX_TX),
  US_TO_RTIMERTICKS_64(CC1200_TSCH_DEFAULT_TS_TIMESLOT_LENGTH),
};

 static const registerSetting_t preferredSettings[]= 
 {
   {CC1200_IOCFG2,            0x06},
   {CC1200_SYNC3,             0x6F},
   {CC1200_SYNC2,             0x4E},
   {CC1200_SYNC1,             0x90},
   {CC1200_SYNC0,             0x4E},
   {CC1200_SYNC_CFG1,         0xE5},
   {CC1200_SYNC_CFG0,         0x23},
   {CC1200_DEVIATION_M,       0x47},
   {CC1200_MODCFG_DEV_E,      0x0B},
   {CC1200_DCFILT_CFG,        0x56},
   {CC1200_PREAMBLE_CFG0,     0xBA},
   {CC1200_IQIC,              0xC8},
   {CC1200_CHAN_BW,           0x84},
   {CC1200_MDMCFG1,           0x42},
   {CC1200_MDMCFG0,           0x05},
   {CC1200_SYMBOL_RATE2,      0x3F},
   {CC1200_SYMBOL_RATE1,      0x75},
   {CC1200_SYMBOL_RATE0,      0x10},
   {CC1200_AGC_REF,           0x27},
   {CC1200_AGC_CS_THR,        0xF1},
   {CC1200_AGC_CFG1,          0x11},
   {CC1200_AGC_CFG0,          0x90},
   {CC1200_FIFO_CFG,          0x00},
   {CC1200_FS_CFG,            0x12},
   {CC1200_PKT_CFG2,          0x24},
   {CC1200_PKT_CFG0,          0x20},
   {CC1200_PKT_LEN,           0xFF},
   {CC1200_IF_MIX_CFG,        0x18},
   {CC1200_TOC_CFG,           0x03},
   {CC1200_MDMCFG2,           0x02},
   {CC1200_FREQ2,             0x56},
   {CC1200_FREQ1,             0xCC},
   {CC1200_FREQ0,             0xCC},
   {CC1200_IF_ADC1,           0xEE},
   {CC1200_IF_ADC0,           0x10},
   {CC1200_FS_DIG1,           0x04},
   {CC1200_FS_DIG0,           0x50},
   {CC1200_FS_CAL1,           0x40},
   {CC1200_FS_CAL0,           0x0E},
   {CC1200_FS_DIVTWO,         0x03},
   {CC1200_FS_DSM0,           0x33},
   {CC1200_FS_DVC1,           0xF7},
   {CC1200_FS_DVC0,           0x0F},
   {CC1200_FS_PFD,            0x00},
   {CC1200_FS_PRE,            0x6E},
   {CC1200_FS_REG_DIV_CML,    0x1C},
   {CC1200_FS_SPARE,          0xAC},
   {CC1200_FS_VCO0,           0xB5},
   {CC1200_IFAMP,             0x05},
   {CC1200_XOSC5,             0x0E},
   {CC1200_XOSC1,             0x03},
 };
/*---------------------------------------------------------------------------*/
/* Global linkage: symbol name must be different in each exported file! */
const cc1200_rf_cfg_t cc1200_868_2gfsk_1_2kbps = {
  .cfg_descriptor = rf_cfg_descriptor,
  .register_settings = preferredSettings,
  .size_of_register_settings = sizeof(preferredSettings),
  .tx_pkt_lifetime = (RTIMER_SECOND),
  .tx_rx_turnaround = (RTIMER_SECOND / 100),
  /* Includes 3 Bytes preamble + 2 Bytes SFD, at 6667usec per byte = 33335 usec */
  .delay_before_tx = ((unsigned)US_TO_RTIMERTICKS(37260)),
  .chan_center_freq0 = RF_CFG_CHAN_CENTER_F0,
  .chan_spacing = RF_CFG_CHAN_SPACING,
  .min_channel = RF_CFG_MIN_CHANNEL,
  .max_channel = RF_CFG_MAX_CHANNEL,
  .max_txpower = RF_CFG_MAX_TXPOWER,
  .cca_threshold = RF_CFG_CCA_THRESHOLD,
  .rssi_offset = RF_CFG_RSSI_OFFSET,
  .bitrate = 1200,
  .tsch_timing = cc1200_1_2kbps_tsch_timing,
};
/*---------------------------------------------------------------------------*/
