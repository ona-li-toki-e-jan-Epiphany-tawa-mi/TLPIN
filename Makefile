CC      ?= gcc
CFLAGS  =  -Wall -Wextra -Wswitch-enum -pedantic -Werror

SOURCES = tlpin.c

tlpin: $(SOURCES)
