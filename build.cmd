@echo off
setlocal

if not exist build mkdir build

set CFLAGS=-std=c99 -g -Wno-implicit-int
set LFLAGS=

clang %CFLAGS% -o build\s7.obj -c s7.c %LFLAGS%
clang %CFLAGS% -o build\s7.exe win32_s7.c build\s7.obj %LFLAGS%
