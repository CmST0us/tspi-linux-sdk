#ifndef _SCREEN_TEST_H_
#define _SCREEN_TEST_H_

void *screen_test(void *argc);
void screenTest_key_detect(unsigned int code);

extern unsigned char key_press;
extern int tiles_count;

struct  screen_msg {
	int result;
	int x;
	int y;
	int w;
	int h;
};

#endif
