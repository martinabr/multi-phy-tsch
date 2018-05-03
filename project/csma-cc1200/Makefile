CONTIKI_PROJECT = csma-cc1200
all: $(CONTIKI_PROJECT)

TARGET ?= zoul
BOARD ?= firefly

CHANNEL ?= 0
CFLAGS += -DCUSTOM_CHANNEL=$(CHANNEL)

CONTIKI = ../..
MAKE_NET = MAKE_NET_NULLNET
MODULES += os/services/deployment
include $(CONTIKI)/Makefile.include
