#ifndef __DUILITE_H__
#define __DUILITE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (!(defined DUILITE_CALL) || !(defined DUILITE_IMPORT_OR_EXPORT))
	#if defined _WIN32
		#if defined _WIN64
			#define DUILITE_CALL __stdcall
		#else
			#define DUILITE_CALL
		#endif

		#ifdef DUILITE_IMPLEMENTION
			#define DUILITE_IMPORT_OR_EXPORT __declspec(dllexport)
		#else
			#define DUILITE_IMPORT_OR_EXPORT __declspec(dllimport)
		#endif
	#elif defined __ANDROID__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
		#undef  JNIEXPORT
		#define JNIEXPORT __attribute ((visibility("default")))
	#elif defined __APPLE__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
	#elif defined __unix__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT __attribute ((visibility("default")))
	#else
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
	#endif
#endif

#define DUILITE_VERSION           "DUILITE 0.1.4"
#define DUILITE_VERSION_NUM       104

#define DUILITE_MSG_TYPE_JSON     0
#define DUILITE_MSG_TYPE_BINARY   1

enum duilite_callback_type {
	DUILITE_CALLBACK_FESPA_WAKEUP = 0,
	DUILITE_CALLBACK_FESPA_DOA,
	DUILITE_CALLBACK_FESPA_BEAMFORMING,
	DUILITE_CALLBACK_FESPL_WAKEUP,
	DUILITE_CALLBACK_FESPL_DOA,
	DUILITE_CALLBACK_FESPL_BEAMFORMING
};

typedef int (*duilite_callback)(void *userdata, int type, char *msg, int len);

DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_library_load(char *cfg);
DUILITE_IMPORT_OR_EXPORT void DUILITE_CALL duilite_library_release();

struct duilite_vad;
DUILITE_IMPORT_OR_EXPORT struct duilite_vad * DUILITE_CALL duilite_vad_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_start(struct duilite_vad *vad, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_feed(struct duilite_vad *vad, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_stop(struct duilite_vad *vad);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_cancel(struct duilite_vad *vad);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_delete(struct duilite_vad *vad);

struct duilite_speexenc;
DUILITE_IMPORT_OR_EXPORT struct duilite_speexenc * DUILITE_CALL duilite_speexenc_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_start(struct duilite_speexenc *speexenc, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_feed(struct duilite_speexenc *speexenc, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_stop(struct duilite_speexenc *speexenc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_delete(struct duilite_speexenc *speexenc);

struct duilite_echo;
DUILITE_IMPORT_OR_EXPORT struct duilite_echo * DUILITE_CALL duilite_echo_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_start(struct duilite_echo *echo, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_feed(struct duilite_echo *echo, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_stop(struct duilite_echo *echo);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_cancel(struct duilite_echo *echo);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_delete(struct duilite_echo *echo);

struct duilite_wakeup;
DUILITE_IMPORT_OR_EXPORT struct duilite_wakeup * DUILITE_CALL duilite_wakeup_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_start(struct duilite_wakeup *wakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_feed(struct duilite_wakeup *wakeup, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_stop(struct duilite_wakeup *wakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_cancel(struct duilite_wakeup *wakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_delete(struct duilite_wakeup *wakeup);

struct duilite_cntts;
DUILITE_IMPORT_OR_EXPORT struct duilite_cntts * DUILITE_CALL duilite_cntts_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_start(struct duilite_cntts *cntts, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_feed(struct duilite_cntts *cntts, char *data);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_delete(struct duilite_cntts *cntts);

struct duilite_gram;
DUILITE_IMPORT_OR_EXPORT struct duilite_gram * DUILITE_CALL duilite_gram_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gram_start(struct duilite_gram *gram, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gram_delete(struct duilite_gram *gram);

struct duilite_asr;
DUILITE_IMPORT_OR_EXPORT struct duilite_asr * DUILITE_CALL duilite_asr_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_start(struct duilite_asr *asr, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_feed(struct duilite_asr *asr, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_stop(struct duilite_asr *asr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_cancel(struct duilite_asr *asr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_delete(struct duilite_asr *asr);

struct duilite_fespa;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespa * DUILITE_CALL duilite_fespa_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_register(struct duilite_fespa *fespa, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_start(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_feed(struct duilite_fespa *fespa, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_stop(struct duilite_fespa *fespa);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_set(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_delete(struct duilite_fespa *fespa);

struct duilite_fespl;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespl * DUILITE_CALL duilite_fespl_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_register(struct duilite_fespl *fespl, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_start(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_feed(struct duilite_fespl *fespl, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_stop(struct duilite_fespl *fespl);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_set(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_delete(struct duilite_fespl *fespl);

struct duilite_fdm;
DUILITE_IMPORT_OR_EXPORT struct duilite_fdm * DUILITE_CALL duilite_fdm_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_start(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_set(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_get(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_feed(struct duilite_fdm *fdm, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_stop(struct duilite_fdm *fdm);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_delete(struct duilite_fdm *fdm);

struct duilite_nr;
DUILITE_IMPORT_OR_EXPORT struct duilite_nr * DUILITE_CALL duilite_nr_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_start(struct duilite_nr *nr, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_feed(struct duilite_nr *nr, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_stop(struct duilite_nr *nr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nr_delete(struct duilite_nr *nr);


#ifdef __cplusplus
}
#endif

#endif
