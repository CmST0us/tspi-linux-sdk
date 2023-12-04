#ifndef RK_WAKE_LOCK_H
#define RK_WAKE_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

struct rk_wake_lock;
struct rk_wake_lock* RK_wake_lock_new(const char *id);//the id must be unique
void RK_wake_lock_delete(struct rk_wake_lock* lock);

int RK_acquire_wake_lock(struct rk_wake_lock* lock);
int RK_release_wake_lock(struct rk_wake_lock* lock);
int RK_wait_all_wake_lock_release(int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif

