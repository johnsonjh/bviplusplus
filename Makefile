# Copyright (c) 2008, 2009, 2010 David Kelley
# Copyright (c) 2016 The Lemon Man
# Copyright (c) 2022 Jeffrey H. Johnson <trnsz@pobox.com>
#
# This file is part of bviplusplis.
#
# Bviplusplus is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Bviplusplus is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with bviplusplus.  If not, see <http://www.gnu.org/licenses/>.
#

TARGET := bviplusplus
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
LIBS += ncurses
LIBS += panel
LIBS += pthread

INCLUDES :=

EXTRA_CFLAGS :=

PROFILE ?= 0
ifneq "$(PROFILE)" "0"
EXTRA_CFLAGS += -pg
endif

DEBUG ?= 0
ifeq "$(DEBUG)" "0"
EXTRA_CFLAGS += -O2
LDFLAGS += -Wl,--gc-sections
else
EXTRA_CFLAGS += -O0 -g3 -ggdb -DDEBUG
LDFLAGS += -s -Wl,--gc-sections
endif

EXTRA_CFLAGS += -Iinclude $(CFLAGS)
EXTRA_CFLAGS += -Wall -pthread -D_FILE_OFFSET_BITS=64 \
		-Wno-stringop-overflow -Wno-unused-but-set-variable \
		-fdata-sections -ffunction-sections \
		-flto -Wl,--gc-sections

VPATH := src
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

# Build all the prereqs and generate dependencies (-MMD -MP)
$(OBJDIR)/%.o: %.c
	$(SHORT) "$(CC) $<"
	$(QUIET)$(CC) $(EXTRA_CFLAGS) $(addprefix -I, $(INCLUDES)) -MMD -MP \
		-c $< -o $@

# Produce our binary
all: mkobjdir $(TARGET)

mkobjdir:
	$(QUIET)$(MKDIR) $(OBJDIR)

$(TARGET): $(BUILD_OBJS)
	$(SHORT) "$(CC) $@"
	$(QUIET)$(CC) $(EXTRA_CFLAGS) $^ $(addprefix -l,$(LIBS)) -o $@ $(LDFLAGS)

clean:
	rm -rf "$(OBJDIR)"
	rm -rf "$(TARGET)"

distclean: clean

install: $(TARGET)
	install -D $(TARGET) $(PREFIX)/bin/$(TARGET)

# Include dependencies
-include $(BUILD_OBJS:.o=.d)
