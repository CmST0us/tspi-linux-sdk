#ifndef __KEY_TEST_H_
#define __KEY_TEST_H_
void *key_test(void *argc);
extern int set_gkey(unsigned int code);
extern int g_key_test;
extern int manual_p_y;
struct key {
	unsigned int code;
	char *name;
};

struct key_msg {
	int result;
	int x;
	int y;
	int w;
	int h;
};

#endif
