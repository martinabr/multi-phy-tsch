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
 *         An example of Rime/TSCH
 * \author
 *         Simon Duquennoy <simon.duquennoy@inria.fr>
 *
 */

#include <stdio.h>
#include "contiki-conf.h"
#include "net/netstack.h"
#include "net/rime/rime.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "dev/multiradio.h"
#include "apps/deployment/deployment.h"

#include "cc1200-const.h"
#include "cc1200-conf.h"
#include "cc1200-arch.h"
#include "cc1200-rf-cfg.h"

extern const cc1200_rf_cfg_t CC1200_RF_CFG;

static uint32_t counter;
#define TSCH_COORDINATOR_ID 1
#define BROADCAST_CHANNEL 129

#define DEBUG WITH_SINGLE_SENDER

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(test_process, "Rime Node");
AUTOSTART_PROCESSES(&test_process);

/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
   PRINTF("App: received from %u seq %u rssi %d\n",
         nodeid_from_linkaddr(from),
         (unsigned)*(uint32_t *)packetbuf_dataptr(),
         (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI)
         );
}
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_process, ev, data)
{
  static struct etimer et;
  int i;
  PROCESS_BEGIN();

  etimer_set(&et, 4*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  node_id = get_node_id();
  tsch_set_coordinator(node_id == TSCH_COORDINATOR_ID);

#if WITH_MULTIRADIO
  multiradio_select(&cc1200_driver);
  NETSTACK_RADIO.off();
  multiradio_select(&cc2538_rf_driver);
  NETSTACK_RADIO.off();

  struct tsch_slotframe *sf_cc1200;
  sf_cc1200 = tsch_schedule_add_slotframe(0, 53);
  sf_cc1200->radio = &cc1200_driver;
  
  struct tsch_slotframe *sf_cc2538;
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
  //tsch_schedule_add_link(sf_cc2538,
    //  LINK_OPTION_TX | LINK_OPTION_RX,
//      LINK_TYPE_NORMAL, &tsch_broadcast_address,
      //3, 0);
#else /* WITH_MULTIRADIO */
  struct tsch_slotframe *sf;
  sf = tsch_schedule_add_slotframe(0, 29);
  tsch_schedule_add_link(sf,
       LINK_OPTION_TX,
       LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
       node_id - 1, 0);
  for(i = 0; i < 25; i++) {
    if(i != node_id - 1) {
     tsch_schedule_add_link(sf,
         LINK_OPTION_TIME_KEEPING | LINK_OPTION_RX,
         LINK_TYPE_ADVERTISING, &tsch_broadcast_address,
         i, 0);
    }
  }
#endif /* WITH_MULTIRADIO */

  NETSTACK_MAC.on();
  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

#if WITH_SINGLE_SENDER
  if(node_id == 1)
#endif
  {
    while(1) {
      etimer_set(&et, 8*CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

      PRINTF("App: sending seq %u\n", (unsigned)counter);
      packetbuf_copyfrom(&counter, sizeof(counter) + 4);
      broadcast_send(&bc);
      counter++;
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
