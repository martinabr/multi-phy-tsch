all: cc1200-demo

TARGET ?= zoul
BOARD ?= firefly
MODULES += os/services/deployment

CHANNEL ?= 0
CFLAGS += -DCUSTOM_CHANNEL=$(CHANNEL)

CONTIKI = ../..
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
