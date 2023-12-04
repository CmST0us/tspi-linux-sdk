/*================================================================
*   Copyright (C) 2018 FREEDOM Ltd. All rights reserved.
*   
*   文件名称：busserver.c
*   创 建 者：chenjie.gu
*   创建日期：2018年05月23日
*   描    述：
*
================================================================*/


#include "busserver.h"
#include "mongoose.h"

#define BUSCLIENT_KEEPALIVE         1
#define BUSCLIENT_RPC_REQUEST       2
#define BUSCLIENT_RPC_RESPONSE      3
#define BUSCLIENT_SEND_TOPIC        4
#define BUSCLIENT_RECV_TOPIC        5

struct mg_connection *conn = NULL;
event_cb user_cb = NULL;
void *user = NULL;

struct message {
    struct multipart bm;
};

static int find_in_str(const char *p)
{
    char *pos = strstr(p, "\r\n");
    if (pos == NULL) {
        return -1;
    }
    return pos - p;
}

static int busclient_parse_multipart_msg(struct mg_connection *nc, struct message *msg, int *type)
{
	const char *pp = nc->recv_mbuf.buf;
	const char *p = pp;
	int len = nc->recv_mbuf.len;
	int handle_len = 0, reserve_len = len, part_len = 0, _len = 0, n = 0;

	for (int i = 0; i < BUSCLIENT_MAX_MULTIPART_OPT; ++i) {
		_len = find_in_str(p);
		part_len = atoi(p);
		if( reserve_len >= _len+2+part_len+2 && strncmp(p+_len,"\r\n",2)==0 && strncmp(p+_len+2+part_len, "\r\n", 2) == 0) {
			msg->bm.part[n].len = part_len;
			msg->bm.part[n].data = p + _len + 2;
			n++;
			msg->bm.n = n;
		} else {
			return -1;
		}
		handle_len = handle_len + _len + 2 + part_len + 2;
		reserve_len = len - handle_len;
		p = pp + handle_len;

		if (reserve_len >=2 && strncmp(p, "\r\n", 2) == 0) {
			if (strncmp(msg->bm.part[0].data, "keepalive", strlen("keepalive")) == 0) {
				*type = BUSCLIENT_KEEPALIVE;
			}
			if (strncmp(msg->bm.part[0].data, "request", strlen("request")) == 0) {
				*type = BUSCLIENT_RPC_REQUEST;
			}
			if (strncmp(msg->bm.part[0].data, "response", strlen("response")) == 0) {
				*type = BUSCLIENT_RPC_RESPONSE;
			}
			if (strncmp(msg->bm.part[0].data, "publish", strlen("publish")) == 0) {
				*type = BUSCLIENT_RECV_TOPIC;
			}
			return handle_len + 2;
		}
	}

	return -1;
}

static void busclient_mg_send(struct mg_connection *nc, const char *data, int len)
{
    char tmp[10] = { 0 };
    int _len = 0;

    _len = sprintf(tmp, "%d", len);
    mg_send(nc, tmp, _len);
    mg_send(nc, "\r\n", 2);
    mg_send(nc, data, len);
    mg_send(nc, "\r\n", 2);
}

static void busclient_multipart_send(struct mg_connection *nc, struct multipart *bm) {
    for (int i = 0; i < bm->n; i++) {
        busclient_mg_send(nc, bm->part[i].data, bm->part[i].len);
    }
    mg_send(nc, "\r\n", 2);
}


static void handle_multipart_msg(struct mg_connection *nc, struct message *msg) {
    if (!strncmp(msg->bm.part[0].data, "request", msg->bm.part[0].len)) {
        if (!strncmp(msg->bm.part[1].data, "/bus/join", msg->bm.part[1].len)) {
            struct multipart m = {1, {
                {strlen("response"), "response"}
            }};

            busclient_multipart_send(nc, &m);
        }
        else if (!strncmp(msg->bm.part[1].data, "/bus/subscribe", msg->bm.part[1].len)) {
            struct multipart m = {1, {
                {strlen("response"), "response"}
            }};

            busclient_multipart_send(nc, &m);
        }
    }

    else if (!strncmp(msg->bm.part[0].data, "publish", msg->bm.part[0].len)) {
        if (!strncmp(msg->bm.part[1].data, "ui.control.topics", msg->bm.part[1].len)) {
            if (user_cb) {
                user_cb("ui.control.topics", msg->bm.part[2].data, msg->bm.part[2].len, user);
            }
        }
    }
}

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  struct mbuf *io = &nc->recv_mbuf;
  (void) p;

  switch (ev) {
    case MG_EV_RECV:
        conn = nc;
      // receive multipart
      int len = 0, type = 0;
      struct message msg;
      while (1) {
          len = busclient_parse_multipart_msg(nc, &msg, &type);
          if (len == -1 || type == 0) break;
          handle_multipart_msg(nc, &msg);
          mbuf_remove(&nc->recv_mbuf, len);
      }
      break;
      
    case MG_EV_TIMER:
      {
          struct multipart *m = (struct multipart *)p;
          busclient_multipart_send(nc, m);
          break;
      }
      
    default:
      break;
  }
}

void busserver_run(const char *addr, event_cb cb, void *u) {
    //
    struct mg_mgr mgr;
    mg_mgr_init(&mgr, NULL);
    mg_bind(&mgr, addr, ev_handler);

    user_cb = cb;
    user = u;

    for (;;) {
        mg_mgr_poll(&mgr, 100);
    }

    mg_mgr_free(&mgr);
}

int busserver_send_msg(const char *topic, const char *data) {

    if (!topic) return -1;

    struct multipart m = {3, {
        {strlen("publish"), "publish"},
            {strlen(topic), topic},
            {strlen(data), data}
    }};

    if (conn)
        ev_handler(conn, MG_EV_TIMER, &m);

    return 0;
}

