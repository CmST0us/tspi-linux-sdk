/**
 * Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
 * author: Chad.ma <Chad.ma@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _USBBOOT_H_
#define _USBBOOT_H_

#define EX_UDISK_ROOT  "/mnt/udisk"
#define EX_UDISK_ROOT2  "/mnt/usb_storage"

#ifdef  __cplusplus
extern "C" {
#endif

bool is_boot_from_udisk(void);
void ensure_udisk_mounted(bool *bSDMounted);
bool is_udisk_update(void);

#ifdef  __cplusplus
}
#endif

#endif  //_USBBOOT_H_
