
#ifndef _IMAP_BELT_H_
#define _IMAP_BELT_H_

#define BELT_MAJOR 171
#define BELT_MAGIC 'b'
#define BELT_IOCMAX 10
#define BELT_NAME "belt"
#define BELT_NODE "/dev/belt"

#define BELT_SCENE_GET          _IOR(BELT_MAGIC, 1, unsigned long)
#define BELT_SCENE_SET          _IOW(BELT_MAGIC, 2, unsigned long)
#define BELT_SCENE_UNSET        _IOW(BELT_MAGIC, 3, unsigned long)
#define BATTERY_LEVEL           _IOW(BELT_MAGIC, 4, unsigned long)

#define SCENE_VIDEO_NET         (1 << 0)
#define SCENE_VIDEO_LOCAL       (1 << 1)
#define SCENE_HDMI              (1 << 2)
#define SCENE_LOWPOWER          (1 << 3)
#endif /* _IMAP_BELT_H_ */
