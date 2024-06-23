#!/bin/sh

# Display commands, error on unset variables.
set -xu



CC=${CC:-cc}
CFLAGS=${CFLAGS:--Wall -Wextra -Wswitch-enum -Wconversion -Werror -pedantic}

SOURCE=tlpin.c
EXECUTABLE=${SOURCE%.c}

"$CC" $CFLAGS "$SOURCE" -o "$EXECUTABLE" || exit 1
