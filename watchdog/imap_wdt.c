#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils/Log.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/time.h>
#include "imap_wdt.h"

static int file = -1;

int imapx_wdt_open(void)
{
    int fd;

    fd = open(WATCHDOG_DEV, O_RDONLY);
    if (fd < 0) {
        ALOGE("watchdog node is not exist\n");
        return -1;
    }

    return fd;
}

void imapx_wdt_close(int fd)
{
    close(fd);
}

int imapx_wdt_bind(int fd)
{
    struct watchdog_info wdt;
    int ret;

    ret = ioctl(fd, WDIOC_GETSUPPORT, &wdt);
    if (ret < 0)
        return -1;
    ALOGE("watchdog: %s\n", wdt.identity);
#if 0
    if(strcmp(wdt.identity, "IMAPX800 Watchdog")) {
        ALOGE("watchdog id is not matching\n");
        return -1;
    }
#endif
    return 0;
}

/*
 * for stoping feeddog in kernel
 */

int imapx_wdt_killtime(int fd)
{
    int time = 0;
    int ret;

    ret = ioctl(file, WDIOC_KILLTIME, &time);
    if (ret < 0)
        return -1;
    ALOGV("watchdog stoping feeddog in kernel\n");
    return 0;
}


int imapx_wdt_init(void)
{
    file = imapx_wdt_open();

    if (file < 0)
        return -1;

    if (imapx_wdt_bind(file) < 0)
        return -1;

    if (imapx_wdt_killtime(file) < 0)
        return -1;
    ALOGE("watchdog init ok\n");
    return 0;
}

int imapx_wdt_settime(int timeout)
{
    int ret;
    
    ALOGV("watchdog set time %d\n", timeout);
    ret = ioctl(file, WDIOC_SETTIMEOUT, &timeout);
    if (ret < 0)
        return -1;
    return 0;
}

int imapx_wdt_gettime(void)
{
    int time = 0;
    int ret;

    ret = ioctl(file, WDIOC_GETTIMEOUT, &time);
    if (ret < 0)
        return -1;
    ALOGV("watchdog get time %d\n", time);
    return time;
}

int imapx_wdt_feeddog(void)
{
    int time;
    int ret;

    ret = ioctl(file, WDIOC_KEEPALIVE, &time);
    if (ret < 0)
        return -1;
    ALOGV("watchdog feeddog ok\n");
    return 0;
}
