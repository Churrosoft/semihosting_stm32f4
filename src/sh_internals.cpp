#include "../include/semihosting.h"
#include <stdint.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <sys/reent.h>
// ----------------------------------------------------------------------------

static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

/* Struct used to keep track of the file position, just so we
 can implement fseek(fh,x,SEEK_CUR).  */
struct fdent
{
  int handle;
  int pos;
};

#define MAX_OPEN_FILES 20
#define OS_INTEGER_TRACE_SEMIHOSTING_BUFF_ARRAY_SIZE 256
/* User file descriptors (fd) are integer indexes into
 the openfiles[] array. Error checking is done by using
 findslot().

 This openfiles array is manipulated directly by only
 these 5 functions:

 findslot() - Translate entry.
 newslot() - Find empty entry.
 initilise_monitor_handles() - Initialize entries.
 _swiopen() - Initialize entry.
 _close() - Handle stdout == stderr case.

 Every other function must use findslot().  */

static struct fdent openfiles[MAX_OPEN_FILES];

static struct fdent *
findslot(int);

/* following is copied from libc/stdio/local.h to check std streams */
extern void __sinit(struct _reent *);
#define CHECK_INIT(ptr)              \
  do                                 \
  {                                  \
    if ((ptr) && !(ptr)->__sdidinit) \
      __sinit(ptr);                  \
  } while (0)

/* Return a pointer to the structure associated with
 the user file descriptor fd. */
static struct fdent *
findslot(int fd)
{

  CHECK_INIT(_REENT);

  /* User file descriptor is out of range. */
  if ((unsigned int)fd >= MAX_OPEN_FILES)
  {
    return NULL;
  }

  /* User file descriptor is open? */
  if (openfiles[fd].handle == -1)
  {
    return NULL;
  }

  /* Valid. */
  return &openfiles[fd];
}

static int
get_errno(void)
{
  return call_host(SEMIHOSTING_SYS_ERRNO, NULL);
}

/* Set errno and return result. */
static int
error(int result)
{
  errno = get_errno();
  return result;
}

/* Check the return and set errno appropriately. */
static int
checkerror(int result)
{
  if (result == -1)
  {
    return error(-1);
  }

  return result;
}

/* fh, is a valid internal file handle.
 Returns the number of bytes *not* written. */
int sh_swiwrite(int fh, char *ptr, int len)
{
  int block[3];

  block[0] = fh;
  block[1] = (int)ptr;
  block[2] = len;

  return checkerror(call_host(SEMIHOSTING_SYS_WRITE, block));
}

ssize_t
write(const void *buf, size_t nbyte)
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

/* fd, is a user file descriptor. */
int sh_write_usf(int fd, char *ptr, int len)
{
  int res;
  struct fdent *pfd;

  pfd = findslot(fd);
  if (pfd == NULL)
  {
    errno = EBADF;
    return -1;
  }

  res = sh_swiwrite(pfd->handle, ptr, len);

  /* Clearly an error. */
  if (res < 0)
  {
    return -1;
  }

  pfd->pos += len - res;

  /* We wrote 0 bytes?
   Retrieve errno just in case. */
  if ((len - res) == 0)
  {
    return error(0);
  }

  return (len - res);
}

void initialise_monitor_handles()
{
  int i;

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

  for (i = 0; i < MAX_OPEN_FILES; i++)
  {
    openfiles[i].handle = -1;
  }

  openfiles[0].handle = monitor_stdin;
  openfiles[0].pos = 0;
  openfiles[1].handle = monitor_stdout;
  openfiles[1].pos = 0;
  openfiles[2].handle = monitor_stderr;
  openfiles[2].pos = 0;
}