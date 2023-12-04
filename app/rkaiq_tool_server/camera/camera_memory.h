#ifndef _CAMERA_MEMORY_H_
#define _CAMERA_MEMORY_H_

#include <assert.h>
#include <fcntl.h>  /* low-level i/o */
#include <getopt.h> /* getopt_long() */
#include <linux/v4l2-controls.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger/log.h"

#define FMT_NUM_PLANES 1
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define DEV_PATH_LEN 64
#define MAX_MEDIA_INDEX 20

enum io_method
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

enum sensor_link
{
    link_to_vicap,
    link_to_isp,
    link_to_dvp,
};

struct buffer {
    void* start;
    size_t length;
    int socket;
    unsigned short checksum;
};

struct isp_t {
    char media_dev_path[DEV_PATH_LEN];
    char isp_main_path[DEV_PATH_LEN];
    char isp_sd_path[DEV_PATH_LEN];
    int width;
    int height;
    __u32 sd_fmt;
};

struct vicap_t {
    char cif_video_path[DEV_PATH_LEN];
    int width;
    int height;
    __u32 sd_fmt;
};

struct sensor_t {
    char device_name[DEV_PATH_LEN];
    char sensor_name[DEV_PATH_LEN];
    int width;
    int height;
    int bits;
    __u32 sen_fmt;
};

struct lens_t {
    char lens_device_name[DEV_PATH_LEN];
};

struct capture_info {
    const char* dev_name;
    int dev_fd;
    int subdev_fd;
    int lensdev_fd;
    enum io_method io;
    struct buffer* buffers;
    struct isp_t vd_path;
    struct vicap_t cif_path;
    struct sensor_t sd_path;
    struct lens_t lens_path;
    unsigned int n_buffers;
    __u32 format;
    int width;
    int height;
    int lhcg;
    enum sensor_link link;
    enum v4l2_buf_type capture_buf_type;
    int frame_count;
    char out_file[255];
    FILE* out_fp;
};

int xioctl(int fh, int request, void* arg);
void init_read(struct capture_info* cap_info, unsigned int buffer_size);
void init_mmap(struct capture_info* cap_info);
void init_userp(struct capture_info* cap_info, unsigned int buffer_size);

int check_io_method(enum io_method io, unsigned int capabilities);
int init_io_method(struct capture_info* cap_info, unsigned int size);

#endif
