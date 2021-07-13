#include "../include/semihosting.h"
#include <stdint.h>
#include <stdio.h>

void send_command(int command, void *message)
{
#ifndef __INTELLISENSE__
  asm("mov r0, %[cmd];"
      "mov r1, %[msg];"
      "bkpt #0xAB"
      :
      : [cmd] "r"(command), [msg] "r"(message)
      : "r0", "r1", "memory");
#endif
}