/*
 * =====================================================================================
 *
 *       Filename:  mmcutils.h
 *
 *    Description:  mmc utils
 *
 *        Version:  1.0
 *        Created:  2011年12月15日 10时51分54秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xecle , xecle@hotmail.com
 *        Company:  InfoTM
 *
 * =====================================================================================
 */

#ifndef AREAUTILS_H_
#define AREATILS_H_
#include <mtd/mtd-user.h>
#include <stdbool.h>
struct part_sz {
	char name[20];
	ssize_t offs;
	ssize_t size;
};
typedef struct DiskInfo{
	bool	diskNNd;
	char	channel;
	struct mtd_info_user mtd_info;
}DiskInfo;
typedef struct AreaInfo{
	char   *name;
	size_t index;
	size_t size;
	int    fd ;
	char   partnum;
	struct DiskInfo disk;
}AreaInfo;


size_t area_read_data(struct AreaInfo* area,size_t offs, char* data, size_t len);
size_t area_write_data(struct AreaInfo* area,size_t offs, char *data, size_t len);
struct AreaInfo* area_open(const char* name);
int area_close(struct AreaInfo* area);
#endif
