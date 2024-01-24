#include "v4l2.h"

int buf_type = -1;
int current_video_state = 0;

struct v4l2_fmtdesc fmtd[];
struct v4l2_format format;

unsigned char * displaybuf = NULL;
unsigned char * rgb24 = NULL;

int c_OpenDevice(char *video)
{
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtdesc;

    /* open video */
    int videofd = open(video, O_RDWR);
    if ( -1 == videofd ) {
        printf("Error: cannot open %s device\n",video);
        return videofd;
    }
    printf("The %s device was opened successfully.\n", video);

    /* check capability */
    memset(&cap, 0, sizeof(struct v4l2_capability));
    if ( ioctl(videofd, VIDIOC_QUERYCAP, &cap) < 0 ) {
        printf("Error: get capability.\n");
        goto fatal;
    }

    /* query all pixformat */
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE){
        buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }else if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE){
        buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }else{
        printf("Error: application not support this device %s.\n",video);
        goto fatal;
    }
    memset(fmtd, 0, sizeof(fmtd));//note: nedd clean array first.
    fmtdesc.index=0;
    fmtdesc.type=buf_type;
    while(ioctl(videofd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
        fmtd[fmtdesc.index] = fmtdesc;
        // printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
        fmtdesc.index++;
    }
    return videofd;
fatal:
    c_CloseDevice(videofd);
}

int c_FormatDevice(unsigned int pixformat, int videofd)
{
    /* set format */
    memset(&format, 0, sizeof(struct v4l2_format));
    format.type = buf_type;

    if (format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE){
        format.fmt.pix.width = SRC_WIDTH;
        format.fmt.pix.height = SRC_HEIGHT;
        format.fmt.pix.pixelformat = pixformat;
        format.fmt.pix.field = V4L2_FIELD_ANY;
        printf("VIDIO_S_FMT: type=%d, w=%d, h=%d, fmt=0x%x, field=%d\n",
               format.type, format.fmt.pix.width,
               format.fmt.pix.height, format.fmt.pix.pixelformat,
               format.fmt.pix.field);

    }else{
        format.fmt.pix_mp.width = SRC_WIDTH;
        format.fmt.pix_mp.height = SRC_HEIGHT;
        format.fmt.pix_mp.pixelformat = pixformat;
        format.fmt.pix_mp.field = V4L2_FIELD_ANY;
        printf("VIDIO_S_FMT: type=%d, w=%d, h=%d, fmt=0x%x, field=%d\n",
               format.type, format.fmt.pix_mp.width,
               format.fmt.pix_mp.height, format.fmt.pix_mp.pixelformat,
               format.fmt.pix_mp.field);
    }

    if (ioctl(videofd, VIDIOC_S_FMT, &format) < 0) {
        printf("Error: set format %d.\n", errno);
        return errno;
    }

    /* get format */
    if (ioctl(videofd, VIDIOC_G_FMT, &format) < 0) {
        printf("Error: get format %d.\n", errno);
        return errno;
    }
    printf("VIDIO_G_FMT: type=%d, w=%d, h=%d, fmt=0x%x, field=%d\n",
           format.type, format.fmt.pix.width,
           format.fmt.pix.height, format.fmt.pix.pixelformat,
           format.fmt.pix.field);
    return 0;
}


int c_RequestBuffer(buffer *vbuffer, int videofd)
{
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_buffer v4l2_buf;

    /* buffer preparation */
    memset(&reqbuf, 0, sizeof(struct v4l2_requestbuffers));
    reqbuf.count = NB_BUFFER;
    reqbuf.type = buf_type;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(videofd, VIDIOC_REQBUFS, &reqbuf) < 0) {
        printf("Error: request buffer error=%d.\n",errno);
        goto fatal;
    }

    //map buffers
    for (unsigned int i = 0; i < reqbuf.count; i++) {
        memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
        v4l2_buf.index = i;
        v4l2_buf.type = buf_type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            struct v4l2_plane buf_planes[format.fmt.pix_mp.num_planes];
            v4l2_buf.m.planes = buf_planes;
            v4l2_buf.length = format.fmt.pix_mp.num_planes;
        }

        if (ioctl(videofd, VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
            printf("Error: query buffer %d.\n", errno);
            goto fatal;
        }

        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            for (int j = 0; j < format.fmt.pix_mp.num_planes; j++) {
                vbuffer[i * format.fmt.pix_mp.num_planes + j].start
                        = mmap(0, v4l2_buf.m.planes[j].length, PROT_READ,
                               MAP_SHARED, videofd, v4l2_buf.m.planes[j].m.mem_offset);

                vbuffer[i * format.fmt.pix_mp.num_planes + j].length
                        = v4l2_buf.m.planes[j].length;
            }
        }else{//V4L2_BUF_TYPE_VIDEO_CAPTURE
            vbuffer[i].start = mmap(0, v4l2_buf.length, PROT_READ, MAP_SHARED, videofd,v4l2_buf.m.offset);
            vbuffer[i].length = v4l2_buf.length;
        }

        if (vbuffer[i].start == MAP_FAILED) {
            printf("Error: mmap buffers.\n");
            goto fatal;
        }
    }

    //queue buffers
    for (int i = 0; i < reqbuf.count; ++i) {
        memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
        v4l2_buf.index = i;
        v4l2_buf.type = buf_type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            struct v4l2_plane buf_planes[format.fmt.pix_mp.num_planes];
            v4l2_buf.m.planes = buf_planes;
            v4l2_buf.length = format.fmt.pix_mp.num_planes;
        }
        if (ioctl(videofd, VIDIOC_QBUF, &v4l2_buf) < 0) {
            printf("Error: queue buffers, ret:%d i:%d\n", errno, i);
            goto fatal;
        }
    }
    printf("Queue buf done.\n");

    //stream on
    if (ioctl(videofd, VIDIOC_STREAMON, &buf_type) < 0) {
        printf("Error: streamon failed erron = %d.\n",errno);
        goto fatal;
    }
    //open success
    current_video_state = 1;
    return 0;

fatal:
    printf("init camera fial!\n");
    current_video_state = 0;
    c_CloseDevice(videofd);
    return -1;
}

int c_GetBuffer(unsigned char* yuvBuffer, buffer *vbuffer, int videofd)
{
    struct v4l2_buffer v4l2_buf;
    int buf_index = -1;
    int planes_num = format.fmt.pix_mp.num_planes;

    memset(yuvBuffer, 0, SRC_WIDTH * SRC_HEIGHT * 3);

    // dqbuf from video node
    memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
    v4l2_buf.type = buf_type;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        struct v4l2_plane planes[planes_num];
        v4l2_buf.m.planes = planes;
        v4l2_buf.length = planes_num;
    }

    if (ioctl(videofd, VIDIOC_DQBUF, &v4l2_buf) < 0) {
        printf("Error: dequeue buffer, errno %d\n", errno);
        return errno;
    }

    buf_index = v4l2_buf.index;
    if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        for(int i = 0;i < planes_num; i++){
            memcpy(yuvBuffer, vbuffer[buf_index].start, format.fmt.pix_mp.plane_fmt[i].sizeimage);
        }
    }else{// V4L2_BUF_TYPE_VIDEO_CAPTURE
        memcpy(yuvBuffer, vbuffer[buf_index].start, format.fmt.pix.sizeimage);
    }

    if (ioctl(videofd, VIDIOC_QBUF, &v4l2_buf) < 0) {
        printf("Error: queue buffer.\n");
        return errno;
    }
    return 0;
}


void c_CloseDevice(int videofd)
{
    close(videofd);
}

void c_DeintDevice(int videofd, buffer *vbuffer)
{
    struct v4l2_requestbuffers v4l2_rb;

//    if(current_video_state != 1)
//        return;

    if(ioctl(videofd, VIDIOC_STREAMOFF, &buf_type) < 0 ){
        printf("Error: stream close failed erron= %d\n", errno);
        return;
    }

    for (int i = 0; i < NB_BUFFER; i++){
        if((i < NB_BUFFER -1) && (vbuffer[i].length !=  vbuffer[i+1].length))
            munmap (vbuffer[i].start, vbuffer[i+1].length);//first buffer.length maybe not current
        else
            munmap (vbuffer[i].start, vbuffer[i].length);
    }

    memset(&v4l2_rb, 0, sizeof(struct v4l2_requestbuffers));
    v4l2_rb.count = 0;
    v4l2_rb.type = buf_type;
    v4l2_rb.memory = V4L2_MEMORY_MMAP;
    if (ioctl(videofd, VIDIOC_REQBUFS, &v4l2_rb) < 0)
        printf("Error: release buffer error=%d.\n",errno);

    free(vbuffer);
    vbuffer=NULL;
}

void c_NV12_TO_RGB24(unsigned char *data, unsigned char *rgb, int width, int height) {
    int index = 0;
    unsigned char *ybase = data;
    unsigned char *ubase = &data[width * height];
    unsigned char Y, U, V;
    int R, G, B;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            //YYYYYYYYUVUV
            Y = ybase[x + y * width];
            U = ubase[y / 2 * width + (x / 2) * 2];
            V = ubase[y / 2 * width + (x / 2) * 2 + 1];

            R = Y + 1.4075 * (V - 128);
            G = Y - 0.3455 * (U - 128) - 0.7169 * (V - 128);
            B = Y + 1.779 * (U - 128);

            if(R > 255)
                R = 255;
            else if(R < 0)
                R = 0;

            if(G > 255)
                G = 255;
            else if(G < 0)
                G = 0;

            if(B > 255)
                B = 255;
            else if(B < 0)
                B = 0;

            rgb[index++] = R; //R
            rgb[index++] = G; //G
            rgb[index++] = B; //B
        }
    }
}


