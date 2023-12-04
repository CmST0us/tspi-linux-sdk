#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "eq_log.h"
#include "Rk_wake_lock.h"

struct rk_wake_lock {
	char *id;
};

#define TAG "WAKE_LOCK"

#define debug(fmt, args...) \
    eq_debug(TAG "[%s] " fmt "\n", __func__, ##args)

enum {
	ACQUIRE_PARTIAL_WAKE_LOCK = 0,
	RELEASE_WAKE_LOCK,
	OUR_FD_COUNT
};

const char * const WAKE_LOCK_PATHS[] = {
	"/sys/power/wake_lock",
	"/sys/power/wake_unlock",
};

static int g_initialized = 0;
static int g_fds[2];
static int g_error = -1;

static int open_file_descriptors(const char * const paths[])
{
	for (int i = 0 ; i < 2; i++) {
		int fd = open(paths[i], O_RDWR | O_CLOEXEC);
		if (fd < 0) {
			g_error = -errno;
			debug("fatal error opening \"%s\": %s", paths[i],
				  strerror(errno));
			return -1;
		}
		g_fds[i] = fd;
	}

	g_error = 0;
	return 0;
}

static inline void initialize_fds(void) {
	if (g_initialized == 0) {
		open_file_descriptors(WAKE_LOCK_PATHS);
		g_initialized = 1;
	}
}

struct rk_wake_lock* RK_wake_lock_new(const char *id) {
	struct rk_wake_lock* lock;
	if (!id)
		return NULL;
	initialize_fds();

	lock = (struct rk_wake_lock*)malloc(sizeof(struct rk_wake_lock));
	lock->id = strdup(id);
	return lock;
}

void RK_wake_lock_delete(struct rk_wake_lock* lock) {
	if (lock && lock->id)
		free(lock->id);
	if (lock)
		free(lock);
}

int RK_acquire_wake_lock(struct rk_wake_lock *wake_lock) {
	char *id = wake_lock->id;
	debug("id=%s", id);

	if (g_error)
		return g_error;

	int fd;
	size_t len;
	ssize_t ret;

	fd = g_fds[0];

	ret = write(fd, id, strlen(id));
	if (ret < 0) {
		return -errno;
	}

	return ret;
}

int RK_release_wake_lock(struct rk_wake_lock *wake_lock) {
	char *id = wake_lock->id;
	int fd;

	debug("id=%s", id);

	if (g_error)
		return g_error;

	fd = g_fds[1];

	ssize_t len = write(fd, id, strlen(id));
	if (len < 0) {
		return -errno;
	}
	return len;
}

static int exec(const char *cmd, char *buf, const size_t size) {
	FILE *stream = NULL;
	char tmp[1024];

	if ((stream = popen(cmd,"r")) == NULL) {
		return -1;
	}

	if (buf == NULL) {
		pclose(stream);
		return -2;
	}

	buf[0] = '\0';
	while (fgets(tmp, sizeof(tmp) -1, stream)) {
		if (strlen(buf) + strlen(tmp) >= size) {
			pclose(stream);
			return -3;
		}
		strcat(buf, tmp);
	}
	pclose(stream);

	return 0;
}

int RK_wait_all_wake_lock_release(int timeout_ms) {
    int cnt = timeout_ms / 10;
	char result[1024];

	while(cnt--) {
		memset(result, 0, 1024);
		exec("cat /sys/power/wake_lock", result, 1024);
		if (strlen(result) <= 1) //换行符
			break;
		usleep(10*1000);
		debug("## suspend waiting wake_lock: %s", result);
	}
	if (cnt == 0) {
		debug("wait suspend timeout, abort suspend");
		debug("unreleased wake_lock: %s", result);
		return -1;
	}
	return 0;
}

