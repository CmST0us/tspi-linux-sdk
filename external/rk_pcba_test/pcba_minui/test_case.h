
#ifndef __TEST_CASE_H__
#define __TEST_CASE_H__

#include<pthread.h>
#include"list.h"

#define CATEGORY_AUTO                   0
#define CATEGORY_MANUAL                 1

#define WAIT_COMPLETION                 0
#define NO_WAIT_COMPLETION              1

struct testcase_base_info
{
    char name[32];
    char display_name[68];
    int activated;
    char binary[20];
    int id;
    int category; /* 0: auto, 1: manual */
    int run_type;
};

struct testcase_info
{
	pthread_t tid;
	int 	err;
	struct testcase_base_info *base_info;
	int x;			//x,y positon and width height on the screen
	int y;
	int w;
	int h;
	int dev_id;		//default 0,but some device have double,such as camera
	int result;
	void *msg;		//this is for testcase spefic msg struct
	void* (*func)(void *argv); //test function
	struct list_head list;
};

#define INIT_CMD_PIPE()                                         \
    FILE *cmd_pipe;                                             \
    int test_case_id;                                           \
    if (argc < 4) {                                             \
        db_error("%s: invalid parameter, #%d\n", argv[0], argc);\
        return -1;                                              \
    }                                                           \
    cmd_pipe = fopen(CMD_PIPE_NAME, "w");                       \
    setlinebuf(cmd_pipe);                                       \
    test_case_id = atoi(argv[3])

#define SEND_CMD_PIPE_OK()                                      \
    fprintf(cmd_pipe, "%d 0\n", test_case_id)

#define SEND_CMD_PIPE_OK_EX(exdata)                             \
    fprintf(cmd_pipe, "%d 0 %s\n", test_case_id, exdata)

#define SEND_CMD_PIPE_FAIL()                                    \
    fprintf(cmd_pipe, "%d 1\n", test_case_id)

#define SEND_CMD_PIPE_FAIL_EX(exdata)                           \
    fprintf(cmd_pipe, "%d 1 %s\n", test_case_id, exdata)

#define EXIT_CMD_PIPE()                                         \
    fclose(cmd_pipe)
    

#endif /* __TEST_CASE_H__ */
