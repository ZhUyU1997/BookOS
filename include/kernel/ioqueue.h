#ifndef _IOQUEQUE_H
#define	_IOQUEQUE_H
#include "kernel/types.h"
#include "kernel/lock.h"
#include "kernel/task.h"

#define IO_QUEUE_BUF_LEN 64

#define IQ_MODE_IDLE 0
#define IQ_MODE_MOVE 1

#define IQ_DATA_RANGE(data, low, high) (low <= data && data < high)

struct ioqueue {
    struct lock lock;
    int32_t *buf;			    // 缓冲区大小
    int32_t in;			    // 队首,数据往队首处写入
    int32_t out;			    // 队尾,数据从队尾处读出
	int32_t size;
	struct task *task;
};

#define IOQUEUE_SIZE sizeof(struct ioqueue)
struct ioqueue* create_ioqueue();
void ioqueue_init(struct ioqueue* ioqueue);

int ioqueque_get(struct ioqueue* ioqueue, int mode);
void ioqueque_put(struct ioqueue* ioqueue, int data); 

bool ioqueque_empty(struct ioqueue* ioqueue);
void ioqueue_bound(struct ioqueue* ioqueue, struct task *task);

#endif