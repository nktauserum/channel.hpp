#pragma once

#include <limits.h>
#include <linux/futex.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <atomic>

static int futex(std::atomic<uint32_t>* lock, int futex_op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3) {
    return syscall(SYS_futex, lock, futex_op, val, timeout, uaddr2, val3);
}

int futex_wait(std::atomic<uint32_t>* lock, uint32_t expect_val) {
    return futex(lock, FUTEX_WAIT_PRIVATE, expect_val, NULL, NULL, 0);
}

int futex_wake(std::atomic<uint32_t>* lock) {
    return futex(lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}
