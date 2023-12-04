#ifndef _TOOL_RKAIQ_API_AE_H_
#define _TOOL_RKAIQ_API_AE_H_

#include "logger/log.h"
#include "rk_aiq_user_api_ae.h"
#include "rkaiq_socket.h"

class RKAiqToolAE
{
  public:
    RKAiqToolAE();
    virtual ~RKAiqToolAE();

    int queryExpResInfo(Uapi_ExpQueryInfo_t* pExpResInfo, int cmdID);
    int setExpSwAttr(const Uapi_ExpSwAttr_t expSwAttr, int cmdID);
    int getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr, int cmdID);
    int setLinAeDayRouteAttr(const Uapi_LinAeRouteAttr_t linAeRouteAttr, int cmdID);
    int getLinAeDayRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr, int cmdID);
    int setLinAeNightRouteAttr(const Uapi_LinAeRouteAttr_t linAeRouteAttr, int cmdID);
    int getLinAeNightRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr, int cmdID);
    int setHdrAeDayRouteAttr(const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr, int cmdID);
    int getHdrAeDayRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr, int cmdID);
    int setHdrAeNightRouteAttr(const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr, int cmdID);
    int getHdrAeNightRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr, int cmdID);
    int setLinExpAttr(const Uapi_LinExpAttr_t linExpAttr, int cmdID);
    int getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr, int cmdID);
    int setHdrExpAttr(const Uapi_HdrExpAttr_t hdrExpAttr, int cmdID);
    int getHdrExpAttr(Uapi_HdrExpAttr_t* pHdrExpAttr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_AE_H_
