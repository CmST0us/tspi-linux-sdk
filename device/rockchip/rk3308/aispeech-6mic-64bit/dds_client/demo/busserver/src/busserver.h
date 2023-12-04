#ifndef BUSSERVER_H_
#define BUSSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

struct bc_str {
	int len;
	const char *data;
};

#define BUSCLIENT_MAX_MULTIPART_OPT 20

struct multipart {
	int n;
    // int capacity;
	struct bc_str part[BUSCLIENT_MAX_MULTIPART_OPT];
};

typedef void (*event_cb)(const char *topic, const char *topic_data, int data_len, void *user);

void busserver_run(const char *addr, event_cb, void *user);

int busserver_send_msg(const char *topic, const char *data);

#ifdef __cplusplus
}
#endif
#endif

