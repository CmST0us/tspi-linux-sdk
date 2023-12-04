#include "rkaiq_ae.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

RKAiqToolAE::RKAiqToolAE()
{
}
RKAiqToolAE::~RKAiqToolAE()
{
}

int RKAiqToolAE::queryExpResInfo(Uapi_ExpQueryInfo_t* pExpResInfo, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pExpResInfo, sizeof(Uapi_ExpQueryInfo_t));
}

int RKAiqToolAE::setExpSwAttr(const Uapi_ExpSwAttr_t expSwAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&expSwAttr), sizeof(Uapi_ExpSwAttr_t));
}

int RKAiqToolAE::getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pExpSwAttr, sizeof(Uapi_ExpSwAttr_t));
}

int RKAiqToolAE::setLinAeDayRouteAttr(const Uapi_LinAeRouteAttr_t linAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&linAeRouteAttr), sizeof(Uapi_LinAeRouteAttr_t));
}

int RKAiqToolAE::getLinAeDayRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pLinAeRouteAttr, sizeof(Uapi_LinAeRouteAttr_t));
}

int RKAiqToolAE::setLinAeNightRouteAttr(const Uapi_LinAeRouteAttr_t linAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&linAeRouteAttr), sizeof(Uapi_LinAeRouteAttr_t));
}

int RKAiqToolAE::getLinAeNightRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pLinAeRouteAttr, sizeof(Uapi_LinAeRouteAttr_t));
}

int RKAiqToolAE::setHdrAeDayRouteAttr(const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&hdrAeRouteAttr), sizeof(Uapi_HdrAeRouteAttr_t));
}

int RKAiqToolAE::getHdrAeDayRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pHdrAeRouteAttr, sizeof(Uapi_HdrAeRouteAttr_t));
}

int RKAiqToolAE::setHdrAeNightRouteAttr(const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&hdrAeRouteAttr), sizeof(Uapi_HdrAeRouteAttr_t));
}

int RKAiqToolAE::getHdrAeNightRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pHdrAeRouteAttr, sizeof(Uapi_HdrAeRouteAttr_t));
}

int RKAiqToolAE::setLinExpAttr(const Uapi_LinExpAttr_t linExpAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&linExpAttr), sizeof(Uapi_LinExpAttr_t));
}

int RKAiqToolAE::getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pLinExpAttr, sizeof(Uapi_LinExpAttr_t));
}

int RKAiqToolAE::setHdrExpAttr(const Uapi_HdrExpAttr_t hdrExpAttr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)(&hdrExpAttr), sizeof(Uapi_HdrExpAttr_t));
}

int RKAiqToolAE::getHdrExpAttr(Uapi_HdrExpAttr_t* pHdrExpAttr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pHdrExpAttr, sizeof(Uapi_HdrExpAttr_t));
}
