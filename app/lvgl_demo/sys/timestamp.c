#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

uint32_t clock_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

uint64_t clock_us(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

void timestamp(char *fmt, int index, int start)
{
    static long int st[128];
    struct timeval t;

    if (start)
    {
        printf("[%d %s]", index, fmt);
        gettimeofday(&t,NULL);
        st[index] = t.tv_sec * 1000000 + t.tv_usec;
    }
    else
    {
        gettimeofday(&t,NULL);
        printf("[%d # cost %ld us]\n", index, (t.tv_sec * 1000000 + t.tv_usec) - st[index]);
    }
}

