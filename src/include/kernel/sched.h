/**
 * @file
 * @brief TODO -- Alina.
 *
 * @date 22.04.10
 * @author Dmitry Avdyukhin
 * @author Kirill Skorodumov
 * @author Alina Kramar
 */

#ifndef KERNEL_SCHED_H_
#define KERNEL_SCHED_H_

#include <sys/types.h>

#include <kernel/thread/types.h>
#include <kernel/sched/sched_lock.h>
#include <kernel/sched/sched_priority.h>


#define SCHED_TIMEOUT_INFINITE     (unsigned long)(-1)

struct thread;
struct event;
struct runq;

/**
 * Initializes scheduler.
 *
 * @param current
 *   Thread structure corresponding to the current control flow.
 * @param idle
 *   Thread that should be executed in the last resort.
 * @return
 *   Operation result.
 * @retval 0
 *   If initialization succeeds.
 * @retval non-zero
 *   Error code if initialization of the underlying policy implementation has
 *   failed.
 */
extern int sched_init(struct thread *current, struct thread *idle);

extern void sched_ticker_init(void);
extern void sched_ticker_fini(void);
extern void sched_ticker_switch(struct thread *prev, struct thread *next);

/**
 * Makes active or waking thread and adds thread to the queue of ready to
 * executing threads.
 *
 * @param thread
 *   Thread which will be added.
 */
extern void __sched_start(struct thread *t);
extern void sched_start(struct thread *t);

/**
 * Makes exit thread and removes thread from scheduler.
 */
extern void sched_finish(struct thread *t);

static inline int sched_ready(struct thread *t) {
	return (t->state & THREAD_READY);
}

static inline int sched_active(struct thread *t) {
	return (t->state & THREAD_ACTIVE);
}

/**
 * Changes scheduling priority of the thread. If the thread is running now
 * (present in the runq) calls runq_change_priority() for rescheduling runq
 * and thread_set_priority() else just calls thread_set_priority() for setup
 * a new value for priority field in sched_strategy_data structure
 *
 * @param t
 *   Thread to operate with.
 * @param priority
 *   New scheduling priority of the the thread.
 */
extern int sched_change_priority(struct thread *t, sched_priority_t priority);

extern int sched_wait(void);
extern int sched_wait_timeout(int timeout);

extern void __sched_wait_prepare(struct waitq *wq, int result);
extern void sched_wait_prepare(struct waitq *wq, int result);

extern int __sched_wait_cleanup(struct waitq *wq);
extern int sched_wait_cleanup(struct waitq *wq);

extern int __sched_wakeup(struct thread *t, int result);
extern int sched_wakeup(struct thread *t, int result);

extern void sched_wakeup_waitq(struct waitq *wq, int nr, int result);
static inline void sched_wakeup_waitq_all(struct waitq *wq, int result) {
	sched_wakeup_waitq(wq, 0, result);
}

/**
 * Wait cond_expr to become TRUE. waitq may be NULL.
 */
#define SCHED_WAIT_ON(waitq, cond_expr, timeout, intr)    \
	((cond_expr) ? 0 : ({                                 \
		int __wait_ret;                                   \
		                                                  \
		while (1) {                                       \
			sched_wait_prepare(waitq, 0);                 \
			if (cond_expr) {                              \
				__wait_ret = 0;                           \
				break;                                    \
			}                                             \
			__wait_ret = sched_wait_timeout(timeout);     \
			if (__wait_ret == -ETIMEDOUT)                 \
				break;                                    \
			if (!intr && (__wait_ret == -EINTR))          \
				break;                                    \
		}                                                 \
		if (cond_expr)                                    \
			__wait_ret = 0;                               \
		                                                  \
		sched_wait_cleanup(waitq);                        \
		                                                  \
		__wait_ret;                                       \
	}))

/**
 * Wait cond_expr to become TRUE.
 */
#define SCHED_WAIT(cond_expr, timeout, intr)     \
	SCHED_WAIT_ON(NULL, cond_expr, timeout, intr)

/**
 * Requests switching of the current thread.
 */
extern void sched_post_switch(void);

/**
 * Runs the scheduler right now.
 */
extern void sched_switch(void);

/**
 * @brief Makes thread to run regardless of it's state if thread is scheduling
 *
 * @param thread Thread to operate with
 *
 * @return
 *   Operation result
 * @retval 0
 *   On success.
 * @retval non-zero
 *   On operation fail.
 */
extern void sched_signal(struct thread *thread);


#endif /* KERNEL_SCHED_H_ */
