CONTIKI_PROJECT = tsch-cc1200
all: $(CONTIKI_PROJECT)

TARGET ?= zoul
BOARD ?= firefly

CONTIKI = ../..
MAKE_MAC = MAKE_MAC_TSCH
MAKE_NET = MAKE_NET_NULLNET
MODULES += os/services/deployment
include $(CONTIKI)/Makefile.include
