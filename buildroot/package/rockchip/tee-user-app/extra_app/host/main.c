#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <tee_client_api.h>
#include <tee_api_defines_extensions.h>
#include "ta_keybox.h"

#define KEY_SIZE 32
#define DEFAULT_STORAGE		TEE_STORAGE_PRIVATE_RPMB

static int run_system(const char *command)
{
	int status;

	status = system(command);
	if (status == -1) {
		printf("ERROR: system fork failed\n");
		return -1;
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status)) {
				printf("ERROR: system command failed\n");
				return -1;
			}
		}
	}

	return 0;
}

static int grep_string(const char *dst, const char *target)
{
	int i;

	for (i = 0; i < strlen(dst); i++) {
		if (!memcmp(&dst[i], target, strlen(target)))
			return 0;
	}
	return -1;
}

static int check_ramdisk_env(const char *file)
{
	FILE *fp;
	char buf[1000];

	fp = fopen(file, "r");
	if (!fp) {
		printf("ERROR: bad node\n");
		return -1;
	}

	while (!feof(fp)) {
		fgets(buf, 1000, fp);
		if (!grep_string(buf, "none / rootfs")) {
			fclose(fp);
			return 0;
		}
	};

	fclose(fp);
	return -1;
}

static void dump_hex(char *var_name, const uint8_t *data,
	      uint32_t len)
{
#ifdef DEBUG_DUMP
	uint32_t i;

	printf("LINE:%d  %s:", __LINE__, var_name);
	for (i = 0; i < len; i++) {
		if ((i % 16) == 7)
			printf(" ");
		if ((i % 16) == 0)
			printf("\n");
		printf("0x%02x", data[i]);
	}
	printf("\n");
#endif
}

static uint32_t js_hash(uint32_t hash, uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		hash ^= ((hash << 5) + buf[i] + (hash >> 2));

	return hash;
}

static int process_key(int cmd, char *key)
{
	TEEC_Result res = TEEC_SUCCESS;
	uint32_t error_origin = 0;
	TEEC_Context contex;
	TEEC_Session session;
	TEEC_Operation operation;
	TEEC_SharedMemory sm;
	const TEEC_UUID uuid = TA_KEYBOX_UUID;

	/* [1] Connect to TEE */
	res = TEEC_InitializeContext(NULL, &contex);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_InitializeContext failed with code 0x%x\n", res);
		return res;
	}

	/* [2] Open seesion with TEE application */
	res = TEEC_OpenSession(&contex, &session, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &error_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_Opensession failed with code 0x%x origin 0x%x\n",
		       res, error_origin);
		goto out;
	}

	/* [3] Perform operation initialization */
	memset(&operation, 0, sizeof(TEEC_Operation));
	if (cmd == TA_KEY_READ) {
		sm.size = KEY_SIZE;
		sm.flags = TEEC_MEM_OUTPUT;

		res = TEEC_AllocateSharedMemory(&contex, &sm);
		if (res != TEEC_SUCCESS) {
			printf("AllocateSharedMemory ERR! res = 0x%x\n", res);
			goto out1;
		}

		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_OUTPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
		operation.params[0].memref.parent = &sm;
		operation.params[0].memref.offset = 0;
		operation.params[0].memref.size = sm.size;

		res = TEEC_InvokeCommand(&session, TA_KEY_RNG, &operation, &error_origin);
		if (res != TEEC_SUCCESS) {
			printf("InvokeCommand ERR! res = 0x%x\n", res);
			TEEC_ReleaseSharedMemory(&sm);
			goto out1;
		}
		dump_hex("RNG -> ", sm.buffer, KEY_SIZE);

		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
		operation.params[0].value.a = js_hash(0x47c6a7e6, sm.buffer, KEY_SIZE);

		res = TEEC_InvokeCommand(&session, TA_KEY_VER, &operation, &error_origin);
		if (res != TEEC_SUCCESS) {
			printf("InvokeCommand ERR! res = 0x%x\n", res);
			TEEC_ReleaseSharedMemory(&sm);
			goto out1;
		}
		//printf("HASH -> %x", operation.params[0].value.a);

		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_OUTPUT,
							TEEC_VALUE_INPUT,
							TEEC_NONE,
							TEEC_NONE);
		operation.params[0].memref.parent = &sm;
		operation.params[0].memref.offset = 0;
		operation.params[0].memref.size = sm.size;
		operation.params[1].value.a = DEFAULT_STORAGE;
		res = TEEC_InvokeCommand(&session, TA_KEY_READ, &operation, &error_origin);
		if (res != TEEC_SUCCESS) {
			printf("InvokeCommand ERR! res = 0x%x\n", res);
			TEEC_ReleaseSharedMemory(&sm);
			goto out1;
		}
		 dump_hex("read key -> ", sm.buffer, KEY_SIZE);

		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_OUTPUT,
							TEEC_VALUE_INPUT,
							TEEC_NONE,
							TEEC_NONE);
		operation.params[0].memref.parent = &sm;
		operation.params[0].memref.offset = 0;
		operation.params[0].memref.size = sm.size;
		operation.params[1].value.a = DEFAULT_STORAGE;
		res = TEEC_InvokeCommand(&session, TA_KEY_READ, &operation, &error_origin);
		if (res != TEEC_SUCCESS) {
			printf("InvokeCommand ERR! res = 0x%x\n", res);
			TEEC_ReleaseSharedMemory(&sm);
			goto out1;
		}
		dump_hex("read key -> ", sm.buffer, KEY_SIZE);

		memcpy(key, sm.buffer, KEY_SIZE);

		TEEC_ReleaseSharedMemory(&sm);
	} else {
		operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
							TEEC_VALUE_INPUT,
							TEEC_NONE,
							TEEC_NONE);

		operation.params[0].tmpref.size = KEY_SIZE;
		operation.params[0].tmpref.buffer = (void *)key;
		operation.params[1].value.a = DEFAULT_STORAGE;

		res = TEEC_InvokeCommand(&session, TA_KEY_WRITE, &operation, &error_origin);
		if (res != TEEC_SUCCESS) {
			printf("InvokeCommand ERR! res = 0x%x\n", res);
		}
	}

out1:
	TEEC_CloseSession(&session);
out:
	TEEC_FinalizeContext(&contex);
	return (res == TEEC_SUCCESS) ? 0 : -1;
}

/* 1 -> string to ascii */
static char transform_byte(int direction, uint8_t c)
{
	if (direction) {
		if (c > 'f')
			return -1;
		else if (c >= 'a')
			return c - 'a' + 10;
		else if (c > 'F')
			return -1;
		else if (c >= 'A')
			return c - 'A' + 10;
		else if (c > '9')
			return -1;
		else if (c >= '0')
			return c - '0';
		else
			return -1;
	} else {
		if (c > 9)
			return c + 'a' - 10;
		else
			return c + '0';
	}
}

static int transform_key_to_ascii(int direction, uint8_t *in, uint8_t *out, int size)
{
	int i;
	uint8_t tmp;

	for (i = 0; i < size; i++) {
		if (direction) {
			out[i] = (transform_byte(1, in[i << 1]) << 4) | transform_byte(1, in[(i << 1) + 1]);
		} else {
			out[i << 1] = transform_byte(0, in[i] >> 4);
			out[(i << 1) + 1] = transform_byte(0, in[i] & 0x0f);
		}
	}
}

static int process_recovery(int argc, char *argv[])
{
	int fd, ret;
	uint8_t pw[KEY_SIZE];
	uint8_t t_pw[KEY_SIZE << 1];

	fd = open("/tmp/syspw", O_RDONLY);
	if (fd == -1) {
		printf("ERROR: failed to open syspw\n");
		return -1;
	}

	ret = read(fd, t_pw, KEY_SIZE << 1);
	if (ret != (KEY_SIZE << 1)) {
		printf("ERROR: bad pw length\n");
		return -1;
	}

	transform_key_to_ascii(1, t_pw, pw, KEY_SIZE);

	dump_hex("pw -> ", pw, KEY_SIZE);
	close(fd);
#ifdef CA_DEBUG
	return process_key((argc > 1) ? TA_KEY_READ : TA_KEY_WRITE,
			   pw);
#else
	return process_key(TA_KEY_WRITE, pw);
#endif
}

static int process_ramfs(void)
{
	int fd, ret;
	uint8_t pw[KEY_SIZE];
	uint8_t t_pw[KEY_SIZE << 1];

	if (process_key(TA_KEY_READ, pw))
		return -1;

	fd = open("/tmp/syspw", O_WRONLY | O_CREAT);
	if (fd == -1) {
		printf("ERROR: failed to open syspw\n");
		return -1;
	}

	ftruncate(fd, 0);
	lseek(fd, 0, SEEK_SET);

	transform_key_to_ascii(0, pw, t_pw, KEY_SIZE);
	ret = write(fd, t_pw, KEY_SIZE << 1);
	if (ret != (KEY_SIZE << 1)) {
		printf("ERROR: failed to write syspw (ret = %d)\n", ret);
		return -1;
	}

	close(fd);
	return 0;
}

int main(int argc, char *argv[])
{
#ifdef CA_DEBUG
	return (argc > 1) ? process_ramfs() : process_recovery(argc, argv);
#else
	int status;

	/* check run under pid 1(init) */
	if ((getppid() != 1) || (argc > 1)) {
		return process_recovery(argc, argv);
	}

	/* check proc has mounted */
	if (run_system("/bin/mount -t proc proc /proc")) {
		printf("ERROR: failed to run system command\n");
		return -1;
	}

	/* check run at ramdisk */
	if (run_system("cat /proc/mounts > /tmp/proc_test")) {
		printf("ERROR: failed to run system command\n");
		return -1;
	}

	status = check_ramdisk_env("/tmp/proc_test");
	remove("/tmp/proc_test");
	if (status)
		return -1;

	return process_ramfs();
#endif
}
