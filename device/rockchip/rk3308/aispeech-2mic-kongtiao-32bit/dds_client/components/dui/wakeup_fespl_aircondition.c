
typedef struct {
    struct timeval major_time;
    struct timeval minor_time;
} wakeup_info_t;

static int wakeup_callback(void *userdata, int type, char *data, int len) {
    wakeup_info_t *info = (wakeup_info_t *)userdata;
    OS_LOG_I(wakeup, "WAKEUP:%s", data);
    cJSON *js = cJSON_Parse(data);
    if (js) {
        dui_msg_t m;
        memset(&m, 0, sizeof(m));
        m.wakeup.index = -1;
        cJSON *major_js = cJSON_GetObjectItem(js, "major");
        cJSON *word_js = cJSON_GetObjectItem(js, "wakeupWord");
        m.wakeup.major = (major_js->valueint != 0) ? true : false;

        int i;
        for (i = 0; i < g_cfg.wakeup.word_count; i++) {
            if (is_same_word(word_js->valuestring, g_cfg.wakeup.word[i])) {
                m.wakeup.index = i;
                break;
            }
        }
        cJSON_Delete(js);
        if (m.wakeup.index != -1) {
            if (m.wakeup.major) {
                m.type = WAKEUP_INFO_WAKEUP;
                m.wakeup.last_major_time = info->major_time;
                gettimeofday(&m.wakeup.cur_major_time, NULL);
                info->major_time = m.wakeup.cur_major_time;
                os_queue_send(process_queue, &m);
                os_queue_send(user_listen_queue, &m);
            } else {
                m.type = WAKEUP_INFO_WAKEUP_MINOR;
                //针对空调应用对算法唤醒输出有过滤
                struct timeval now;
                gettimeofday(&now, NULL);
                if (check_time_expire(&now, &info->major_time, 15000) && check_time_expire(&now, &info->minor_time, 15000)) {

                } else {
                    m.wakeup.last_minor_time = info->minor_time;
                    m.wakeup.cur_minor_time = now;
                    info->minor_time = m.wakeup.cur_minor_time;
                    os_queue_send(process_queue, &m);
                    os_queue_send(user_listen_queue, &m);
                }
            }
        }
    }
    return 0;
}

static int beamforming_callback(void *userdata, int type, char *data, int len) {
    if (type == DUILITE_MSG_TYPE_JSON) {
    } else {
#ifdef SAVE_AUDIO
        fwrite(data, 1, len, output_fd);
#endif
        os_stream_write(vad_stream, data, len);
    }   
    return 0;
}

static int doa_callback(void *userdata, int type, char *data, int len) {
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_INFO_DOA;
    OS_LOG_I(wakeup, "DOA: %s", data);
    cJSON *js = cJSON_Parse(data);
    if (js) {
        cJSON *doa_js = cJSON_GetObjectItem(js, "doa");
	    m.wakeup.doa = doa_js->valueint;
        cJSON_Delete(js);
        os_queue_send(user_listen_queue, &m);
    }
    return 0;
}

static void wakeup_run(void *args) {
    int ret;
    dui_msg_t m;
    wakeup_info_t info;
    memset(&info, 0, sizeof(info));
    //50ms
    int read_buf_size = g_cfg.recorder.channels * g_cfg.recorder.bits / 8 * g_cfg.recorder.samplerate / 20;
    char *read_buf = (char *)os_malloc(read_buf_size);
    assert(read_buf != NULL);
    struct duilite_fespl *fespl_engine = duilite_fespl_new(g_cfg.wakeup.cfg);
    assert(fespl_engine != NULL);
    duilite_fespl_register(fespl_engine, DUILITE_CALLBACK_FESPL_WAKEUP, wakeup_callback, &info);
    duilite_fespl_register(fespl_engine, DUILITE_CALLBACK_FESPL_DOA, doa_callback, NULL);
    duilite_fespl_register(fespl_engine, DUILITE_CALLBACK_FESPL_BEAMFORMING, beamforming_callback, NULL);

    //置位线程READY标志
    os_event_group_set_bits(task_ready_ev, WAKEUP_READY_BIT);
    OS_LOG_I(wakeup, "READY");

    while (1) {
        ret = os_queue_receive(wakeup_queue, &m);
        if (ret == -1) break;
        OS_LOG_I(wakeup, "%s", dui_msg_table[m.type]);
        if (m.type == WAKEUP_CMD_START) {
            OS_LOG_I(wakeup, "START");
            //注意：duilite_fespl_start内部会清空已有数据，避免数据乱序
            duilite_fespl_start(fespl_engine, g_cfg.wakeup.param);
            int read_bytes;
            while (1) {
                read_bytes = os_stream_read(wakeup_stream, read_buf, read_buf_size);
                if (read_bytes == -1) break;        //录音缓冲被终止
#ifdef SAVE_AUDIO
                fwrite(read_buf, 1, read_bytes, input_fd);
#endif
                duilite_fespl_feed(fespl_engine, read_buf, read_bytes);
            }
        } else if (m.type == WAKEUP_CMD_STOP) {
            os_queue_send(user_listen_queue, &m);
            OS_LOG_I(wakeup, "STOP");
        }
    }
    os_free(read_buf);
    duilite_fespl_delete(fespl_engine);
    OS_LOG_I(wakeup, "EXIT");
}
