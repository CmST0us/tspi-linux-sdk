#ifndef __DDR_TEST_H_
#define __DDR_TEST_H_
#include "rk_pcba_test_led.h"

#ifdef PCBA_3308
#define DDR_CAPACITY 256
#endif

#ifdef PCBA_PX3SE
#define DDR_CAPACITY 1010
#endif

#ifdef PCBA_3229GVA
#define DDR_CAPACITY 256
#endif

#ifdef PCBA_1808
#define DDR_CAPACITY 928
#endif

#ifdef PCBA_3326
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_PX30
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_3288
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_3328
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_3399
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_3399PRO
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_1126_1109
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_356X
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 4096
#endif

#ifdef PCBA_3588
//TODO: According to real DDR Capacity to setting
#define   DDR_CAPACITY 8192
#endif

void *ddr_test(void *argv);

#endif
