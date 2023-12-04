#include "dui_fsm.h"

void dui_fsm_handle(dui_fsm_t *self, int event, void *args) {
    int i;
    for (i = 0; i < self->t_size; i++) {
        if (event == self->t[i].event && self->cur_state == self->t[i].cur_state) {
            self->t[i].action(args);
            self->cur_state = self->t[i].next_state;
            break;
        }
    }
}
