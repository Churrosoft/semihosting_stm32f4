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
#define TRACE
#if defined(TRACE)
#include "../include/trace.h"
#include "../include/sh_internals.h"
#include <stdio.h>
#include <stdarg.h>

#include "string.h"

#ifndef OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE
#define OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE (128)
#endif

// Este namespace hace poco y nada, no uso newlib, la mayoria del codigo ta' repetido
namespace trace
{
  // ----------------------------------------------------------------------

  /*   ssize_t __attribute__((weak))
  write(const void *buf __attribute__((unused)), size_t nbyte)
  {
    return write(buf, nbyte);
  }
 */
  // ----------------------------------------------------------------------

  int __attribute__((weak))
  printf(const char *format, ...)
  {
    va_list args;
    va_start(args, format);

    int ret = vprintf(format, args);

    va_end(args);
    return ret;
  }

  int __attribute__((weak))
  vprintf(const char *format, va_list args)
  {
    // Caution: allocated on the stack!
    char buf[OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE];

    // TODO: possibly rewrite it to no longer use newlib,
    // (although the nano version is no longer very heavy).

    // Print to the local buffer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    int ret = ::vsnprintf(buf, sizeof(buf), format, args);
#pragma GCC diagnostic pop
    if (ret > 0)
    {
      // Transfer the buffer to the device.
      ret = static_cast<int>(write(buf, static_cast<size_t>(ret)));
    }
    return ret;
  }

  int __attribute__((weak))
  puts(const char *s)
  {
    int ret = static_cast<int>(write(s, strlen(s)));
    if (ret >= 0)
    {
      ret = static_cast<int>(write("\n", 1)); // Add a line terminator
    }
    if (ret > 0)
    {
      return ret;
    }
    else
    {
      return EOF;
    }
  }

  int __attribute__((weak))
  putchar(int c)
  {
    int ret = static_cast<int>(write(reinterpret_cast<const char *>(&c), 1));
    if (ret > 0)
    {
      return c;
    }
    else
    {
      return EOF;
    }
  }

  void __attribute__((weak))
  dump_args(int argc, char *argv[])
  {
    printf("main(argc=%d, argv=[", argc);
    for (int i = 0; i < argc; ++i)
    {
      if (i != 0)
      {
        printf(", ");
      }
      printf("\"%s\"", argv[i]);
    }
    printf("]);\n");
  }

} /* namespace trace */

// ----------------------------------------------------------------------------

int trace_printf(const char *format, ...)
{
  int ret;
  va_list ap;

  va_start(ap, format);

  static char buf[OS_INTEGER_TRACE_PRINTF_TMP_ARRAY_SIZE];

  // Print to the local buffer
  ret = vsnprintf(buf, sizeof(buf), format, ap);
  if (ret > 0)
  {
    // Transfer the buffer to the device
    ret = write(buf, (size_t)ret);
  }

  va_end(ap);
  return ret;
}

int trace_puts(const char *s)
{
  write(s, strlen(s));
  return write("\n", 1);
}

int trace_putchar(int c)
{
  write((const char *)&c, 1);
  return c;
}

void trace_dump_args(int argc, char *argv[])
{
  trace_printf("main(argc=%d, argv=[", argc);
  for (int i = 0; i < argc; ++i)
  {
    if (i != 0)
    {
      trace_printf(", ");
    }
    trace_printf("\"%s\"", argv[i]);
  }
  trace_printf("]);\n");
}

void trace_initialize()
{
  initialise_monitor_handles();
}
// ----------------------------------------------------------------------------

#endif // TRACE
