/***************************************************************************** 
** common/oem/config.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: OEM configurations parser.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 04/01/2012 XXX	draft
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils/Log.h>
#include <sys/ioctl.h>
#include "items.h"
#include "areautils.h"
#define LOG_TAG "items"
static int __get_node(void)
{
	int fd;

	fd = open(ITEMS_NODE, O_RDONLY);
	//if(fd < 0)
	//  SLOGE("failed to open dev node %s: %d\n",
	//	                  __func__, fd);

	return fd;
}

static void __put_node(int d) {
	close(d);
}
typedef struct ItemArea{
	struct AreaInfo* area;
	char* contentbuf;
#define CONTENT_BUFFER_SIZE  0x4000
	int   contentlen;
	bool  changed;
}ItemArea;

int item_exist(const char *key)
{
	int d = __get_node(), ret = 0;
	struct items_query qt;

	if(d >= 0) {
		strncpy(qt.key, key, ITEM_MAX_LEN);
		ret = ioctl(d, ITEMS_EXIST, &qt);
		__put_node(d);
	}

	return ret;
}

int item_open(void)
{
	//SLOGE("_______________item open enter_______________");
	struct ItemArea *item = malloc(sizeof(ItemArea));
	if(NULL == item) {
		SLOGE("Failed to alloc item %s", __func__);
		return -1;
	}
//	SLOGE("item open enter");
	item->area = area_open("items");
//	 SLOGE("item open exit");
	if(item->area == NULL || item->area < 0){
		SLOGE("failed to get infomation of area items %s: %d\n", __func__, item->area);
		free(item);
		return 0;
	}
	item->contentbuf = malloc(CONTENT_BUFFER_SIZE);
		
//	SLOGE("fd %d item->contentbuf:0x%8x",item->area, item->contentbuf);
	if(NULL == item->contentbuf){
		SLOGE("failed to alloc buffer %s",__func__);
		area_close(item->area);
		free(item);
		return 0;
	}

	bool get_end = false;
	char* s = item->contentbuf;

//	SLOGE("enter area_read_data");
	if(area_read_data(item->area, 0, s , CONTENT_BUFFER_SIZE) == -1){
		SLOGE("Failed to read original data");
	       	area_close(item->area);
		free(item);            
		return 0;
	}
//	SLOGE("area_read_data exit");

	int k = 0;
//	int fd = __get_node();
//	size_t size = read(fd , s , CONTENT_BUFFER_SIZE);
//	SLOGE("read size error 0x%x", size);
//	SLOGE("%s",s);
	while(false == get_end&&k < CONTENT_BUFFER_SIZE){
		if( (s[k]!='#' )&& (strncmp(s+k+1, "items.end", 9) == 0)){
			get_end = true;
			item->contentlen = k;
			SLOGE("Got items content length %d",item->contentlen);
			break;
		}
		k++;
	}
	if(get_end == false) SLOGE("canot find the items end 0x%x",k);
	item->changed = false;
//	SLOGE("exit item open");
	return (int)item;
}
int item_close(int fd)
{
	struct ItemArea* item = (struct ItemArea* ) fd;
	if(NULL == item) return 0;
	/**write new added content to disk**/
	if(true == item->changed){
//		SLOGE("Write the changed items to disk");
		area_write_data(item->area,0,item->contentbuf,0x4000);	
	}
	int ret = area_close(item->area);
	if(item->contentbuf)free(item->contentbuf);
	free(item);
	return ret;
}
int item_add_content(int fd, const char* content)
{
	ItemArea* item = (ItemArea*) fd;
//	SLOGE("fd:0x%x %s",fd ,__func__);
	if(fd <= 0) return -1;
	char name[64] = {0,};
	int  n = 0;
	char buf[256]={0,};
	if(content[0]=='#'){
		SLOGE("Invalid content format: start with '#' %s",__func__);
		return -1;
	}
	memcpy(buf , content , 256);
	while(buf[n]!= '\0'&& buf[n] != ' ') n++;
	memcpy( name , buf , n);
//	SLOGE("Try to add content %s",name);
	while(buf[n]!='\0') n++;
	if(buf[n-1]!= '\n'){
		buf[n] = '\n';
		n += 1;
		buf[n] = '\0';
	}
	if(item_exist(name)){
		SLOGE("The content (name:%s) is already exist %s",name ,__func__);
		return -1;
	}
	if(item->contentlen + n > (CONTENT_BUFFER_SIZE-9)){
		SLOGE("No more space for new content %s",__func__);
		return -2;
	}else{
//		SLOGE("Add new conten at 0x%x",item->contentlen);
		memcpy(item->contentbuf + item->contentlen , buf, n);
		item->contentlen += n;
//		SLOGE("Now the items length is 0x%x",item->contentlen);
		memcpy(item->contentbuf + item->contentlen , "items.end" , 9);
		int items[2] = {item->contentbuf , item->contentlen + 9};
		int d = __get_node();
		ioctl(d,ITEMS_INIT , items);
		__put_node(d);
	}
//	SLOGE("items changed");
	item->changed = true;
	return 0;
	
}
int item_delete_content(int fd ,const char* name)
{
//	SLOGE("item_delete_content enter 0x%x",fd);
	if(fd <= 0) return -1;
//	SLOGE("Try to delete content %s",name); 
	ItemArea* item = (ItemArea*) fd;
	int start = 0, end = 0;
	char* s = item->contentbuf;
	char *buf = strdup(name);
	int i = 0;
	while(buf[i]!= ' ' && buf[i]!='\0') i++;
	if(buf[i]!= '\0') buf[i] = '\0';
	int len = strlen(buf);
	while(start < item->contentlen){
		if(s[start]!='#'&& strncmp(s+start+1 , buf , len )==0) break;
		start++;
	}
	if(start == item->contentlen){
		SLOGE("The content required to delete is not exist %s",__func__);
		return -1;
	}
	start +=1;
	end = start + strlen(buf);
	while(end < item->contentlen){
		if(s[end]=='\n'||s[end] == '\0')break;
		end++;
	}         
//	SLOGE("delete start 0x%x end 0x%x ",start , end);
	memcpy(item->contentbuf + start, item->contentbuf + end ,item->contentlen- end + 9);
	item->contentlen = item->contentlen - end + start;
	int items[2] = {item->contentbuf , item->contentlen + 9};     
 	int d = __get_node();                                         
  	ioctl(d,ITEMS_INIT , items);                                  
   	__put_node(d);        
	item->changed = true;	
//	SLOGE("item_delete_content exit");
	return 0;
}
int item_string(char *buf, const char *key, int section)
{
	int d = __get_node(), ret = -1;
	struct items_query qt;
	if(d >= 0) {
		strncpy(qt.key, key, ITEM_MAX_LEN);
		qt.section = section;
		ret = ioctl(d, ITEMS_STRING, &qt);
		__put_node(d);
		strncpy(buf, qt.fb, ITEM_MAX_LEN);
	}

	return ret;
}

int item_integer(const char *key, int section)
{
	int d = __get_node(), ret = -ITEMS_EINT;
	struct items_query qt;

	if(d >= 0) {
		strncpy(qt.key, key, ITEM_MAX_LEN);
		qt.section = section;
		ret = ioctl(d, ITEMS_INTEGER, &qt);
		__put_node(d);
	}

	return ret;
}

int item_equal(const char *key, const char *value, int section)
{
	int d = __get_node(), ret = 0;
	struct items_query qt;

	if(d >= 0) {
		strncpy(qt.key,   key,   ITEM_MAX_LEN);
		strncpy(qt.value, value, ITEM_MAX_LEN);
		qt.section = section;
		ret = ioctl(d, ITEMS_EQUAL, &qt);
		__put_node(d);
	}

	return ret;
}

int item_string_item(char *buf, const char *string, int section)
{
	int d = __get_node(), ret = -1;
	struct items_query qt;

	if(d >= 0) {
		strncpy(qt.key, string, ITEM_MAX_LEN);
		ret = ioctl(d, ITEMS_ITEM, &qt);
		qt.section = section;
		__put_node(d);
		strncpy(buf, qt.fb, ITEM_MAX_LEN);
	}

	return ret;
}


