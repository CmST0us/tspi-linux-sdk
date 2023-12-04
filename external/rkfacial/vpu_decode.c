/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "vpu_decode.h"
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>

int vpu_decode_jpeg_init(struct vpu_decode* decode, int width, int height)
{
    int ret;
    decode->in_width = width;
    decode->in_height = height;

    ret = mpp_buffer_group_get_internal(&decode->memGroup, MPP_BUFFER_TYPE_ION);
    if (MPP_OK != ret) {
        printf("memGroup mpp_buffer_group_get failed\n");
        return ret;
    }

    ret = mpp_create(&decode->mpp_ctx, &decode->mpi);
    if (MPP_OK != ret) {
        printf("mpp_create failed\n");
        return -1;
    }

    ret = mpp_init(decode->mpp_ctx, MPP_CTX_DEC, MPP_VIDEO_CodingMJPEG);
    if (MPP_OK != ret) {
        printf("mpp_init failed\n");
        return -1;
    }

    MppApi* mpi = decode->mpi;
    MppCtx mpp_ctx = decode->mpp_ctx;
    MppFrame frame;
    ret = mpp_frame_init(&frame);
    if (!frame || (MPP_OK != ret)) {
        printf("failed to init mpp frame!");
        return MPP_ERR_NOMEM;
    }

    mpp_frame_set_fmt(frame, MPP_FMT_YUV420SP);
    mpp_frame_set_width(frame, decode->in_width);
    mpp_frame_set_height(frame, decode->in_height);
    mpp_frame_set_hor_stride(frame, MPP_ALIGN(decode->in_width, 16));
    mpp_frame_set_ver_stride(frame, MPP_ALIGN(decode->in_height, 16));

    ret = mpi->control(mpp_ctx, MPP_DEC_SET_FRAME_INFO, (MppParam)frame);
    mpp_frame_deinit(&frame);

    return 0;
}

int vpu_decode_jpeg_doing(struct vpu_decode* decode, void* in_data, RK_S32 in_size,
                          int out_fd, void* out_data)
{
    MPP_RET ret = MPP_OK;
    MppTask task = NULL;

    decode->pkt_size = in_size;
    if (decode->pkt_size <= 0) {
        printf("invalid input size %d\n", decode->pkt_size);
        return MPP_ERR_UNKNOW;
    }

    if (NULL == in_data) {
        ret = MPP_ERR_NULL_PTR;
        goto DECODE_OUT;
    }

    /* try import input buffer and output buffer */
    RK_U32 width = decode->in_width;
    RK_U32 height = decode->in_height;
    RK_U32 hor_stride = MPP_ALIGN(width, 16);
    RK_U32 ver_stride = MPP_ALIGN(height, 16);
    decode->hor_stride = hor_stride;
    decode->ver_stride = ver_stride;
    MppFrame frame = NULL;
    MppPacket packet = NULL;
    MppBuffer str_buf = NULL; /* input */
    MppBuffer pic_buf = NULL; /* output */
    MppCtx mpp_ctx = decode->mpp_ctx;
    MppApi* mpi = decode->mpi;

    ret = mpp_frame_init(&frame);
    if (MPP_OK != ret) {
        printf("mpp_frame_init failed\n");
        goto DECODE_OUT;
    }

    ret = mpp_buffer_get(decode->memGroup, &str_buf, decode->pkt_size);
    if (ret) {
        printf("allocate input picture buffer failed\n");
        goto DECODE_OUT;
    }
    memcpy((RK_U8*)mpp_buffer_get_ptr(str_buf), in_data, decode->pkt_size);

    if (out_fd > 0) {
        MppBufferInfo outputCommit;

        memset(&outputCommit, 0, sizeof(outputCommit));
        /* in order to avoid interface change use space in output to transmit
         * information */
        outputCommit.type = MPP_BUFFER_TYPE_ION;
        outputCommit.fd = out_fd;
        outputCommit.size = hor_stride * ver_stride * 2;
        outputCommit.ptr = out_data;

        ret = mpp_buffer_import(&pic_buf, &outputCommit);
        if (ret) {
            printf("import output stream buffer failed\n");
            goto DECODE_OUT;
        }
    } else {
        ret =
            mpp_buffer_get(decode->memGroup, &pic_buf, hor_stride * ver_stride * 2);
        if (ret) {
            printf("allocate output stream buffer failed\n");
            goto DECODE_OUT;
        }
    }

    mpp_packet_init_with_buffer(&packet, str_buf); /* input */
    mpp_frame_set_buffer(frame, pic_buf);          /* output */
    mpp_packet_set_length(packet, decode->pkt_size);

    // printf("mpp import input fd %d output fd %d\n",
    //        mpp_buffer_get_fd(str_buf), mpp_buffer_get_fd(pic_buf));

    ret = mpi->poll(mpp_ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
    if (ret) {
        printf("mpp input poll failed\n");
        goto DECODE_OUT;
    }

    ret = mpi->dequeue(mpp_ctx, MPP_PORT_INPUT, &task); /* input queue */
    if (ret) {
        printf("mpp task input dequeue failed\n");
        goto DECODE_OUT;
    }

    assert(task);

    mpp_task_meta_set_packet(task, KEY_INPUT_PACKET, packet);
    mpp_task_meta_set_frame(task, KEY_OUTPUT_FRAME, frame);

    ret = mpi->enqueue(mpp_ctx, MPP_PORT_INPUT, task); /* input queue */
    if (ret) {
        printf("mpp task input enqueue failed\n");
        goto DECODE_OUT;
    }

    /* poll and wait here */
    ret = mpi->poll(mpp_ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK);
    if (ret) {
        printf("mpp output poll failed\n");
        goto DECODE_OUT;
    }

    ret = mpi->dequeue(mpp_ctx, MPP_PORT_OUTPUT, &task); /* output queue */
    if (ret) {
        printf("mpp task output dequeue failed\n");
        goto DECODE_OUT;
    }

    assert(task);

    if (task) {
        RK_U32 err_info = 0;
        MppFrame frame_out = NULL;
        decode->fmt = MPP_FMT_YUV420SP;

        mpp_task_meta_get_frame(task, KEY_OUTPUT_FRAME, &frame_out);
        assert(frame_out == frame);

        err_info = mpp_frame_get_errinfo(frame_out);
        if (!err_info) {
            decode->fmt = mpp_frame_get_fmt(frame_out);
            if (MPP_FMT_YUV422SP != decode->fmt && MPP_FMT_YUV420SP != decode->fmt)
                printf("No support USB JPEG decode format!\n");
        }

        ret = mpi->enqueue(mpp_ctx, MPP_PORT_OUTPUT, task);
        if (ret) {
            printf("mpp task output enqueue failed\n");
            goto DECODE_OUT;
        }
        task = NULL;

        if (err_info)
            ret = MPP_NOK;
        else if (MPP_FMT_YUV422SP != decode->fmt && MPP_FMT_YUV420SP != decode->fmt)
            ret = MPP_NOK;
    }

DECODE_OUT:
    if (str_buf) {
        mpp_buffer_put(str_buf);
        str_buf = NULL;
    }

    if (pic_buf) {
        mpp_buffer_put(pic_buf);
        pic_buf = NULL;
    }

    if (frame)
        mpp_frame_deinit(&frame);

    if (packet)
        mpp_packet_deinit(&packet);

    return ret;
}

int vpu_decode_jpeg_done(struct vpu_decode* decode)
{
    MPP_RET ret = MPP_OK;
    ret = mpp_destroy(decode->mpp_ctx);
    if (ret != MPP_OK) {
        printf("something wrong with mpp_destroy! ret:%d\n", ret);
    }

    if (decode->memGroup) {
        mpp_buffer_group_put(decode->memGroup);
        decode->memGroup = NULL;
    }

    return ret;
}
