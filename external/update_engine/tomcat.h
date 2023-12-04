int getDataFromUrl(char *url);
typedef int(*RK_showprogress_callback)(double dlnow, double dltotal);
void setProgressCallBack(RK_showprogress_callback _cb);
int showProgressValue();
