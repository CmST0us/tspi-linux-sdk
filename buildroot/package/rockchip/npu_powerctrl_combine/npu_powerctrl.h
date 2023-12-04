// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
// Copyright (c) 2018 Fuzhou Rockchip Electronics Co., Ltd

#ifndef NPU_POWERCTRL_H
#define NPU_POWERCTRL_H

#ifdef __cplusplus 
extern "C" {
#endif

void npu_power_gpio_init(void);
void npu_power_gpio_exit(void);
void npu_reset(void);
int npu_suspend(void);
int npu_resume(void);
void npu_poweroff(void);

#ifdef __cplusplus 
}
#endif

#endif
