/**
 * @file
 * @brief
 *
 * @date 08.02.13
 * @author Ilia Vaprol
 */

#ifndef DRIVERS_TTY_H_
#define DRIVERS_TTY_H_

#include <stddef.h>
#include <stdint.h>
#include <termios.h>

#include <framework/mod/options.h>
#include <module/embox/driver/tty/tty.h>

#define TTY_QUEUE_SZ \
	OPTION_MODULE_GET(embox__driver__tty__tty, NUMBER, queue_sz)

/* tty ioctls */
#define TTY_IOCTL_GETATTR  0x1
#define TTY_IOCTL_SETATTR  0x2
#define TTY_IOCTL_SETBAUD  0x3

struct tty_ops;

struct tty_queue {
	char   buff[TTY_QUEUE_SZ];
	size_t offset;
	size_t count;
};

struct tty {
	const struct tty_ops *ops;
	struct termios termios;
	struct tty_queue rx, tx;
};

struct tty_ops {
	void (*setup)(struct tty *, struct termios *);
	void (*tx_start)(struct tty *);
};

extern int tty_read(struct tty *, char *, size_t *);
extern int tty_write(struct tty *, char *, size_t *);
extern int tty_ioctl(struct tty *, unsigned long, void *);

/* These functions can be called from IRQ context. */

extern void tty_tx_done(struct tty *);
extern void tty_rx_char(struct tty *, char);

/* TTY RX/TX queues operations. */

extern struct tty_queue *tty_queue_init(struct tty_queue *);
extern int tty_enqueue(struct tty_queue *, char);
extern int tty_dequeue(struct tty_queue *);

#endif /* DRIVERS_TTY_H_ */
