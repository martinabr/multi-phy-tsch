/*
 * Copyright (c) 2018, RISE SICS
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
#include "contiki.h"
#include "sys/etimer.h"
#include "net/nullnet/nullnet.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "deployment/deployment.h"
#include "net/mac/tsch/tsch.h"

#include <stdio.h>
#include <stdint.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Same interval for data packets as we have for EBs */
#define LOOP_INTERVAL       (TSCH_CONF_EB_PERIOD)

#include "cc1200-rf-cfg.h"
extern const cc1200_rf_cfg_t cc1200_868_4gfsk_1000kbps;
extern const cc1200_rf_cfg_t cc1200_868_2gfsk_250kbps;
extern const cc1200_rf_cfg_t cc1200_868_2gfsk_50kbps_802154g;
extern const cc1200_rf_cfg_t cc1200_868_2gfsk_8kbps;
extern const cc1200_rf_cfg_t cc1200_868_2gfsk_1_2kbps_sp;
extern const struct radio_driver cc1200_driver;

#if TABLETOP_TEST
#define TSCH_COORDINATOR_ID 1
#else
#define TSCH_COORDINATOR_ID 12
#endif

#define RPL_ROOT_ID TSCH_COORDINATOR_ID

#define SFLEN 67
#define SLOT1_OFFSET 33

/*---------------------------------------------------------------------------*/
void
rpl_parent_switch_do_nothing(rpl_parent_t *old, rpl_parent_t *new)
{
}
/*---------------------------------------------------------------------------*/
PROCESS(cc1200_demo_process, "cc1200 demo process");
AUTOSTART_PROCESSES(&cc1200_demo_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc1200_demo_process, ev, data)
{
  struct tsch_slotframe *sf0;
  struct tsch_slotframe *sf1;
  static struct etimer et;
  int i;

  PROCESS_BEGIN();

  LOG_INFO("Waiting 5 seconds before starting\n");
  etimer_set(&et, 5*CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));
  LOG_INFO("Starting\n");

  sf0 = tsch_schedule_add_slotframe(0, SFLEN);
  sf0->cc1200_config = &CC1200_CONF_RF_CFG;
  sf0->radio = &cc1200_driver;

  tsch_schedule_add_link(sf0,
      (LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED | LINK_OPTION_TIME_KEEPING),
      LINK_TYPE_ADVERTISING_ONLY, &tsch_broadcast_address,
      0, 0);

  sf1 = tsch_schedule_add_slotframe(1, SFLEN);
  sf1->cc1200_config = &cc1200_868_4gfsk_1000kbps;
  sf1->radio = &cc1200_driver;

  for(i=0; i<SLOT1_OFFSET; i++) {
    tsch_schedule_add_link(sf1,
      (LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED),
      LINK_TYPE_NORMAL, &tsch_broadcast_address,
      SLOT1_OFFSET + i, 0);
  }

  /* Initialize TSCH */
  tsch_set_coordinator(node_id == TSCH_COORDINATOR_ID);
  NETSTACK_MAC.on();

  /* Initialize RPL root */
  if(node_id == RPL_ROOT_ID) {
    NETSTACK_ROUTING.root_start();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
