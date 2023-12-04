#include "wifi_util_wrapper.h"
#include "WifiUtil.h"
 
#ifdef __cplusplus
extern "C" {
#endif

static WifiUtil *get_instance() {
    static WifiUtil *wifi_util = NULL;
    if (!wifi_util)
        wifi_util = new WifiUtil();
    return wifi_util;
}

char *get_wifi_list(void) {
    
    std::string list = get_instance()->getWifiListJson();
    return strdup(list.c_str());
}

char *get_device_context(void) {
    std::string list = get_instance()->getDeviceContextJson();
    return strdup(list.c_str());
}
#ifdef __cplusplus
};
#endif
