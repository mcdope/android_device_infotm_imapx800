/*
 * mmc block IO
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mount.h>  // for _IOW, _IOR, mount()
#include <mtd/mtd-user.h>
#include <sys/stat.h>
#include <assert.h>
#include "areautils.h"
#include "items.h"
#include "cutils/log.h"
#define LOG_TAG "Area"
static int mtd_read(const AreaInfo* area, size_t start, char *data, size_t size)          
{                                                                                 
	struct mtd_ecc_stats before, after;      
	int fd = area->fd;	
	if (ioctl(fd, ECCGETSTATS, &before)) {                                        
		SLOGE("mtd: ECCGETSTATS error (%s)\n", strerror(errno));        
		return -1;        
	}                                                                             
//	SLOGE("Start 0x%8x  buffer 0x%x size 0x%x", start , data ,size);
	size_t skipsize = area->disk.mtd_info.erasesize;                                         
	int mgbb;                                                                     
	size_t readsz =0 , remain = size;
	loff_t pos = start;
	while (pos + skipsize <= (int) area->size){
		if(remain > area->disk.mtd_info.erasesize)
			readsz =  area->disk.mtd_info.erasesize;
		else readsz = remain;		
//		LOGE("readsize 0x%x, offs:0x%8x",readsz,area->index + pos);
		if (lseek64(fd, area->index + pos, SEEK_SET) != (pos + area->index)){ 
			SLOGE("mtd: lseek error at 0x%8x (%s)\n",
					pos, strerror(errno));  
		}else if(read(fd, data, size) != size){
			SLOGE("mtd: read error at 0x%8x (%s)\n",                 
					pos, strerror(errno));    
			return -1;

		} else if (ioctl(fd, ECCGETSTATS, &after)) {                              
			SLOGE("mtd: ECCGETSTATS error (%s)\n", strerror(errno));    
			return -1;                                                            
		} else if (after.failed != before.failed) {                               
			SLOGE("mtd: ECC errors (%d soft, %d hard) at 0x%08llx\n",   
					after.corrected - before.corrected,                           
					after.failed - before.failed, pos);                           
			// copy the comparison baseline for the next read.                    
			memcpy(&before, &after, sizeof(struct mtd_ecc_stats));      
		}else if((mgbb = ioctl(fd, MEMGETBADBLOCK, &pos))){
			fprintf(stderr,                                                    
					"mtd: MEMGETBADBLOCK returned %d at 0x%08llx (errno=%d)\n",
					mgbb, pos, errno);                                         
		}else{
//			SLOGE("Read success");
			remain -= readsz;			
			if(0==remain) return 0;  // Success!                                                
		}                                                                         
		pos += skipsize;  
	}                                  
//	SLOGE("Mtd read failed");
	errno = ENOSPC;                    
	return -1;                         
}                                      
/* Return the offset of the first good block at or after pos (which
 * might be pos itself).                                           
 */                                                                
static off_t mtd_erase_block(struct AreaInfo *area , size_t start)        
{                                                                                           
	// Zero-pad and write any pending data to get us to a block boundary                                           

	off_t pos = lseek64(area->fd, start + area->index, SEEK_CUR);                                       
	if ((off_t) pos == (off_t) -1) return pos;                                      
	if((area->size - start)<area->disk.mtd_info.erasesize) 
	{
		SLOGE("It will be out of the area boundary %s" ,__func__);
		return -ENOSPC;
	}

	loff_t skiped = 0;
	bool erased = false;
	// Erase the specified number of blocks                                                                        
	while (erased == false) {     
		loff_t bpos = pos;		
		while(ioctl(area->fd, MEMGETBADBLOCK, &bpos) > 0) {                           
			SLOGE( "mtd: not erasing bad block at 0x%08lx\n", pos);                                       
			skiped += area->disk.mtd_info.erasesize;  
			pos += area->disk.mtd_info.erasesize;
			bpos = pos;
			continue;  // Don't try to erase known factory-bad blocks.                                             
		}                                                                                                          

		struct erase_info_user erase_info;                                                                         
		erase_info.start = pos;
		erase_info.length = area->disk.mtd_info.erasesize;     

		if (ioctl(area->fd, MEMERASE, &erase_info) < 0) {                                                       
			SLOGE( "mtd: erase failure at 0x%08lx\n", pos);                                               
		}else{
			erased = true ;
		}                                                                                                          
	}                                                                                                              
	return skiped;// return skiped bad blocks size count. 
}               
/*
   static void 
   add_bad_block_offset(struct AreaInfo *area) {
   if (area->disk.bad_block_count + 1 > area->disk.bad_block_alloc) {                        
   area->disk.bad_block_alloc = (area->disk.bad_block_alloc*2) + 1;                      
   area->disk.bad_block_offsets = realloc(area->disk.bad_block_offsets,                  
   area->disk.bad_block_alloc * sizeof(off_t));   
   }   
   area->disk.bad_block_offsets[area->disk.bad_block_count++] = start;                         
   }  
*/
static int mtd_write(struct AreaInfo *area, size_t offs, const char *data ,size_t size)                      
{                                                                                   

	int fd = area->fd;                                                               
	ssize_t skipsz = area->disk.mtd_info.erasesize;
	size_t start = offs;	
	lseek(fd ,area->index + offs , SEEK_SET );
	start = lseek(fd , 0 , SEEK_CUR );
//	SLOGE("write start:0x%x index 0x%x offs 0x%x", start ,area->index , offs);
	while (start-area->index + skipsz <= (int) area->size) {                                   
		loff_t bpos = start;                                                          
		int ret = ioctl(area->fd, MEMGETBADBLOCK, &bpos);                                 
		if (ret != 0 && !((ret == -1) && (errno == EOPNOTSUPP))) {                      
			//	add_bad_block_offset(area);                                         
			SLOGE("mtd: not writing bad block at 0x%08lx (ret %d errno %d)\n",    
					start, ret, errno);                                               
			start += skipsz;                                           
			continue;  // Don't try to erase known factory-bad blocks.              
		}                                                                           

		struct erase_info_user erase_info;                                          
		erase_info.start = start;                                                     
		erase_info.length = area->disk.mtd_info.erasesize;
//		SLOGE("Erase mtd from 0x%x for 0x%x Bytes"
//				, erase_info.start , erase_info.length );
		int retry;                                                                  
		for (retry = 0; retry < 2; ++retry) {   
	//		SLOGE("fd %d",area->fd);			
			if (ioctl(area->fd, MEMERASE, &erase_info) < 0) {                             
				SLOGE("mtd: erase failure at 0x%8x (%s)\n",
						start, strerror(errno));                                      
				continue;                                                           
			}                                                                       
			if ((lseek(area->fd, start , SEEK_SET) != start )
					||write(area->fd, data, size) != size) {                                    
				SLOGE("mtd: write error at 0x%8x (%s)\n",               
						start, strerror(errno));                                      
			}                                                                       

			char verify[size];                                                           
			if ((lseek(area->fd, start , SEEK_SET) != start )
					||  read(area->fd, verify, size) != size) {                                        
				SLOGE("mtd: re-read error at 0x%8x (%s)\n",                  
						start, strerror(errno));                                           
				continue;                                                                
			}                                                                            
			if (memcmp(data, verify, size) != 0) {                                       
				SLOGE("mtd: verification error at 0x%8x (%s)\n",             
						start, strerror(errno));                                           
				continue;                                                                
			}                                                                            

			if (retry > 0) {                                                             
				SLOGE("mtd: wrote block after %d retries\n", retry);           
			}                                                                            
			SLOGE("mtd: successfully wrote block at %llx\n", start);
			return 0;  // Success!                                                       
		}                                                                                

		// Try to erase it once more as we give up on this block                         
		//		add_bad_block_offset(area);                                                  
		SLOGE("mtd: skipping write block at 0x%8x\n", start);                  
		ioctl(fd, MEMERASE, &erase_info);                                                
		start += skipsz;                                                    
	}                                                                                    

	// Ran out of space on the device                                                    
	errno = ENOSPC;                                                                      
	return -1;                                                                           
}                                                                                        
static int mtd_info(const struct AreaInfo* area)
{                                                                  
	
	struct mtd_info_user mtd_info;
	char mtddevname[32];                                           
	sprintf(mtddevname, "/dev/mtd/mtd%d", area->partnum);
	int fd = open(mtddevname, O_RDONLY);                           
	if (fd < 0) return -1;                                         

//	SLOGE("mtddevname,%s 0x%8x",mtddevname, &(area->disk.mtd_info));	
	int ret = ioctl(fd, MEMGETINFO, &(area->disk.mtd_info)); 	
	close(fd);               
//      SLOGE("erasesize,0x%8x %d",area->disk.mtd_info.erasesize,ret);	
	if (ret < 0) return -1;                                        
	return 0;                                                      
}                                                                  
static int open_area(struct AreaInfo* area)
{
	char device[64];
	if(area->disk.diskNNd == true){
		sprintf(device,"/dev/mtd/mtd%d", area->partnum);
	}else{
		if(area->partnum > 0){
			sprintf(device,"/dev/block/mmcblk%dp%d", 
					area->disk.channel,area->partnum);
		}else{
			sprintf(device,"/dev/block/mmcblk%d", area->disk.channel);
		}
	}
	int fd =open(device, O_RDWR);
//	SLOGE(" open_area fd %d",fd);
	if(fd < 0) return -1;
	area->fd = fd;
	return 0;
}
static void get_disk(struct DiskInfo* info)
{
	char disk[ITEM_MAX_LEN];
	item_string(disk, "board.disk", 0);
	SLOGE("disk type %s",disk);
	if(strncmp(disk,"mmc",3)== 0 ||strncmp(disk,"MMC",3)== 0  ){
		info->diskNNd = false;
		info->channel = atoi(disk+3);
	}else{
		info->diskNNd = true;
		info->channel = 0;
	}
}
static int area_get_info(struct AreaInfo* area)
{
	struct part_sz psz[] = {
		{ "uboot0", 2, 6 },
		{ "uboot1", 8, 8 },
		{ "items", 16, 8 },
		{ "logo", 24, 8 },
		{ "ramdisk", 32, 8 },
		{ "flags", 40, 8 },
		{ "reserved", 48, 8 },
		{ "kernel0", 56, 16 },
		{ "kernel1", 72, 16 },
		{ "recovery", 88, 24 },
		{ "system", 0, 152 },
		{ "misc", 0, 20 },
		{ "cache", 0, 64 },
		{ "exten", 0, 64 },
		{ "userdata", 0, 164 },
		{ "local", 0, 1024 },
	};

	int i = 0;
	int index = 0;
	area->partnum = 0;
	while(psz[i].offs != 0){
		if (strcmp(psz[i].name, area->name) == 0) { 
			area->index = psz[i].offs<<20;
			area->size = psz[i].size<<20;
			goto continue_next;
		}
		i++;
	}
	int partnum = 1;
	while(i< 17){
		if(strcmp(psz[i].name, area->name) == 0){
			area->index = 0;
			area->partnum = partnum;
			area->size = psz[i].size<<20;
			goto continue_next;
		}
		i++;
		partnum++;
	}
	if(i== 17) return 17;
continue_next:
	get_disk(&area->disk);
	if(area->disk.diskNNd == true) mtd_info(area);
	return open_area(area);
}
size_t area_read_data(struct AreaInfo* area,size_t offs, char* data, size_t len)
{
	SLOGE("0x%8x 0x%8x area->size:0x%x",data , len , area->size);
	if(offs + len > area->size){
		SLOGE("Read length overfllow %s",__func__);
		return -1;
	}
	if(area->disk.diskNNd == true) return  mtd_read(area ,offs ,data , len);
//	SLOGE("Read from mmc card");
	lseek64(area->fd , offs+ area->index , SEEK_SET);
	return read(area->fd, data, len);
}
size_t area_write_data(struct AreaInfo* area, size_t offs, char *data, size_t len)
{
	int start = offs;
	if(start + len > area->size){
		SLOGE("Write length overfllow %s",__func__);
		return -1;
	}
	int ret = 0;

	if(area->disk.diskNNd == true) {
		int writesz = 0, remain = len;
		while(remain){
			if(len > area->disk.mtd_info.erasesize)
				writesz = area->disk.mtd_info.erasesize ;
			else
				writesz = len;
			//	int ret = mtd_erase_blocks(area, start ,1 );
			//		if(-ENOSPC == ret){
			//			return ret;
			//		}
			//		start += ret ; //skip bad blocks;
			int ret = mtd_write(area , start , data , writesz);
			if(ret) return ret;
			remain -= writesz;
			start += ret;
		}
		ret = len;
	}else{
		lseek64(area->fd , start + area->index , SEEK_SET);
		ret = write(area->fd, data, len);
		if(ret != len ){
			SLOGE("write failed %s",__func__);
			ret =-1;
		}
	}
	return ret;
}
struct AreaInfo* area_open(const char* name)
{
	if(NULL == name) return -1;
	struct AreaInfo *area = malloc(sizeof(struct AreaInfo));
	if(NULL == area) return -2;
	area->name = strdup(name);
	if(area->name == NULL){
		area_close(area);
		return -1;
	}
//	SLOGE("Area name %s", area->name);
	int ret = area_get_info(area);
	if(ret < 0)
	{
		area_close(area);
		SLOGE("opend failed return NULL");
		return (struct AreaInfo*)NULL;
	}
//	SLOGE("area start 0x%8x",area->index);
	return area;
}
int area_close(struct AreaInfo* area)
{
	if(area == NULL)
		return 0;
//	SLOGE("1");
	if(area->fd) close(area->fd);
	sync();
//	SLOGE("2");
	if(area->name)free(area->name);
//	SLOGE("3");
	free(area);
	return 0;
}
