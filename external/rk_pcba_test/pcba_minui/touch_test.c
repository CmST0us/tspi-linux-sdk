#include <pthread.h>

typedef enum {
	TOUCH_START = 0,
	TOUCH_DRAG = 1,
	TOUCH_RELEASE = 2,
	TOUCH_HOLD = 3,
	TOUCH_REPEAT = 4
} TOUCH_STATE;

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#undef ABS
#define ABS(a)		((a) >= 0 ? (a) : (-(a)))

static pthread_mutex_t gUpdateMutex = PTHREAD_MUTEX_INITIALIZER;

int draw_dot(int x, int y)
{
	if(x < 0 || y < 0){
//		LOGE("%s invalid dot! (%d,%d), \n", x, y);
		return -1;
	}

//	LOGE("draw (%d,%d)\n", x, y);
	pthread_mutex_lock(&gUpdateMutex);
//	drawline(0,0,255,255,x,y,2,2);
	gr_color(0, 0, 255, 255);
	gr_fill(x, y, 2, 2);
	//gr_flip();
	pthread_mutex_unlock(&gUpdateMutex);

	//FillColor(0, 0, 255, 255,x, y, 1, 1);


	return 0;
}

int draw_line(int x1, int y1, int x2, int y2)
{
	int x, y;

//	printf("line: (%d,%d)-(%d,%d)\n", x1, y1, x2, y2);

	if(x1 == x2){
		x = x1;
		for(y = MIN(y1, y2); y <= MAX(y1, y2); y++)
			draw_dot(x, y);
	}else if(y1 == y2){
		y = y1;
		for(x = MIN(x1, x2); x <= MAX(x1, x2); x++)
			draw_dot(x, y);
	}else if(ABS(x1-x2) > ABS(y1-y2)){
		for(x = MIN(x1, x2); x <= MAX(x1, x2); x++){
			y = ((y2 - y1) * (x - x1)) / (x2 - x1) + y1;
			draw_dot(x, y);
		}
	}else{
		for(y = MIN(y1, y2); y <= MAX(y1, y2); y++){
                        x = ((x2 - x1) * (y - y1)) / (y2 - y1) + x1;
                        draw_dot(x, y);
                }
	}

	return 0;
}

int last_x = 0, last_y = 0;
#ifdef SOFIA3GR_PCBA
extern int sync_screen_for_prompt(void);
#endif

int NotifyTouch(int action, int x, int y)
{
	switch(action){
	case TOUCH_START:
		draw_dot(x, y);
		last_x = x;
		last_y = y;

		break;
	case TOUCH_DRAG:
		draw_line(last_x, last_y, x, y);
		last_x = x;
		last_y = y;
		pthread_mutex_lock(&gUpdateMutex);
		gr_flip();
		pthread_mutex_unlock(&gUpdateMutex);

		break;
	case TOUCH_RELEASE:
		pthread_mutex_lock(&gUpdateMutex);
                gr_flip();
        pthread_mutex_unlock(&gUpdateMutex);
		#ifdef SOFIA3GR_PCBA
				sync_screen_for_prompt();
		#endif
		break;
	case TOUCH_HOLD:
		break;
	case TOUCH_REPEAT:
		break;
	default:
		break;
	}

	return 0;
}
