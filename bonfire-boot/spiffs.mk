.phony: test build

spiffs?=spiffs

include spiffs/files.mk

vpath %.c $(CPATH)

OBJFILES = $(CFILES:%.c=${BUILD}/%.o)

include  ../bonfire_defs.Makefile.include

TARGET_CFLAGS += $(INC) -I. $(FLAGS)

TARGET= $(BUILD)/spiffs.a 

$(TARGET) : $(OBJFILES)
	$(TARGET_AR) r $(TARGET) $(OBJFILES)
	$(TARGET_PREFIX)-ranlib $(TARGET)
	$(TARGET_SIZE) $(TARGET) 

build: $(TARGET)

test:
	echo $(OBJFILES) ; echo $(CFILES)