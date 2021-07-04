#ifndef SH_INTERNALS_H
#define SH_INTERNALS_H

//static struct fdent *findslot();
int sh_write_usf(int, char *, int);
int sh_swiwrite(int, char *, int);
void initialise_monitor_handles(void);

ssize_t write(const void *, size_t);

#endif