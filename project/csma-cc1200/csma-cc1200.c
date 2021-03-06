/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-cc1200-demo Zoul on-board CC1200 RF transceiver test
 *
 * Demonstrates the use of the TI CC1200 RF transceiver on Sub-1GHz
 * @{
 *
 * \file
 *         Test file for the CC1200 demo
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "net/nullnet/nullnet.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "deployment/deployment.h"

#include "cc1200-const.h"
#include "cc1200-conf.h"
#include "cc1200-arch.h"
#include "cc1200-rf-cfg.h"

#include <stdio.h>
#include <stdint.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define LOOP_INTERVAL       (CLOCK_SECOND)

void cc1200_reconfigure(const cc1200_rf_cfg_t *config, uint8_t channel);
extern const cc1200_rf_cfg_t cc1200_868_4gfsk_1000kbps;
extern const cc1200_rf_cfg_t cc1200_868_2gfsk_50kbps_802154g;
extern const cc1200_rf_cfg_t cc1200_868_2gfsk_1_2kbps_sp;

/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
void
input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  LOG_INFO("Received seq %u from ", (unsigned)(*(uint32_t *)data));
  LOG_INFO_LLADDR(src);
  LOG_INFO_(" rssi %d\n",
    (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI)
  );
}
/*---------------------------------------------------------------------------*/
PROCESS(cc1200_demo_process, "cc1200 demo process");
AUTOSTART_PROCESSES(&cc1200_demo_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc1200_demo_process, ev, data)
{
  static uint32_t count = 0;
  radio_value_t radio_tx_mode;

  PROCESS_BEGIN();

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  /* Radio Tx mode: disable CCA */
  NETSTACK_RADIO.get_value(RADIO_PARAM_TX_MODE, &radio_tx_mode);
  radio_tx_mode &= ~RADIO_TX_MODE_SEND_ON_CCA;
  NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE, radio_tx_mode);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CUSTOM_CHANNEL);

static rtimer_clock_t t0, t1;
t0 = RTIMER_NOW();
  cc1200_reconfigure(&cc1200_868_4gfsk_1000kbps, 0);
t1 = RTIMER_NOW();
printf("Time delta: %lu %u\n", t1-t0, RTIMER_SECOND);

  NETSTACK_RADIO.on();

  if(node_id == 1) {
    etimer_set(&et, LOOP_INTERVAL);
    while(1) {
      PROCESS_WAIT_UNTIL(etimer_expired(&et));
      LOG_INFO("Sending seq %u\n", (unsigned)count);
      NETSTACK_NETWORK.output(NULL);
      count++;
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
