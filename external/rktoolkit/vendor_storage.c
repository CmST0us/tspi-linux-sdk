/* V2.1:
 *	1. remove VENDOR_SN_ID len limit
 *	2. add custom id
 *	3. exten max vendor string len to 1024
 *	4. support file read/write
 *	5. support build a library
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vendor_storage.h"

#define VENDOR_STORAGE_DEBUG
#ifdef VENDOR_STORAGE_DEBUG
#define DEBUG(fmt, args...)	fprintf(stdout, "[Debug] "fmt, ##args)
#else
#define DEBUG(fmt, args...)
#endif
#define INFO(fmt, args...)	fprintf(stdout, "[INFO] "fmt, ##args)
#define ERROR(fmt, args...)	fprintf(stderr, "[ERROR] "fmt, ##args)

typedef		unsigned short	    uint16;
typedef		unsigned int	    uint32;
typedef		unsigned char	    uint8;

#define VENDOR_MAX_SIZE		1024
#define VENDOR_REQ_TAG		0x56524551
#define VENDOR_READ_IO		_IOW('v', 0x01, unsigned int)
#define VENDOR_WRITE_IO		_IOW('v', 0x02, unsigned int)

#define VENDOR_ID_MAX	16
#define VENDOR_CMD_CUSTOM_LEN	sizeof("VENDOR_CUSTOM_ID")
static char *vendor_id_table[] = {
	"VENDOR_SN_ID",
	"VENDOR_WIFI_MAC_ID",
	"VENDOR_LAN_MAC_ID",
	"VENDOR_BT_MAC_ID",
	"VENDOR_HDCP_14_HDMI_ID",
	"VENDOR_HDCP_14_DP_ID",
	"VENDOR_HDCP_2x_ID",
	"VENDOR_DRM_KEY_ID",
	"VENDOR_PLAYREADY_Cert_ID",
	"VENDOR_ATTENTION_KEY_ID",
	"VENDOR_PLAYREADY_ROOT_KEY_0_ID",
	"VENDOR_PLAYREADY_ROOT_KEY_1_ID",
	"VENDOR_SENSOR_CALIBRATION_ID",
	"VENODR_RESERVE_ID_14",
	"VENDOR_IMEI_ID",
	"VENDOR_CUSTOM_ID" /* CUSTOM_ID must be last one */
};

#define VENDOR_PR_HEX		0
#define VENDOR_PR_STRING	1

/* Set custom_id to hex print default */
#define GET_PR_FORMAT(ID, FORMAT) \
	if ((ID) == VENDOR_IMEI_ID || (ID) == VENDOR_SN_ID) \
		FORMAT = VENDOR_PR_STRING; \
	else \
		FORMAT = VENDOR_PR_HEX;

struct rk_vendor_req {
	uint32 tag;
	uint16 id;
	uint16 len;
	uint8 data[1024];
};

#ifndef BUILD_LIB_VENDOR_STORAGE
static char *argv0;

static void rknand_print_string_data(uint8 *s, struct rk_vendor_req *buf, uint32 len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		printf("%c", buf->data[i]);
	fprintf(stdout, "\n");
}

static void rknand_print_hex_data(uint8 *s, struct rk_vendor_req *buf, uint32 len)
{
	unsigned int i, line;

	line = 0;
	for (i = 0; i < len; i++) {
		if (i & 0x000f) {
			printf(" %02x", buf->data[i]);
		} else {
			printf("\n %04x-%04x: %02x",
			       line << 4,
			       (line << 4) | 0xf,
			       buf->data[i]);
			line++;
		}
	}

	fprintf(stdout, "\n");
}

static void rknand_print_data(uint8 *s, struct rk_vendor_req *buf,
				  uint32 len, int pr_type)
{
	DEBUG("%s\n",s);
	DEBUG("tag = %d // id = %d // len = %d // data = %p\n", buf->tag, buf->id, buf->len, buf->data);

	INFO("%s:", (buf->id > VENDOR_ID_MAX) ?
		       "VENDOR_CUSTOM_ID" : vendor_id_table[buf->id - 1]);

	if (pr_type)
		rknand_print_string_data(s, buf, len);
	else
		rknand_print_hex_data(s, buf, len);
}

static int vendor_storage_read(int cmd, int pr_type, char *output)
{
	uint32 i;
	int ret ;
	uint8 p_buf[sizeof(struct rk_vendor_req)]; /* malloc req buffer or used extern buffer */
	struct rk_vendor_req *req;
	FILE *foutput = NULL;

	DEBUG("%s id = %d\n", __func__, cmd);

	req = (struct rk_vendor_req *)p_buf;
	memset(p_buf, 0, 100);
	int sys_fd = open("/dev/vendor_storage", O_RDWR, 0);
	if(sys_fd < 0){
		ERROR("vendor_storage open fail\n");
		return -1;
	}

	req->tag = VENDOR_REQ_TAG;
	req->id = cmd;
	req->len = VENDOR_MAX_SIZE;

	ret = ioctl(sys_fd, VENDOR_READ_IO, req);

	if(ret){
		ERROR("vendor read error %d\n", ret);
		return -1;
	}
	close(sys_fd);

	rknand_print_data("vendor read:", req, req->len, pr_type);
	if (output) {
		foutput=fopen(output,"wb");
		if (!foutput) {
			ERROR("failed to save %s\n", output);
			return 0;
		}
		fwrite(req->data, req->len, 1, foutput);
		fclose(foutput);
		INFO("save output to %s\n", output);
	}

	return 0;
}

static int vendor_storage_write(int cmd, char *num, int pr_type, int len)
{
	uint32 i;
	int ret ;
	uint8 p_buf[sizeof(struct rk_vendor_req)]; /* malloc req buffer or used extern buffer */
	struct rk_vendor_req *req;

	DEBUG("%s id = %d\n", __func__, cmd);
	req = (struct rk_vendor_req *)p_buf;
	int sys_fd = open("/dev/vendor_storage",O_RDWR,0);
	if(sys_fd < 0){
		ERROR("vendor_storage open fail\n");
		return -1;
	}

	req->tag = VENDOR_REQ_TAG;
	req->id = cmd;

	req->len = len;
	DEBUG("%s: strlen = %d\n", __func__, req->len);
	memcpy(req->data, num, req->len);

	ret = ioctl(sys_fd, VENDOR_WRITE_IO, req);
	if(ret){
		ERROR("vendor write error\n");
		return -1;
	}

	rknand_print_data("vendor write:", req, req->len, pr_type);
	return 0;
}

static void usage(void)
{
	int i;

	fprintf(stderr,
		"vendor storage tool - Revision: 2.0 \n\n"
		"%s [-r/w <vendor_id> -t <pr_type> -i <input>] [-R]\n"
		"  -r           Read specify vendor_id\n"
		"  -R           Read common vendor_id\n"
		"  -w           Write specify vendor_id\n"
		"  -t           print type\n"
		"  -i           input string\n"
		"  <vendor_id>  There are %d types\n",
		argv0, VENDOR_ID_MAX);
	for (i = 0; i < VENDOR_ID_MAX; i++)
		fprintf(stderr,
		"               \"%s\"\n",
		vendor_id_table[i]);
	fprintf(stderr,
		"               And custom can define other id like\n"
		"               VENDOR_CUSTOM_ID_1A (define ID = 26)\n");
	fprintf(stderr,
		"  <pr_type>    In write case, used with -i <input>\n"
		"               There are 3 types\n"
		"               \"hex\": <input> must be hex form like 0123\n"
		"               \"string\": <input> must be ASCII string\n"
		"               \"file\": <input> must be path to file\n"
		"               Note: If use \"file\" and -i with read, it means save storage to file\n"
		"Examples:\n"
		"  %s -w VENDOR_CUSTOM_ID_1A -t file -i /userdata/test.bin\n"
		"                       write userdata/test.bin to storage\n"
		"                       Or -t string -i test_storage\n"
		"                       write string \"test_storage\" to storage\n"
		"                       ID = 26\n"
		"  %s -r VENDOR_CUSTOM_ID_1A -t file -i /userdata/read.bin\n"
		"                       read storage(ID=26) to userdata/read.bin\n"
		"                       Or -t string\n"
		"                       print storage(ID=26) with ASCII string\n",
		argv0, argv0);
	exit(1);
}

static int vendor_len_mask(int cmd, int len, int cnt)
{
	if (cnt != len) {
		ERROR("%s must be %d bytes!!!\n",
		       vendor_id_table[cmd - 1], len);
		return -1;
	}
	return 0;
}

static bool is_hex(char c)
{
	if (c < '0' || (c > '9' && c < 'A') ||
	    (c > 'F' && c < 'a') || c > 'f')
		return false;
	return true;
}

static char char_to_hex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - ('a' - 10);
	else
		return c - ('A' - 10);
}

static int hex_string_format(char *str, char *hex_str)
{
	int i, tmp;

	tmp = strlen(str);
	if (tmp & 1)
		return 0;

	for (i = 0; i < tmp; i++) {
		if (!is_hex(str[i])) {
			ERROR("[%s] must be HEX input\n", __func__);
			return 0;
		}

		/* string to hex */
		str[i] = char_to_hex(str[i]);
		if (i & 1)
			hex_str[(i - 1) >> 1] = (str[i - 1] << 4) | str[i];
	}
	hex_str[i >> 1] = 0;

	return i >> 1;
}

static int vendor_get_custom_id(char *cmd)
{
	int id;

	/* Check vendor_custom_id */
	if (cmd[VENDOR_CMD_CUSTOM_LEN - 1] != '_' ||
	    !is_hex(cmd[VENDOR_CMD_CUSTOM_LEN]) ||
	    !is_hex(cmd[VENDOR_CMD_CUSTOM_LEN + 1])) {
		goto head_error;
	}

	id = (char_to_hex(cmd[VENDOR_CMD_CUSTOM_LEN]) << 4) |
	     char_to_hex(cmd[VENDOR_CMD_CUSTOM_LEN + 1]);

	return id;
head_error:
	return -1;
}

static int vendor_storage_get_id(char *cmd)
{
	int i, id;

	for (i = 0; i < VENDOR_ID_MAX; i++) {
		if (!memcmp(optarg, vendor_id_table[i], strlen(vendor_id_table[i]))) {
			if (i == (VENDOR_CUSTOM_ID - 1)) {
				id = vendor_get_custom_id(optarg);
				if (id < 0) {
					usage();
					return -1;
				}
			} else {
				id = i + 1;
			}
			break;
		}
	}

	if (i == VENDOR_ID_MAX)
		return -1;

	return id;
}

#define OPTION_FLAG_R		(0)
#define OPTION_FLAG_W		(!0)
int main(int argc, char **argv)
{
	int opt, i;
	int id = -1;
	int pr_type = -1;
	int flag_rw, flag_file = 0;
	unsigned char *vendor_hex = NULL;
	FILE *finput = NULL;
	long int size;

	argv0 = argv[0];
	while ((opt = getopt(argc, argv, "hRr:w:t:i:")) > 0) {
		switch (opt) {
		case 'r':
			id = vendor_storage_get_id(optarg);
			if (id < 0) {
				ERROR("form_error, check cmd with -h\n");
				return -1;
			}
			flag_rw = OPTION_FLAG_R;
			break;
		case 'R':
			/* Read Common Vendor ID */
			for (i = 0; i < VENDOR_HDCP_2x_ID; i++) {
				GET_PR_FORMAT(i + 1, pr_type);
				vendor_storage_read(i + 1, pr_type, NULL);
			}
			return 0;
			break;
		case 'w':
			id = vendor_storage_get_id(optarg);
			if (id < 0) {
				ERROR("form_error, check cmd with -h\n");
				return -1;
			}
			flag_rw = OPTION_FLAG_W;
			break;

		case 't':
			if (!memcmp(optarg, "string", strlen("string"))) {
				pr_type = VENDOR_PR_STRING;
			} else if (!memcmp(optarg, "hex", strlen("hex"))) {
				pr_type = VENDOR_PR_HEX;
			} else {
				pr_type = VENDOR_PR_HEX;
				flag_file = 1;
			}
			break;
		case 'i':
			vendor_hex = strdup(optarg);
			DEBUG("intput = %s\n", vendor_hex);
			break;
		case 'h':
			usage();
			break;
		default:
			ERROR("Unknown option: %c\n", opt);
			usage();
			break;
		}
	}

	if (id < 0) {
		ERROR("Set id first\n");
		goto error;
	}

	if (id <= VENDOR_HDCP_2x_ID) {
		GET_PR_FORMAT(id, pr_type);
	}

	if (pr_type < 0) {
		INFO("Set hex output default\n");
		pr_type = VENDOR_PR_HEX;
	}

	if (!vendor_hex && (flag_rw & OPTION_FLAG_W)) {
		ERROR("No input\n");
		goto error;
	}

	if (flag_rw == OPTION_FLAG_R) {
		vendor_storage_read(id, pr_type,
				    flag_file ? vendor_hex: NULL);
	} else {
		if (flag_file) {
			finput = fopen(vendor_hex, "rb");
			if (!finput) {
				ERROR("Can't open %s\n", vendor_hex);
				goto error;
			}
			free(vendor_hex);

			fseek(finput, 0, SEEK_END);
			size = ftell(finput);
			DEBUG("size = %d\n", size);
			fseek(finput, 0, SEEK_SET);
			vendor_hex = malloc(size + 1);
			fread(vendor_hex, 1, size, finput);
		} else if (pr_type == VENDOR_PR_HEX) {
			size = hex_string_format(vendor_hex, vendor_hex);
			if (!size) {
				ERROR("input is not hex form\n");
				goto error;
			}
		} else if (pr_type == VENDOR_PR_STRING) {
			size = strlen(vendor_hex);
		}

		vendor_storage_write(id, vendor_hex, pr_type, size);
	}

	if (finput)
		fclose(finput);
	if (vendor_hex)
		free(vendor_hex);

	return 0;

error_file:
	if (finput)
		fclose(finput);
error:
	if (vendor_hex)
		free(vendor_hex);
	return -1;
}
#endif

#ifdef BUILD_LIB_VENDOR_STORAGE
int rkvendor_read(int vendor_id, char *data, int size)
{
	int ret ;
	uint8 p_buf[sizeof(struct rk_vendor_req)]; /* malloc req buffer or used extern buffer */
	struct rk_vendor_req *req;

	req = (struct rk_vendor_req *)p_buf;
	memset(p_buf, 0, sizeof(struct rk_vendor_req));
	int sys_fd = open("/dev/vendor_storage", O_RDONLY);
	if(sys_fd < 0){
		fprintf(stderr, "vendor_storage open fail\n");
		return -1;
	}

	req->tag = VENDOR_REQ_TAG;
	req->id = vendor_id;
	req->len = VENDOR_MAX_SIZE;

	ret = ioctl(sys_fd, VENDOR_READ_IO, req);
	close(sys_fd);
	if (ret) {
		fprintf(stderr, "vendor read error %d\n", ret);
		return -1;
	}

	if ( size < req->len ) {
		fprintf(stderr, "vendor storage: param size is lower then read size %d\n", strlen(req->data) );
		return -1;
	}

	memcpy(data, req->data, req->len);
	return 0;
}

int rkvendor_write(int vendor_id, const char *data, int size)
{
	int ret ;
	uint8 p_buf[sizeof(struct rk_vendor_req)]; /* malloc req buffer or used extern buffer */
	struct rk_vendor_req *req;

	if (size > VENDOR_MAX_SIZE) {
		fprintf(stderr, "vendor storage input data overflow\n");
		return -1;
	}

	req = (struct rk_vendor_req *)p_buf;
	int sys_fd = open("/dev/vendor_storage",O_RDWR,0);
	if (sys_fd < 0) {
		fprintf(stderr, "vendor storage open fail\n");
		return -1;
	}

	req->tag = VENDOR_REQ_TAG;
	req->id = vendor_id;

	req->len = size;
	memcpy(req->data, data, req->len);

	ret = ioctl(sys_fd, VENDOR_WRITE_IO, req);
	close(sys_fd);
	if (ret) {
		fprintf(stderr, "vendor write error\n");
		return -1;
	}

	return 0;
}
#endif
