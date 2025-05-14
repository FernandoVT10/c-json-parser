#!/bin/sh

gcc -Wall -Werror -o main main.c lexer.c cTooling.c diagnostics.c parser.c arena.c json.c
