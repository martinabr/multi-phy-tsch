DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = cc1200-demo

all: $(CONTIKI_PROJECT)

TARGET ?= zoul
BOARD ?= firefly
MODULES += os/services/deployment

CHANNEL ?= 0
CFLAGS += -DCUSTOM_CHANNEL=$(CHANNEL)

CONTIKI = ../..
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
