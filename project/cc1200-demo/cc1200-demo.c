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
#include "cpu.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "net/rime/broadcast.h"
#include "net/netstack.h"
#include "apps/deployment/deployment.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LOOP_PERIOD         1
#define LOOP_INTERVAL       ((CLOCK_SECOND * LOOP_PERIOD) / 4)
#define BROADCAST_CHANNEL   129
/*---------------------------------------------------------------------------*/
static struct etimer et;
static uint32_t counter;
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("App: received from %u seq %u rssi %d\n",
         node_id_from_linkaddr(from),
         (unsigned)*(uint32_t *)packetbuf_dataptr(),
         (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI)
        );
  leds_toggle(LEDS_GREEN);
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
PROCESS(cc1200_demo_process, "cc1200 demo process");
AUTOSTART_PROCESSES(&cc1200_demo_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc1200_demo_process, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&bc))
  PROCESS_BEGIN();

  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

  etimer_set(&et, LOOP_INTERVAL);

  /* Radio Tx mode: disable CCA */
  radio_value_t radio_tx_mode;
  NETSTACK_RADIO.get_value(RADIO_PARAM_TX_MODE, &radio_tx_mode);
  radio_tx_mode &= ~RADIO_TX_MODE_SEND_ON_CCA;
  NETSTACK_RADIO.set_value(RADIO_PARAM_TX_MODE, radio_tx_mode);
#undef CUSTOM_CHANNEL
#define CUSTOM_CHANNEL 0
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CUSTOM_CHANNEL);

  node_id = get_node_id();
  NETSTACK_MAC.on();

  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER) {
      if(node_id == 1) {
        printf("App: sending seq %u\n", (unsigned)counter);
        leds_toggle(LEDS_RED);
        packetbuf_copyfrom(&counter, sizeof(counter));
        broadcast_send(&bc);
        //NETSTACK_RADIO.send(&counter, 4);
        counter++;
      }
      etimer_set(&et, LOOP_INTERVAL);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */

