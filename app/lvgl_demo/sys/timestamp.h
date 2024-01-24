#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

uint32_t clock_ms(void);
uint64_t clock_us(void);
void timestamp(char *fmt, int index, int start);

#endif

