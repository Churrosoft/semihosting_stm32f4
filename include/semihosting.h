/*
* awante stackoverflow vieja:
* https://electronics.stackexchange.com/questions/149387/how-do-i-print-debug-messages-to-gdb-console-with-stm32-discovery-board-using-gd
*/

#ifndef ARM_SEMIHOSTING_H_
#define ARM_SEMIHOSTING_H_

// Semihosting operations.
enum OperationNumber
{
  // Regular operations
  SEMIHOSTING_EnterSVC = 0x17,
  SEMIHOSTING_ReportException = 0x18,
  SEMIHOSTING_SYS_CLOSE = 0x02,
  SEMIHOSTING_SYS_CLOCK = 0x10,
  SEMIHOSTING_SYS_ELAPSED = 0x30,
  SEMIHOSTING_SYS_ERRNO = 0x13,
  SEMIHOSTING_SYS_FLEN = 0x0C,
  SEMIHOSTING_SYS_GET_CMDLINE = 0x15,
  SEMIHOSTING_SYS_HEAPINFO = 0x16,
  SEMIHOSTING_SYS_ISERROR = 0x08,
  SEMIHOSTING_SYS_ISTTY = 0x09,
  SEMIHOSTING_SYS_OPEN = 0x01,
  SEMIHOSTING_SYS_READ = 0x06,
  SEMIHOSTING_SYS_READC = 0x07,
  SEMIHOSTING_SYS_REMOVE = 0x0E,
  SEMIHOSTING_SYS_RENAME = 0x0F,
  SEMIHOSTING_SYS_SEEK = 0x0A,
  SEMIHOSTING_SYS_SYSTEM = 0x12,
  SEMIHOSTING_SYS_TICKFREQ = 0x31,
  SEMIHOSTING_SYS_TIME = 0x11,
  SEMIHOSTING_SYS_TMPNAM = 0x0D,
  SEMIHOSTING_SYS_WRITE = 0x05,
  SEMIHOSTING_SYS_WRITEC = 0x03,
  SEMIHOSTING_SYS_WRITE0 = 0x04,

  // Codes returned by SEMIHOSTING_ReportException
  ADP_Stopped_ApplicationExit = ((2 << 16) + 38),
  ADP_Stopped_RunTimeError = ((2 << 16) + 35),

};

enum OperationType{
  STDERR = 0x02,
};

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------

#endif // ARM_SEMIHOSTING_H_
