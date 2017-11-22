DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = cc1200-demo

all: $(CONTIKI_PROJECT)

TARGET ?= zoul
BOARD ?= firefly
APPS = deployment

CFLAGS += -DCUSTOM_CHANNEL=$(CHANNEL)

CONTIKI = ../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
