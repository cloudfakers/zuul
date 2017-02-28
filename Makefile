# Copyright (C) 2017 Ignasi Barrera
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

TARGETS = zuul

CC = gcc
LN = $(CC)
PREFIX ?= /usr/local

CFLAGS = -pipe -O2 -Wall -ansi -pedantic
LDFLAGS = -lcircus -lpthread -lwiringPi


all: $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) -c $@.c
	$(LN) -o $@ $@.o $(LDFLAGS)

install: all
	install -d -m 0755 $(PREFIX)/bin
	install -m 0555 zuul $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/zuul

clean:
	rm -f *.o
	rm -f $(TARGETS)

.PHONY:
	install uninstall clean
