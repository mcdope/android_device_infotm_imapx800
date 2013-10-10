#!/system/bin/busybox sh
for apk in `busybox  ls -1 system/custom/*.apk`; do pm customize  $apk /data/app;done
for apk in `busybox  ls -1 data/vendor/*.apk`; do pm customize  $apk /data/app;done
while true; do hassd=$( mount | busybox grep "/mnt/sdcard" ); if [ -z "$hassd" ]; then sleep 1; else break; fi; done
for apk in `busybox  ls -1 system/ext_app/*.apk`; do pm install -s $apk;done
busybox touch /data/data/isFirstTime
busybox chmod 644 /data/data/isFirstTime

if [ ! -d /mnt/sdcard/Video/demo ]; then
	mkdir /mnt/sdcard/Video/demo
fi
if [ ! -d /mnt/sdcard/Photo/demo ]; then
	mkdir /mnt/sdcard/Photo/demo
fi
if [ ! -d /mnt/sdcard/Music/demo ]; then
	mkdir /mnt/sdcard/Music/demo
    sleep 3
    reboot
fi

