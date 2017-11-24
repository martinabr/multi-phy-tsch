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
#include "net/mac/tsch/tsch-schedule.h"
#include "dev/multiradio.h"

#include <stdio.h>
#include <stdint.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define LOOP_INTERVAL       (CLOCK_SECOND)
#define TSCH_COORDINATOR_ID 1

/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
void
input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  LOG_INFO("Received seq %u from ", *(unsigned *)data);
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
  struct tsch_slotframe *sf_cc1200;
  struct tsch_slotframe *sf_cc2538;
  int i;

  PROCESS_BEGIN();

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  /* Create TSCH schedule */
  multiradio_select(&cc1200_driver);
  NETSTACK_RADIO.off();
  multiradio_select(&cc2538_rf_driver);
  NETSTACK_RADIO.off();


  sf_cc1200 = tsch_schedule_add_slotframe(0, 53);
  sf_cc1200->radio = &cc1200_driver;

  sf_cc2538 = tsch_schedule_add_slotframe(1, 53);
  sf_cc2538->radio = &cc2538_rf_driver;

  tsch_schedule_add_link(sf_cc1200,
      LINK_OPTION_TX,
      LINK_TYPE_ADVERTISING_ONLY, &tsch_broadcast_address,
      node_id - 1, 0);
  tsch_schedule_add_link(sf_cc1200,
      LINK_OPTION_SAMPLE_RSSI,
      LINK_TYPE_NORMAL, &tsch_broadcast_address,
      25, 0);
  for(i = 0; i < 25; i++) {
    if(i != node_id - 1) {
      tsch_schedule_add_link(sf_cc1200,
          LINK_OPTION_TIME_KEEPING | LINK_OPTION_RX,
          LINK_TYPE_ADVERTISING_ONLY, &tsch_broadcast_address,
          i, 0);
    }
  }
  tsch_schedule_add_link(sf_cc2538,
      LINK_OPTION_TX,
      LINK_TYPE_NORMAL, &tsch_broadcast_address,
      26 + node_id - 1, 0);
  tsch_schedule_add_link(sf_cc2538,
      LINK_OPTION_SAMPLE_RSSI,
      LINK_TYPE_NORMAL, &tsch_broadcast_address,
      26 + 25, 0);
  for(i = 0; i < 25; i++) {
    if(i != node_id - 1) {
      tsch_schedule_add_link(sf_cc2538,
          LINK_OPTION_RX,
          LINK_TYPE_NORMAL, &tsch_broadcast_address,
          26 + i, 0);
    }
  }

  /* Initialize TSCH */
  tsch_set_coordinator(node_id == TSCH_COORDINATOR_ID);
  NETSTACK_MAC.on();

  if(node_id == 1) {
    etimer_set(&et, LOOP_INTERVAL);
    while(1) {
      PROCESS_YIELD();
      if(ev == PROCESS_EVENT_TIMER) {
        LOG_INFO("Sending seq %u\n", (unsigned)count);
        NETSTACK_NETWORK.output(NULL);
        count++;
        etimer_reset(&et);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
