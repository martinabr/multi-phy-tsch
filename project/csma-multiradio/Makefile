CONTIKI_PROJECT = csma-multiradio
all: $(CONTIKI_PROJECT)

TARGET ?= zoul
BOARD ?= firefly

CONTIKI = ../..
MAKE_NET = MAKE_NET_NULLNET
MODULES += os/services/deployment
include $(CONTIKI)/Makefile.include
