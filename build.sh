#!/bin/sh

gcc -Wall -Werror -o main main.c lexer.c cTooling.c parser.c json_types.c
