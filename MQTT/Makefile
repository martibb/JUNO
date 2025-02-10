CONTIKI_PROJECT = test
all: $(CONTIKI_PROJECT)

CONTIKI = ../..
ROOT=..

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/mqtt

-include $(CONTIKI)/Makefile.identify-target

MODULES_REL += arch/platform/$(TARGET)
MODULES_REL += $(ROOT)/sensing/lidar
MUDULES_REL += $(ROOT)/sensing/gyroscope

include $(CONTIKI)/Makefile.include
