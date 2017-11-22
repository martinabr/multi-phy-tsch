#include "multiradio.h"

static const struct radio_driver * const radios[] = MULTIRADIO_DRIVERS;
static const int num_radios = sizeof(radios) / sizeof(struct radio_driver *);
static const struct radio_driver * current_radio = NULL;

/*---------------------------------------------------------------------------*/
int
multiradio_select(const struct radio_driver *radio)
{
  int i;
  for(i=0; i<num_radios; i++) {
    if(radios[i] == radio) {
      /* Check that the radio is in our list */
      current_radio = radio;
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  if(num_radios != 0) {
    int i;
    current_radio = radios[0];
    for(i=0; i<num_radios; i++) {
      if(radios[i]->init() == 0) {
      return 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  if(current_radio != NULL) {
    return current_radio->prepare(payload, payload_len);
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  if(current_radio != NULL) {
    return current_radio->transmit(transmit_len);
  } else {
    return RADIO_TX_ERR;
  }
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  if(current_radio != NULL) {
    return current_radio->send(payload, payload_len);
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
radio_read(void *buf, unsigned short buf_len)
{
  if(current_radio != NULL) {
    return current_radio->read(buf, buf_len);
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  if(current_radio != NULL) {
    return current_radio->channel_clear();
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  if(current_radio != NULL) {
    return current_radio->receiving_packet();
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  if(current_radio != NULL) {
    return current_radio->pending_packet();
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  if(current_radio != NULL) {
    return current_radio->on();
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  if(current_radio != NULL) {
    return current_radio->off();
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(current_radio != NULL) {
    return current_radio->get_value(param, value);
  } else {
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  if(current_radio != NULL) {
    return current_radio->set_value(param, value);
  } else {
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  if(current_radio != NULL) {
    return current_radio->get_object(param, dest, size);
  } else {
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  if(current_radio != NULL) {
    return current_radio->set_object(param, src, size);
  } else {
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
const struct radio_driver multiradio_driver =
{
    init,
    prepare,
    transmit,
    send,
    radio_read,
    channel_clear,
    receiving_packet,
    pending_packet,
    on,
    off,
    get_value,
    set_value,
    get_object,
    set_object
};
/*---------------------------------------------------------------------------*/
