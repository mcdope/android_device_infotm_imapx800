#include <errno.h>      
#include <fcntl.h>      
#include <linux/input.h>
#include <pthread.h>    
#include <stdarg.h>     
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <sys/stat.h>   
#include <sys/time.h>   
#include <sys/types.h>  
#include <sys/resource.h>
#include <utils/threads.h>
#include <utils/Log.h>
#include <time.h>       
#include <unistd.h>     
#include <private/android_filesystem_config.h>
#include "items.h"
#include "imap_wdt.h"

static void *wdt_thread(void *cookie)
{
    for (;;) {
        imapx_wdt_feeddog();
        sleep(WATCHDOG_PERIOD);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t t;
    int rate = 0;
    
    rate = item_integer("board.wdt", 0);    
    if (rate == 0)
        return -1;
    else if (rate < 0)
        rate = WATCHDOG_TIMEOUT;
    
    setgid(AID_SYSTEM);
    setuid(AID_SYSTEM);

    if (imapx_wdt_init() < 0)
        return -1;
    if (imapx_wdt_settime(rate) < 0)
        return -1;
    if (imapx_wdt_gettime() < 0)
        return -1;
    
    setpriority(PRIO_PROCESS, getpid(), ANDROID_PRIORITY_HIGHEST);
//    ALOGE("watchdog pid is %d <==> priority is %d\n", getpid(),
//            getpriority(PRIO_PROCESS, getpid()));

    ALOGE("Watchdog threads init ok\n");
    pthread_create(&t, NULL, wdt_thread, NULL);

    while(1)
        sleep(1);

    return 0;
}
