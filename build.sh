#!/bin/sh

gcc -Wall -Werror -o main main.c lexer.c cTooling.c json_types.c diagnostics.c parser.c arena.c
