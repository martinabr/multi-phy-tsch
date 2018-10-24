CONTIKI_PROJECT = tsch-rpl-multiPHY
all: $(CONTIKI_PROJECT)

TARGET ?= zoul
ifeq ($(TARGET),zoul)
BOARD ?= firefly
endif

CONTIKI = ../..
MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/deployment
include $(CONTIKI)/Makefile.include
