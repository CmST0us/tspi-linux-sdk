#ifndef __RK_BLE_APP_H__
#define __RK_BLE_APP_H__

#include <DeviceIo/RkBtBase.h>
#include <DeviceIo/RkBle.h>

#ifdef __cplusplus
extern "C" {
#endif

void rk_ble_wifi_init(void *data);
void rk_ble_wifi_deinit(void *data);

#ifdef __cplusplus
}
#endif

#endif /* __RK_BLE_APP_H__ */
