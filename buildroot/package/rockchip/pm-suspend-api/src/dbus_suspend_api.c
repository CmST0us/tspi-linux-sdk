#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <dbus/dbus.h>
#include <unistd.h>
#include "dbus_suspend_api.h"

#define SENSSION_REGISTER_DIR "/var/run/pm_dbus_services"/*注册后，在此目录创建同名文件，bus_name*/

#define OBJECT_PATH "/"
#define SUSPEND_INTERFACE "local.interface"
#define SUSPEND_FUNCTION "suspend"
#define RESUME_FUNCTION "resume"

typedef enum __bool {
	false = 0,
	true =1,
}bool;

typedef struct __listener {
	char bus_name[64];
	CALLBACK suspend_callback;
	CALLBACK resume_callback;
}suspend_listener;

void reply_to_method_call(suspend_listener *listener, DBusMessage * msg, DBusConnection * conn, bool suspend)
{
    DBusMessage * reply;
    DBusMessageIter arg;
    char *param = NULL;
    dbus_bool_t stat = TRUE;
    dbus_uint32_t level = 2010;
    dbus_uint32_t serial = 0;

    if(!dbus_message_iter_init(msg,&arg))
        printf("Message has noargs\n");
    else if(dbus_message_iter_get_arg_type(&arg)!= DBUS_TYPE_STRING)
        printf("Arg is notstring!\n");
    else
       dbus_message_iter_get_basic(&arg, &param);
    if(param == NULL && !suspend) return;

	if (suspend) {
		listener->suspend_callback(NULL);
	} else {
		printf("resume irq: %s\n", param);
		listener->resume_callback(param);
	}
    //创建返回消息reply
    reply = dbus_message_new_method_return(msg);
    //在返回消息中填入两个参数，和信号加入参数的方式是一样的。这次我们将加入两个参数。
    dbus_message_iter_init_append(reply,&arg);
    if(!dbus_message_iter_append_basic(&arg,DBUS_TYPE_BOOLEAN,&stat)){
        printf("Out ofMemory!\n");
        exit(1);
    }
    if(!dbus_message_iter_append_basic(&arg,DBUS_TYPE_UINT32,&level)){
        printf("Out ofMemory!\n");
        exit(1);
    }
	//发送返回消息
	if( !dbus_connection_send(conn, reply,&serial)){
        printf("Out of Memory\n");
        exit(1);
    }
    dbus_connection_flush (conn);
    dbus_message_unref (reply);
}

void listen_dbus(suspend_listener *listener)
{
    DBusMessage * msg;
    DBusMessageIter arg;
    DBusConnection * connection;
    DBusError err;
    int ret;
    char * sigvalue;

    dbus_error_init(&err);
    //创建于session D-Bus的连接
    connection =dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if(dbus_error_is_set(&err)){
        fprintf(stderr,"ConnectionError %s\n",err.message);
        dbus_error_free(&err);
    }
    if(connection == NULL)
        return;

    ret =dbus_bus_request_name(connection, listener->bus_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if(dbus_error_is_set(&err)) {
        fprintf(stderr,"Name Error%s\n",err.message);
        dbus_error_free(&err);
    }
    if(ret !=DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        return;

	/*注册到目录,记录*/
	char cmd[128] = {0};
	sprintf(cmd, "mkdir -p %s && touch %s/%s", SENSSION_REGISTER_DIR, SENSSION_REGISTER_DIR, listener->bus_name);
	system(cmd);
    while(true) {
        dbus_connection_read_write(connection,100);
        msg =dbus_connection_pop_message (connection);

        if(msg == NULL){
            usleep(1000);
            continue;
        }

        if(dbus_message_is_method_call(msg, SUSPEND_INTERFACE, SUSPEND_FUNCTION)){
            if(strcmp(dbus_message_get_path(msg),OBJECT_PATH) == NULL)
               reply_to_method_call(listener, msg, connection, true);
        } else if(dbus_message_is_method_call(msg, SUSPEND_INTERFACE, RESUME_FUNCTION)){
            if(strcmp(dbus_message_get_path(msg),OBJECT_PATH) == NULL)
               reply_to_method_call(listener, msg, connection, false);
        }
        dbus_message_unref(msg);
    }
}

void *dbus_service_thread(void *data)
{	
	suspend_listener *listener = (suspend_listener *)data;

	listen_dbus(listener);
	pthread_exit(NULL);
}

SUSPEND_API int request_system_suspend(void)
{
	system("pm-suspend");
	return 0;
}

SUSPEND_API int register_suspend_listener(char *bus_name, CALLBACK suspend_callback, CALLBACK resume_callback
)
{
	int rc;
	pthread_t thread_id;
	suspend_listener *listener;

	listener = malloc(sizeof(suspend_listener));
	assert(listener);
	strncpy(listener->bus_name, bus_name, 64);
	listener->bus_name[64] = '\0';
	listener->suspend_callback = suspend_callback;
    listener->resume_callback = resume_callback;

	rc=pthread_create(thread_id, NULL, (void *)dbus_service_thread, (void *)listener);
	if(rc!=0){
		printf("error. return code is %d\n", rc);
		return -1;
	}
	return 0;
}

