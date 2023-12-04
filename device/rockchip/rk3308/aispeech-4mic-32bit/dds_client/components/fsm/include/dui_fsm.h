#ifndef DUI_FSM_H
#define DUI_FSM_H
#ifdef __cplusplus
extern "C" {
#endif

//有限状态机实现

//[当前状态]下根据[触发事件]执行[动作]并进入[下一个状态]
typedef struct {
    //触发事件
    int event;
    //当前状态
    int cur_state;
    //执行动作
    void (*action)(void *userdata);
    //下一个状态
    int next_state;
} dui_fsm_transfer_t;

typedef struct {
    //当前状态
    int cur_state;
    //状态迁移表
    dui_fsm_transfer_t *t;
    //状态迁移表大小
    int t_size;
} dui_fsm_t;

void dui_fsm_handle(dui_fsm_t *self, int event, void *userdata);

#ifdef __cplusplus
}
#endif
#endif
