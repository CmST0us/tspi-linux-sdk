#ifndef _RK_AIQ_CONFIG_H_
#define _RK_AIQ_CONFIG_H_

#define ENABLE_ALL 1
#define DBUS_NAME                "rockchip.ispserver"
#define DBUS_PATH                "/"
#define DBUS_IF                  DBUS_NAME ".sysctl"
#define SHARE_PATH               "."
#define DBG(...) do { printf(__VA_ARGS__); } while(0)
#define ENABLE_TEST 0

#define CALL_SET_AIQ(fun) \
     DBG("enter %s, line=%d\n",__FUNCTION__, __LINE__);\
     fun##_t *fun_st = args; \
     const rk_aiq_sys_ctx_t* ctx = fun_st->sys_ctx;\
     XCamReturn r = 0; \
     if (ENABLE_TEST == 0) \
      r = fun(ctx,  fun_st->attr); \
     fun_st->returnvalue = r;

#define CALL_SET_AIQ_EXT(fun) \
     DBG("enter %s, line=%d\n",__FUNCTION__, __LINE__);\
     fun##_t *fun_st = args; \
     const rk_aiq_sys_ctx_t* ctx = fun_st->sys_ctx;\
     XCamReturn r = 0;\
     if (ENABLE_TEST == 0) \
     r = fun(ctx,  fun_st->level); \
     fun_st->returnvalue = r;

#define CALL_SET_AIQ_P(fun) \
     DBG("enter %s,line=%d\n",__FUNCTION__, __LINE__);\
     fun##_t *fun_st = args; \
     const rk_aiq_sys_ctx_t* ctx = fun_st->sys_ctx;\
     XCamReturn r = 0; \
     if (ENABLE_TEST == 0) \
        r = fun(ctx,  &fun_st->attr); \
     fun_st->returnvalue = r;

#define CALL_GET_AIQ(fun) \
     DBG("enter %s,line=%d\n",__FUNCTION__,__LINE__);\
     fun##_t *fun_st = args; \
     const rk_aiq_sys_ctx_t* ctx = fun_st->sys_ctx;\
     XCamReturn r = 0;\
     if (ENABLE_TEST == 0) \
       r = fun(ctx,  &fun_st->attr); \
     fun_st->returnvalue = r;

#define CALL_GET_AIQ_EXT(fun) \
     DBG("enter %s,line=%d\n",__FUNCTION__,__LINE__);\
     fun##_t *fun_st = args; \
     const rk_aiq_sys_ctx_t* ctx = fun_st->sys_ctx;\
     XCamReturn r = 0; \
     if (ENABLE_TEST == 0) \
     r = fun(ctx,  &fun_st->level); \
     fun_st->returnvalue = r;

#define CLIENT_CALL_SET_AIQ(fun)\
     DBG("===enter %s, line=%d\n",__FUNCTION__,__LINE__);\
    fun##_t  para; \
    para.sys_ctx = sys_ctx; \
    memcpy(&para.attr, (void*)&attr, sizeof(para.attr)); \
    call_fun_ipc_call((char *)__FUNCTION__, &para, sizeof(fun##_t), 1); \
    return para.returnvalue;

#define CLIENT_CALL_SET_AIQ_EXT(fun)\
     DBG("enter %s, line=%d\n",__FUNCTION__,__LINE__);\
    fun##_t  para; \
    para.sys_ctx = sys_ctx; \
    para.level = level; \
    call_fun_ipc_call((char *)__func__, &para, sizeof(fun##_t), 1); \
    return para.returnvalue;

#define CLIENT_CALL_SET_AIQ_P(fun)\
    DBG("enter %s,line=%d\n",__FUNCTION__, __LINE__);\
    fun##_t  para; \
    para.sys_ctx = sys_ctx; \
    memcpy(&para.attr, (void*)attr, sizeof(para.attr)); \
    call_fun_ipc_call((char *)__func__, &para, sizeof(fun##_t), 1); \
    return para.returnvalue;


#define CLIENT_CALL_GET_AIQ(fun)\
    DBG("enter %s,line=%d\n",__FUNCTION__,__LINE__);\
    if (attr == NULL) { \
        return -1; \
    } \
    fun##_t  para; \
    para.sys_ctx = sys_ctx; \
    call_fun_ipc_call((char *)__func__, &para, sizeof(para), 1); \
    memcpy(attr, &para.attr, sizeof(para.attr)); \
    return para.returnvalue;

#define CLIENT_CALL_GET_AIQ_EXT(fun)\
    DBG("enter %s,line=%d\n",__FUNCTION__,__LINE__);\
    if (level == NULL) { \
        return -1; \
    } \
    fun##_t  para; \
    para.sys_ctx = sys_ctx; \
    call_fun_ipc_call((char *)__func__, &para, sizeof(para), 1); \
    *level = para.level; \
    return para.returnvalue;

#endif
