/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
// ----------------------------------------------------------------------------
#if defined(TRACE)
#include "../include/trace.h"

#include <stdarg.h>
#include <stdio.h>

#include "../include/semihosting.h"
#include "../include/sh_internals.h"
#include "string.h"

#ifndef OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE
#define OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE (256)
#endif

// ----------------------------------------------------------------------------

int trace_printf(const char *format, ...) {
  int ret;
  va_list ap;

  va_start(ap, format);

  static char buf[OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE];

  // Print to the local buffer
  ret = vsnprintf(buf, sizeof(buf), format, ap);
  if (ret > 0) {
    // Transfer the buffer to the device
    uint32_t message[] = {STDERR, (uint32_t)(buf), strlen(buf) / sizeof(char) - 1};
    send_command(SEMIHOSTING_SYS_WRITE, message);
  }

#if defined(TESTING)
  printf("\n");
#endif
  va_end(ap);
  return ret;
}

int debug_printf(const char *format, ...) {
  int ret;
  va_list ap;
#if defined(TESTING)

  va_start(ap, format);

  static char buf[OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE];

  // Print to the local buffer
  ret = vsnprintf(buf, sizeof(buf), format, ap);
  if (ret > 0) {
    // Transfer the buffer to the device
    uint32_t message[] = {STDERR, (uint32_t)(buf), strlen(buf) / sizeof(char) - 1};
    send_command(SEMIHOSTING_SYS_WRITE, message);
  }

  printf("\n");
#endif
  va_end(ap);
  return ret;
}

int trace_puts(const char *s) {
  uint32_t message[] = {STDERR, (uint32_t)(s), strlen(s) / sizeof(char) - 1};
  send_command(SEMIHOSTING_SYS_WRITE, message);
  return 0;
}

void trace_dump_args(int argc, char *argv[]) {
  trace_printf("main(argc=%d, argv=[", argc);
  for (int i = 0; i < argc; ++i) {
    if (i != 0) {
      trace_printf(", ");
    }
    trace_printf("\"%s\"", argv[i]);
  }
  trace_printf("]);\n");
}

/* [[deprecated]] */ void trace_initialize() {}

void hexStr(unsigned char data, char *dataArr) {
  const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  char A = hexmap[(data & 0xF0) >> 4];
  char B = hexmap[data & 0x0F];

  dataArr[0] = '0';
  dataArr[1] = 'x';
  dataArr[2] = A;
  dataArr[3] = B;
  dataArr[4] = '\0';
}

// ----------------------------------------------------------------------------
#else
int trace_printf(const char *format, ...) { return -1; }
int debug_printf(const char *format, ...) { return -1; }

int trace_puts(const char *s) { return -1; }

int trace_putchar(int c) { return -1; }

void trace_dump_args(int argc, char *argv[]) {}

void trace_initialize() {}
void hexStr(unsigned char data, char *dataArr) {}
#endif    // TRACE
