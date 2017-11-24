all: csma-cc1200

TARGET ?= zoul
BOARD ?= firefly

CHANNEL ?= 0
CFLAGS += -DCUSTOM_CHANNEL=$(CHANNEL)

CONTIKI = ../..
MAKE_NET = MAKE_NET_NULLNET
MODULES += os/services/deployment
include $(CONTIKI)/Makefile.include
