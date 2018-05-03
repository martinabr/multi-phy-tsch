/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 * \addtogroup tsch
 * @{
*/

#ifndef __TSCH_QUEUE_H__
#define __TSCH_QUEUE_H__

/********** Includes **********/

#include "contiki.h"
#include "lib/ringbufindex.h"
#include "net/linkaddr.h"
#include "net/mac/mac.h"

/***** External Variables *****/

/* Broadcast and EB virtual neighbors */
extern struct tsch_neighbor *n_broadcast;
extern struct tsch_neighbor *n_eb;

/********** Functions *********/

/* Add a TSCH neighbor */
struct tsch_neighbor *tsch_queue_add_nbr(const linkaddr_t *addr);
/* Get a TSCH neighbor */
struct tsch_neighbor *tsch_queue_get_nbr(const linkaddr_t *addr);
/* Get a TSCH time source (we currently assume there is only one) */
struct tsch_neighbor *tsch_queue_get_time_source(void);
/* Update TSCH time source */
int tsch_queue_update_time_source(const linkaddr_t *new_addr);
/* Add packet to neighbor queue. Use same lockfree implementation as ringbuf.c (put is atomic) */
struct tsch_packet *tsch_queue_add_packet(const linkaddr_t *addr, uint8_t max_transmissions,
                                          mac_callback_t sent, void *ptr);
/* Returns the number of packets currently in any TSCH queue */
int tsch_queue_global_packet_count(void);
/* Returns the number of packets currently a given neighbor queue */
int tsch_queue_packet_count(const linkaddr_t *addr);
/* Remove first packet from a neighbor queue. The packet is stored in a separate
 * dequeued packet list, for later processing. Return the packet. */
struct tsch_packet *tsch_queue_remove_packet_from_queue(struct tsch_neighbor *n);
/* Free a packet */
void tsch_queue_free_packet(struct tsch_packet *p);
/* Updates neighbor queue state after a transmission */
int tsch_queue_packet_sent(struct tsch_neighbor *n, struct tsch_packet *p, struct tsch_link *link, uint8_t mac_tx_status);
/* Reset neighbor queues */
void tsch_queue_reset(void);
/* Deallocate neighbors with empty queue */
void tsch_queue_free_unused_neighbors(void);
/* Is the neighbor queue empty? */
int tsch_queue_is_empty(const struct tsch_neighbor *n);
/* Returns the first packet from a neighbor queue */
struct tsch_packet *tsch_queue_get_packet_for_nbr(const struct tsch_neighbor *n, struct tsch_link *link);
/* Returns the head packet from a neighbor queue (from neighbor address) */
struct tsch_packet *tsch_queue_get_packet_for_dest_addr(const linkaddr_t *addr, struct tsch_link *link);
/* Returns the head packet of any neighbor queue with zero backoff counter.
 * Writes pointer to the neighbor in *n */
struct tsch_packet *tsch_queue_get_unicast_packet_for_any(struct tsch_neighbor **n, struct tsch_link *link);
/* May the neighbor transmit over a share link? */
int tsch_queue_backoff_expired(const struct tsch_neighbor *n);
/* Reset neighbor backoff */
void tsch_queue_backoff_reset(struct tsch_neighbor *n);
/* Increment backoff exponent, pick a new window */
void tsch_queue_backoff_inc(struct tsch_neighbor *n);
/* Decrement backoff window for all queues directed at dest_addr */
void tsch_queue_update_all_backoff_windows(const linkaddr_t *dest_addr);
/* Initialize TSCH queue module */
void tsch_queue_init(void);

#endif /* __TSCH_QUEUE_H__ */
/** @} */
