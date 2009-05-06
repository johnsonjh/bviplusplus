TARGET := bviplus
PREFIX ?= /usr/local

OBJS :=
OBJS += actions.o
OBJS += app_state.o
OBJS += creadline.o
OBJS += display.o
OBJS += help.o
OBJS += key_handler.o
OBJS += main.o
OBJS += search.o
OBJS += user_prefs.o
OBJS += vf_backend.o
OBJS += virt_file.o

LIBS :=
LIBS += curses
LIBS += panel
LIBS += pthread

INCLUDES :=

EXTRA_CFLAGS :=

DEBUG ?= 0
ifeq "$(DEBUG)" "0"
EXTRA_CFLAGS += -O2
else
EXTRA_CFLAGS += -O0 -g
endif
EXTRA_CFLAGS += $(CFLAGS)
EXTRA_CFLAGS += -Wall -D_FILE_OFFSET_BITS=64

OBJDIR := objs
BUILD_OBJS := $(addprefix $(OBJDIR)/,$(OBJS))

# By default do a quiet build
ifeq "$(V)" "1"
QUIET :=
SHORT := @true
else
QUIET := @
SHORT := @echo
endif
MKDIR := mkdir -p
SILENT := @

.PHONY: all mkobjdir clean install

# Build all the prereqs and generate dependencies (-MMD)
$(OBJDIR)/%.o: %.c
	$(SHORT) "CC $<"
	$(QUIET)$(CC) $(EXTRA_CFLAGS) $(addprefix -I, $(INCLUDES)) -MMD -c $< -o $@

# Produce our binary
all: mkobjdir $(TARGET)

mkobjdir:
	$(QUIET)$(MKDIR) $(OBJDIR)

$(TARGET): $(BUILD_OBJS)
	$(SHORT) "LD $@"
	$(QUIET)$(CC) $(EXTRA_CFLAGS) $^ $(addprefix -l,$(LIBS)) -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

distclean: clean

install: $(TARGET)
	install -D $(TARGET) $(PREFIX)/bin/$(TARGET)

# Include dependencies
-include $(BUILD_OBJS:.o=.d)

