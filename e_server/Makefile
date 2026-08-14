# e_server reference backend (host-side).
#
# Targets:
#   make            build web_assets.h + e_server
#   make run        build and run on :8080
#   make clean
#
# The web bundle (web_assets.h) is regenerated whenever web/ or public/
# changes. Needs Python for build_web.py and a host C compiler (gcc).

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
LDLIBS  :=
EXT     :=

ifeq ($(OS),Windows_NT)
  LDLIBS += -lws2_32
  EXT    := .exe
endif

WEB_SRC   := web/index.html web/style.css web/app.js
PUB_SRC   := $(wildcard public/*)

all: e_server$(EXT)

web_assets.h: build_web.py $(WEB_SRC) $(PUB_SRC)
	python build_web.py

server.o: server.c web_assets.h
	$(CC) $(CFLAGS) -c server.c

e_server$(EXT): server.o
	$(CC) $(CFLAGS) -o $@ server.o $(LDLIBS)

run: all
	./e_server$(EXT) 8080

clean:
	rm -f server.o e_server e_server.exe

.PHONY: all run clean
