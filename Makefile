XML_DIR   = ./xml
REG_DIR   = ./register
OBJECTS   = $(patsubst %.c,%.o,$(wildcard *.c))
XML_FILES = $(wildcard $(XML_DIR)/*.atdf $(XML_DIR)/*.svd)

ARCH ?= x86
ifeq (${ARCH},x86)
	CROSS_COMPILE =
	OUT_DIR = out_x86
endif
ifeq (${ARCH},arm)
	CROSS_COMPILE ?= arm-buildroot-linux-uclibcgnueabihf-
	OUT_DIR = out_arm
endif

ifndef V
QUIET_CC    = @echo '  CC       '$@;
QUIET_LINK  = @echo '  LINK     '$@;
QUIET_STRIP = @echo '  STRIP    '$@;
endif

CC      = $(QUIET_CC)$(CROSS_COMPILE)gcc
LD      = $(QUIET_LINK)$(CROSS_COMPILE)gcc
STRIP   = $(QUIET_STRIP)$(CROSS_COMPILE)strip
CFLAGS  = -Wall -Werror -O3 -I./include
LDFLAGS = -ldl

.PHONY: all
all: samconf

register: $(XML_FILES)
	rm -rf $(REG_DIR)
	mkdir -p $(REG_DIR)
	$(XML_DIR)/xml2c.py $(XML_DIR) $(REG_DIR)

samconf: $(OBJECTS)
	$(LD) $(CFLAGS) ${LDFLAGS} $(OBJECTS) -o $@
	$(STRIP) $@
	mkdir -p ./$(OUT_DIR)
	cp $@ ./$(OUT_DIR)
	#cp $@ ~/srv/nfs

clean:
	-rm -f samconf *.o *.map

distclean: clean
	-rm -rf $(REG_DIR) out_*
