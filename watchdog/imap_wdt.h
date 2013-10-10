#ifndef __HARDWARE_WDT__
#define __HARDWARE_WDT__

#ifdef __cplusplus
extern "C" {
#endif

#define WATCHDOG_TIMEOUT        8 
#define WATCHDOG_PERIOD         2

#define WATCHDOG_DEV    "/dev/watchdog"
#define WATCHDOG_IOCTL_BASE 'W'                                               

struct watchdog_info {                                                        
    __u32 options;      /*  Options the card/driver supports */                
    __u32 firmware_version; /*  Firmware version of the card */                
    __u8  identity[32]; /*  Identity of the board */                           
};                                                                            

#define WDIOC_GETSUPPORT    _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define WDIOC_GETSTATUS     _IOR(WATCHDOG_IOCTL_BASE, 1, int)                 
#define WDIOC_GETBOOTSTATUS _IOR(WATCHDOG_IOCTL_BASE, 2, int)                 
#define WDIOC_GETTEMP       _IOR(WATCHDOG_IOCTL_BASE, 3, int)                 
#define WDIOC_SETOPTIONS    _IOR(WATCHDOG_IOCTL_BASE, 4, int)                 
#define WDIOC_KEEPALIVE     _IOR(WATCHDOG_IOCTL_BASE, 5, int)                 
#define WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)            
#define WDIOC_GETTIMEOUT        _IOR(WATCHDOG_IOCTL_BASE, 7, int)             
#define WDIOC_SETPRETIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 8, int)                
#define WDIOC_GETPRETIMEOUT _IOR(WATCHDOG_IOCTL_BASE, 9, int)                 
#define WDIOC_GETTIMELEFT   _IOR(WATCHDOG_IOCTL_BASE, 10, int)                
#define WDIOC_KILLTIME      _IOR(WATCHDOG_IOCTL_BASE, 11, int)

int imapx_wdt_init(void);
int imapx_wdt_settime(int timout);
int imapx_wdt_gettime(void);
int imapx_wdt_feeddog(void);

#ifdef __cplusplus
}
#endif

#endif
