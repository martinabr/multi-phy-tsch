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
#include "dev/multiradio.h"
#include "net/rime/broadcast.h"
#include "net/netstack.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LOOP_PERIOD         2
#define LOOP_INTERVAL       (CLOCK_SECOND * LOOP_PERIOD)
#define BROADCAST_CHANNEL   129
/*---------------------------------------------------------------------------*/
static struct etimer et;
static uint32_t counter;
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%u'\n",
         from->u8[0], from->u8[1], (unsigned)*(uint32_t *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  etimer_set(&et, LOOP_INTERVAL);
/*
  if(linkaddr_node_addr.u8[1] % 2) {
    multiradio_select(&cc2538_rf_driver);
    NETSTACK_RADIO.off();
    multiradio_select(&cc1200_driver);
    NETSTACK_RADIO.on();
  } else {
    multiradio_select(&cc1200_driver);
    NETSTACK_RADIO.off();
    multiradio_select(&cc2538_rf_driver);
    NETSTACK_RADIO.on();
  }
  */
  multiradio_select(&cc1200_driver);
  NETSTACK_RADIO.on();
  multiradio_select(&cc2538_rf_driver);
  NETSTACK_RADIO.off();
  
  if(linkaddr_node_addr.u8[0] == 15 && linkaddr_node_addr.u8[1] == 204) {
    while(1) {
      PROCESS_YIELD();
      if(ev == PROCESS_EVENT_TIMER) {
        printf("Broadcast --> %u\n", (unsigned)counter);
        leds_toggle(LEDS_RED);
        if(counter % 2 == 0) {
          printf("Select cc1200\n");
          multiradio_select(&cc1200_driver);
        } else {
          printf("Select cc2538\n");
          multiradio_select(&cc2538_rf_driver);
        }
        packetbuf_copyfrom(&counter, 4);
        broadcast_send(&broadcast);
        counter++;
        etimer_set(&et, LOOP_INTERVAL);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */

