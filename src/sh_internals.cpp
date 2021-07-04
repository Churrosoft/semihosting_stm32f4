#include "../include/semihosting.h"
#include <stdint.h>
#include <stdio.h>

// ----------------------------------------------------------------------------

static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

#define MAX_OPEN_FILES 20
#define OS_INTEGER_TRACE_SEMIHOSTING_BUFF_ARRAY_SIZE 256

ssize_t write(const void *buf, size_t nbyte)
{
  if (buf == nullptr || nbyte == 0)
  {
    return 0;
  }

  const char *cbuf = (const char *)buf;

  // Since the single character debug channel is quite slow, try to
  // optimise and send a null terminated string, if possible.
  if (cbuf[nbyte] == '\0')
  {
    // send string
    // en otros casos se usa 'SEMIHOSTING_SYS_WRITE'
    call_host(SEMIHOSTING_SYS_WRITE0, (void *)cbuf);
  }
  else
  {
    // If not, use a local buffer to speed things up.
    // For re-entrance, this bugger must be allocated on the stack,
    // so be cautious with the size.
    char tmp[OS_INTEGER_TRACE_SEMIHOSTING_BUFF_ARRAY_SIZE];
    size_t togo = nbyte;
    while (togo > 0)
    {
      size_t n = ((togo < sizeof(tmp)) ? togo : sizeof(tmp) - 1);
      size_t i = 0;
      for (; i < n; ++i, ++cbuf)
      {
        tmp[i] = *cbuf;
      }
      tmp[i] = '\0';

      call_host(SEMIHOSTING_SYS_WRITE0, (void *)tmp);

      togo -= n;
    }
  }

  // All bytes written.
  return (ssize_t)nbyte;
}

void initialise_monitor_handles()
{

  /* Open the standard file descriptors by opening the special
   * teletype device, ":tt", read-only to obtain a descriptor for
   * standard input and write-only to obtain a descriptor for standard
   * output. Finally, open ":tt" in append mode to obtain a descriptor
   * for standard error. Since this is a write mode, most kernels will
   * probably return the same value as for standard output, but the
   * kernel can differentiate the two using the mode flag and return a
   * different descriptor for standard error.
   */

  int volatile block[3];

  block[0] = (int)":tt";
  block[2] = 3; /* length of filename */
  block[1] = 0; /* mode "r" */
  monitor_stdin = call_host(SEMIHOSTING_SYS_OPEN, (void *)block);

  block[0] = (int)":tt";
  block[2] = 3; /* length of filename */
  block[1] = 4; /* mode "w" */
  monitor_stdout = call_host(SEMIHOSTING_SYS_OPEN, (void *)block);

  block[0] = (int)":tt";
  block[2] = 3; /* length of filename */
  block[1] = 8; /* mode "a" */
  monitor_stderr = call_host(SEMIHOSTING_SYS_OPEN, (void *)block);

  /* If we failed to open stderr, redirect to stdout. */
  if (monitor_stderr == -1)
  {
    monitor_stderr = monitor_stdout;
  }
}