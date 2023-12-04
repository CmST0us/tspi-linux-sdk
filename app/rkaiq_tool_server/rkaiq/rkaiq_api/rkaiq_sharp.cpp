#include "rkaiq_sharp.h"

#include "rkaiq_socket.h"

RKAiqToolSharp::RKAiqToolSharp()
{
}

RKAiqToolSharp::~RKAiqToolSharp()
{
}

int RKAiqToolSharp::SetAttrib(rk_aiq_sharp_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)attr, sizeof(rk_aiq_sharp_attrib_t));
}

int RKAiqToolSharp::GetAttrib(rk_aiq_sharp_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_sharp_attrib_t));
}

int RKAiqToolSharp::SetIQPara(void* para, int cmdID)
{
    int paramSize = 0;
    if (cmdID == ENUM_ID_SHARP_SET_IQPARA) {
        paramSize = sizeof(CalibDb_Sharp_t);
    } else if (cmdID == ENUM_ID_SHARP_SET_EF_IQPARA) {
        paramSize = sizeof(CalibDb_EdgeFilter_t);
    } else {
        return 1;
    }
    return RkAiqSocketClientINETSend(cmdID, para, paramSize);
}

int RKAiqToolSharp::GetIQPara(void* para, int cmdID)
{
    int paramSize = 0;
    if (cmdID == ENUM_ID_SHARP_GET_IQPARA) {
        paramSize = sizeof(CalibDb_Sharp_t);
    } else if (cmdID == ENUM_ID_SHARP_GET_EF_IQPARA) {
        paramSize = sizeof(CalibDb_EdgeFilter_t);
    } else {
        return 1;
    }
    return RkAiqSocketClientINETReceive(cmdID, para, paramSize);
}

int RKAiqToolSharp::SetStrength(float fPercent, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&fPercent, sizeof(float));
}

int RKAiqToolSharp::GetStrength(float* pPercent, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pPercent, sizeof(float));
}