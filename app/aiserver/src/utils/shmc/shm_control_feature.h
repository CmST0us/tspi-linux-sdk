
#ifndef SHM_CONTROL_FEATURE_H_
#define SHM_CONTROL_FEATURE_H_

#include <string>

void shm_queue_send_buffer(std::string buffer);
void shm_queue_recv_buffer(std::string *buffer);

#endif // SHM_CONTROL_FEATURE_H_