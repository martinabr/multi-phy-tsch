CONTIKI_PROJECT = tsch-multiPHY
all: $(CONTIKI_PROJECT)

TARGET ?= zoul
ifeq ($(TARGET),zoul)
BOARD ?= firefly
endif

CONTIKI = ../..
MAKE_MAC = MAKE_MAC_TSCH
MAKE_NET = MAKE_NET_NULLNET
MODULES += os/services/deployment
include $(CONTIKI)/Makefile.include
