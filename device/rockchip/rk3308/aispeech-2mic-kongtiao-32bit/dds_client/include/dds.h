#ifndef __DDS_H__
#define __DDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (!(defined DDS_CALL) || !(defined DDS_IMPORT_OR_EXPORT))
	#if defined _WIN32
		#if defined _WIN64
			#define DDS_CALL __stdcall
		#else
			#define DDS_CALL
		#endif

		#ifdef DDS_IMPLEMENTION
			#define DDS_IMPORT_OR_EXPORT __declspec(dllexport)
		#else
			#define DDS_IMPORT_OR_EXPORT __declspec(dllimport)
		#endif
	#elif defined __ANDROID__
		#define DDS_CALL
		#define DDS_IMPORT_OR_EXPORT
		#undef  JNIEXPORT
		#define JNIEXPORT __attribute ((visibility("default")))
	#elif defined __APPLE__
		#define DDS_CALL
		#define DDS_IMPORT_OR_EXPORT
	#elif defined __unix__
		#define DDS_CALL
		#define DDS_IMPORT_OR_EXPORT __attribute ((visibility("default")))
	#else
		#define DDS_CALL
		#define DDS_IMPORT_OR_EXPORT
	#endif
#endif

#define DDS_VERSION     "DDS 0.2.12"
#define DDS_VERSION_NUM 212

/* callback event */
#define DDS_EV_OUT_RECORD_AUDIO                    1
#define DDS_EV_OUT_NATIVE_CALL                     2
#define DDS_EV_OUT_COMMAND                         3
#define DDS_EV_OUT_MEDIA                           4
#define DDS_EV_OUT_STATUS                          5
#define DDS_EV_OUT_TTS                             6
#define DDS_EV_OUT_ERROR                           7
#define DDS_EV_OUT_ASR_RESULT                      8
#define DDS_EV_OUT_DUI_RESPONSE                    9
#define DDS_EV_OUT_DUI_LOGIN                       10
#define DDS_EV_OUT_CINFO_RESULT                    11

/* external event */
#define DDS_EV_IN_SPEECH                           101
#define DDS_EV_IN_WAKEUP                           102
#define DDS_EV_IN_NATIVE_RESPONSE                  103
#define DDS_EV_IN_RESET                            104
#define DDS_EV_IN_EXIT                             105
#define DDS_EV_IN_CUSTOM_TTS_TEXT                  106
#define DDS_EV_IN_AUDIO_STREAM                     107
#define DDS_EV_IN_PLAYER_STATUS                    108
#define DDS_EV_IN_NLU_TEXT						   109
#define DDS_EV_IN_WAKEUP_WORD					   110
#define DDS_EV_IN_CINFO_OPERATE                    111

/* error id */
#define DDS_ERROR_BASE 1000
#define DDS_ERROR_FATAL     (DDS_ERROR_BASE + 1)
#define DDS_ERROR_TIMEOUT   (DDS_ERROR_BASE + 2)
#define DDS_ERROR_NETWORK   (DDS_ERROR_BASE + 3)
#define DDS_ERROR_SERVER    (DDS_ERROR_BASE + 4)  
#define DDS_ERROR_LOGIC		(DDS_ERROR_BASE + 5)

struct dds_msg;

typedef int (*dds_ev_callback)(void *userdata, struct dds_msg *msg);

struct dds_opt {
	dds_ev_callback _handler;
	void *userdata;
};

DDS_IMPORT_OR_EXPORT int DDS_CALL dds_start(struct dds_msg *conf, struct dds_opt *opt);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_send(struct dds_msg *msg);

/* message pack or unpack */
DDS_IMPORT_OR_EXPORT struct dds_msg * DDS_CALL dds_msg_new();
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_delete(struct dds_msg *msg);
DDS_IMPORT_OR_EXPORT void DDS_CALL dds_msg_print(struct dds_msg *msg);

DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_type(struct dds_msg *msg, int value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_integer(struct dds_msg *msg, const char *key, int value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_double(struct dds_msg *msg, const char *key, double value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_boolean(struct dds_msg *msg, const char *key, int value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_string(struct dds_msg *msg, const char *key, const char *value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_bin(struct dds_msg *msg, const char *key, const char *value, int value_len);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_set_bin_p(struct dds_msg *msg, const char *key, const char *value, int value_len);

DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_type(struct dds_msg *msg, int *value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_integer(struct dds_msg *msg, const char *key, int *value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_double(struct dds_msg *msg, const char *key, double *value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_boolean(struct dds_msg *msg, const char *key, int *value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_string(struct dds_msg *msg, const char *key, char **value);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_bin(struct dds_msg *msg, const char *key, char **value, int *value_len);
DDS_IMPORT_OR_EXPORT int DDS_CALL dds_msg_get_bin_p(struct dds_msg *msg, const char *key, char **value, int *value_len);

#ifdef __cplusplus
}
#endif
#endif
