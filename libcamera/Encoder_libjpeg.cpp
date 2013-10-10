/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file Encoder_libjpeg.cpp
*
* This file encodes a YUV422I buffer to a jpeg
* TODO(XXX): Need to support formats other than yuv422i
*            Change interface to pre/post-proc algo framework
*
*/

#define LOG_TAG "CameraHAL"

#include "CameraHal.h"
#include "Encoder_libjpeg.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array)[0]))
#define MIN(x,y) ((x < y) ? x : y)

namespace android {
struct integer_string_pair {
    unsigned int integer;
    const char* string;
};

static integer_string_pair degress_to_exif_lut [] = {
    // degrees, exif_orientation
    {0,   "1"},
    {90,  "6"},
    {180, "3"},
    {270, "8"},
};
struct libjpeg_destination_mgr : jpeg_destination_mgr {
    libjpeg_destination_mgr(uint8_t* input, int size);

    uint8_t* buf;
    int bufsize;
    size_t jpegsize;
};

static void libjpeg_init_destination (j_compress_ptr cinfo) {
    libjpeg_destination_mgr* dest = (libjpeg_destination_mgr*)cinfo->dest;

    dest->next_output_byte = dest->buf;
    dest->free_in_buffer = dest->bufsize;
    dest->jpegsize = 0;
}

static boolean libjpeg_empty_output_buffer(j_compress_ptr cinfo) {
    libjpeg_destination_mgr* dest = (libjpeg_destination_mgr*)cinfo->dest;

    dest->next_output_byte = dest->buf;
    dest->free_in_buffer = dest->bufsize;
    return TRUE; // ?
}

static void libjpeg_term_destination (j_compress_ptr cinfo) {
    libjpeg_destination_mgr* dest = (libjpeg_destination_mgr*)cinfo->dest;
    dest->jpegsize = dest->bufsize - dest->free_in_buffer;
}

libjpeg_destination_mgr::libjpeg_destination_mgr(uint8_t* input, int size) {
    this->init_destination = libjpeg_init_destination;
    this->empty_output_buffer = libjpeg_empty_output_buffer;
    this->term_destination = libjpeg_term_destination;

    this->buf = input;
    this->bufsize = size;

    jpegsize = 0;
}
void ExifElementsTable::stringToRational(const char* str, unsigned int* num, unsigned int* den) {
    int len;
    char * tempVal = NULL;

    if (str != NULL) {
        len = strlen(str);
        tempVal = (char*) malloc( sizeof(char) * (len + 1));
    }

    if (tempVal != NULL) {
        // convert the decimal string into a rational
        size_t den_len;
        char *ctx;
        unsigned int numerator = 0;
        unsigned int denominator = 0;
        char* temp = NULL;

        memset(tempVal, '\0', len + 1);
        strncpy(tempVal, str, len);
        temp = strtok_r(tempVal, ".", &ctx);

        if (temp != NULL)
            numerator = atoi(temp);

        if (!numerator)
            numerator = 1;

        temp = strtok_r(NULL, ".", &ctx);
        if (temp != NULL) {
            den_len = strlen(temp);
            if(HUGE_VAL == den_len ) {
                den_len = 0;
            }

            denominator = static_cast<unsigned int>(pow(10, den_len));
            numerator = numerator * denominator + atoi(temp);
        } else {
            denominator = 1;
        }

        free(tempVal);

        *num = numerator;
        *den = denominator;
    }
}

static status_t convertGPSCoord(double coord, int &deg, int &min, int &sec, int &secDivisor)
{
#define GPS_SEC_DIV                 60
#define GPS_MIN_DIV                 60
#define GPS_SEC_ACCURACY            1000

    double tmp;

    if ( coord == 0 ) {
        ALOGE("Invalid GPS coordinate");
        return -EINVAL;
    }

    deg = (int) floor(fabs(coord));
    tmp = ( fabs(coord) - floor(fabs(coord)) ) * GPS_MIN_DIV;
    min = (int) floor(tmp);
    tmp = ( tmp - floor(tmp) ) * ( GPS_SEC_DIV * GPS_SEC_ACCURACY );
    sec = (int) floor(tmp);
    secDivisor = GPS_SEC_ACCURACY;

    if( sec >= ( GPS_SEC_DIV * GPS_SEC_ACCURACY ) ) {
        sec = 0;
        min += 1;
    }

    if( min >= 60 ) {
        min = 0;
        deg += 1;
    }

    return NO_ERROR;
}

bool ExifElementsTable::isAsciiTag(const char* tag) {
    // TODO(XXX): Add tags as necessary
    return (strcmp(tag, TAG_GPS_PROCESSING_METHOD) == 0);
}

void ExifElementsTable::insertExifToJpeg(unsigned char* jpeg, size_t jpeg_size) {
    ReadMode_t read_mode = (ReadMode_t)(READ_METADATA | READ_IMAGE);

    ResetJpgfile();
    if (ReadJpegSectionsFromBuffer(jpeg, jpeg_size, read_mode)) {
        jpeg_opened = true;
        create_EXIF(table, exif_tag_count, gps_tag_count, has_datetime_tag);
    }
}

status_t ExifElementsTable::insertExifThumbnailImage(const char* thumb, int len) {
    status_t ret = NO_ERROR;

    if ((len > 0) && jpeg_opened){ 
        ret = ReplaceThumbnailFromBuffer(thumb, len);
    }

    return ret;
}

void ExifElementsTable::saveJpeg(unsigned char* jpeg, size_t jpeg_size) {
    if (jpeg_opened) {
       WriteJpegToBuffer(jpeg, jpeg_size);
       DiscardData();
       jpeg_opened = false;
    }
}

/* public functions */
ExifElementsTable::~ExifElementsTable() {
    int num_elements = gps_tag_count + exif_tag_count;

    for (int i = 0; i < num_elements; i++) {
        if (table[i].Value) {
            free(table[i].Value);
        }
    }

    if (jpeg_opened) {
        DiscardData();
    }
}
status_t ExifElementsTable::insertElement(const char* tag, const char* value) {
    unsigned int value_length = 0;
    status_t ret = NO_ERROR;

    if (!value || !tag) {
        return -EINVAL;
    }

    if (position >= MAX_EXIF_TAGS_SUPPORTED) {
        ALOGE("Max number of EXIF elements already inserted");
        return NO_MEMORY;
    }

    if (isAsciiTag(tag)) {
        value_length = sizeof(ExifAsciiPrefix) + strlen(value + sizeof(ExifAsciiPrefix));
    } else {
        value_length = strlen(value);
    }

    if (IsGpsTag(tag)) {
        table[position].GpsTag = TRUE;
        table[position].Tag = GpsTagNameToValue(tag);
        gps_tag_count++;
    } else {
        table[position].GpsTag = FALSE;
        table[position].Tag = TagNameToValue(tag);
        exif_tag_count++;

        if (strcmp(tag, TAG_DATETIME) == 0) {
            has_datetime_tag = true;
        }
    }

    table[position].DataLength = 0;
    table[position].Value = (char*) malloc(sizeof(char) * (value_length + 1));

    if (table[position].Value) {
        memcpy(table[position].Value, value, value_length + 1);
        table[position].DataLength = value_length + 1;
    }

    position++;
    return ret;
}

ExifEncode::~ExifEncode()
{
}    

status_t ExifEncode::setupEXIF_libjpeg(ExifElementsTable* exifTable)
{
    status_t ret = NO_ERROR;
    struct timeval sTv;
    struct tm *pTime;
    const char *valstr = NULL;
    char temp_value[256];

    LOGMSG(DBGINFO, "%s ExifEncode::setupEXIF_libjpeg()++", INFOHEAD);
    //initial
    
     if (NO_ERROR == ret)  {
        char Model[] = "unkown";
        ret = exifTable->insertElement(TAG_MODEL, Model);
        LOGMSG(DBGINFO, "%s exif-mode ret=%d", INFOHEAD,ret);
     }
     if (NO_ERROR == ret)  {
        char Make[] = "unkown";
        ret = exifTable->insertElement(TAG_MAKE, Make);
        LOGMSG(DBGINFO, "%s exif-make ret=%d", INFOHEAD,ret);
     }

    if ((NO_ERROR == ret)) {
		 valstr = mHal->mParameters.get(CameraParameters::KEY_FOCAL_LENGTH);
         
         unsigned int focalNum=0, focalDen=0;
         if(valstr){
             exifTable->stringToRational(valstr, &focalNum, &focalDen);
             LOGMSG(DBGINFO, "%s focalNum=%d, focalDen=%d ret=%d", INFOHEAD,focalNum, focalDen,ret);
         }

         snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u/%u",
                 focalNum,
                 focalDen);

         ret = exifTable->insertElement(TAG_FOCALLENGTH, temp_value);
         LOGMSG(DBGINFO, "%s focallength ret=%d", INFOHEAD,ret);
    }

    if ((NO_ERROR == ret)) {
        int status = gettimeofday (&sTv, NULL);
        pTime = localtime (&sTv.tv_sec);
        char tmp_value[EXIF_DATE_TIME_SIZE + 1];
        if ((0 == status) && (NULL != pTime)) {
            snprintf(tmp_value, EXIF_DATE_TIME_SIZE,
                     "%04d:%02d:%02d %02d:%02d:%02d",
                     pTime->tm_year + 1900,
                     pTime->tm_mon + 1,
                     pTime->tm_mday,
                     pTime->tm_hour,
                     pTime->tm_min,
                     pTime->tm_sec );
            ret = exifTable->insertElement(TAG_DATETIME, tmp_value);
            LOGMSG(DBGINFO, "%s datetime ret=%d", INFOHEAD,ret);
        }
     }

    if ((NO_ERROR == ret)) {
        int pic_width, pic_height;
        char tmp_value[5];
        mHal->mParameters.getPictureSize(&pic_width, &pic_height);
        LOGMSG(DBGINFO, "%s pic_width=%d pic_height = %d\n",INFOHEAD,pic_width,pic_height);
        if((pic_width  < 0) || (pic_height < 0))
        {
            ret= -1; 
        }
        else{
            snprintf(tmp_value, sizeof(tmp_value)/sizeof(char), "%u", pic_width);
            ret = exifTable->insertElement(TAG_IMAGE_WIDTH, tmp_value);
            LOGMSG(DBGINFO, "%s image width ret=%d", INFOHEAD,ret);

            snprintf(tmp_value, sizeof(tmp_value)/sizeof(char), "%u", pic_height);
            ret = exifTable->insertElement(TAG_IMAGE_LENGTH, tmp_value);
            LOGMSG(DBGINFO, "%s image length ret=%d", INFOHEAD,ret);
        }
    }


    if (NO_ERROR == ret) {
		 valstr = mHal->mParameters.get(CameraParameters::KEY_GPS_LATITUDE);
         if(valstr){
            double gpsPos = strtod(valstr, NULL);
            int latDeg, latMin, latSec, latSecDiv;
            char latRef[2];

            if(convertGPSCoord(gpsPos, latDeg,latMin, latSec, latSecDiv) == NO_ERROR){
                if(0 < gpsPos){
                    strncpy(latRef, "N", 2);
                }else{
                    strncpy(latRef, "S", 2);
                }

                snprintf(temp_value,
                        sizeof(temp_value)/sizeof(char) - 1,
                        "%d/%d,%d/%d,%d/%d",
                        abs(latDeg), 1,
                        abs(latMin), 1,
                        abs(latSec), abs(latSecDiv));

                ret = exifTable->insertElement(TAG_GPS_LAT, temp_value);
                LOGMSG(DBGINFO, "%s GPS_LAT ret=%d", INFOHEAD,ret);

                ret = exifTable->insertElement(TAG_GPS_LAT_REF, latRef);
                LOGMSG(DBGINFO, "%s GPS_LAT_REF ret=%d", INFOHEAD,ret);
            }
         }
    }

    if (NO_ERROR == ret) {
        valstr = mHal->mParameters.get(CameraParameters::KEY_GPS_LONGITUDE);

        if(valstr){
            double gpsPos = strtod(valstr, NULL);
            int longDeg, longMin, longSec, longSecDiv;
            char longRef[2];

            if(convertGPSCoord(gpsPos, longDeg, longMin, longSec, longSecDiv) == NO_ERROR){
                if(0 < gpsPos){
                    strncpy(longRef, "E", 2);
                }else{
                    strncpy(longRef, "W", 2);
                }

                snprintf(temp_value,
                        sizeof(temp_value)/sizeof(char) - 1,
                        "%d/%d,%d/%d,%d/%d",
                        abs(longDeg), 1,
                        abs(longMin), 1,
                        abs(longSec), abs(longSecDiv));


                ret = exifTable->insertElement(TAG_GPS_LONG, temp_value);
                LOGMSG(DBGINFO, "%s GPS_LAT_LONG ret=%d", INFOHEAD,ret);

                ret = exifTable->insertElement(TAG_GPS_LONG_REF, longRef);
                LOGMSG(DBGINFO, "%s GPS_LAT_LONG_REF ret=%d", INFOHEAD,ret);
            }
        }
    }

    if (NO_ERROR == ret) {
	    valstr = mHal->mParameters.get(CameraParameters::KEY_GPS_ALTITUDE);
        if(valstr){ 
            double gpsPos = strtod(valstr, NULL);
            int altitude = floor(fabs(gpsPos));
            unsigned char altitudeRef = (gpsPos < 0) ? 1 : 0;

            snprintf(temp_value,
                    sizeof(temp_value)/sizeof(char) - 1,
                    "%d/%d",
                    abs(altitude), 1);

            ret = exifTable->insertElement(TAG_GPS_ALT, temp_value);
            LOGMSG(DBGINFO, "%s GPS_ALT ret=%d", INFOHEAD,ret);

            snprintf(temp_value,
                    sizeof(temp_value)/sizeof(char) - 1,
                    "%d",
                    altitudeRef);
            ret = exifTable->insertElement(TAG_GPS_ALT_REF, temp_value);
            LOGMSG(DBGINFO, "%s GPS_ALT_REF ret=%d", INFOHEAD,ret);
        }
     }

    if (NO_ERROR == ret) {
        const char *proc_method = NULL; // arbitrarily long string
		proc_method = mHal->mParameters.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
        if(proc_method ){
            char procMethod[100];
            char tmp_value[GPS_PROCESSING_SIZE] = {0};
            strncpy(procMethod, proc_method, GPS_PROCESSING_SIZE-1);

            memcpy(tmp_value, ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
            memcpy(tmp_value + sizeof(ExifAsciiPrefix), procMethod, (GPS_PROCESSING_SIZE - sizeof(ExifAsciiPrefix)));

            ret = exifTable->insertElement(TAG_GPS_PROCESSING_METHOD, tmp_value);
            LOGMSG(DBGINFO, "%s GPS_PROCESSING_METHOD ret=%d", INFOHEAD,ret);
        }
    }

    //not support now
    #if 0         
    if(NO_ERROR == ret){
        if(valstr != NULL){
            char versionId[4]={0};
            strncpy(versionId, temp_value, 4-1);
            snprintf(temp_value,
                    sizeof(temp_value)/sizeof(char) - 1,
                    "%d,%d,%d,%d",
                    versionId[0],
                    versionId[1],
                    versionId[2],
                    versionId[3]);
            ret = exifTable->insertElement(TAG_GPS_VERSION_ID, temp_value);
        }
    }
    #endif       

    if (NO_ERROR == ret) {
        valstr = mHal->mParameters.get(CameraParameters::KEY_GPS_TIMESTAMP);
        if(valstr){
            long gpsTimestamp = strtol(valstr, NULL, 10); 
            struct tm *timeinfo = gmtime( ( time_t * ) & (gpsTimestamp));
            if ( NULL != timeinfo ){
                snprintf(temp_value,
                        sizeof(temp_value)/sizeof(char) - 1,
                        "%d/%d,%d/%d,%d/%d",
                        timeinfo->tm_hour, 1,
                        timeinfo->tm_min, 1,
                        timeinfo->tm_sec, 1);
                
                ret = exifTable->insertElement(TAG_GPS_TIMESTAMP, temp_value);
                LOGMSG(DBGINFO, "%s GPS_TIMESTAMP ret=%d", INFOHEAD,ret);
            }
        }
    }

    if (NO_ERROR == ret) {
        char dateStamp[GPS_DATESTAMP_SIZE];
        valstr = mHal->mParameters.get(CameraParameters::KEY_GPS_TIMESTAMP);
        if(valstr){
            long gpsDatestamp = strtol(valstr, NULL, 10);
            struct tm *timeinfo = gmtime( ( time_t * ) & (gpsDatestamp) );

            if(timeinfo){
                strftime(dateStamp, GPS_DATESTAMP_SIZE, "%Y:%m:%d", timeinfo);
                ret = exifTable->insertElement(TAG_GPS_DATESTAMP, dateStamp);
                LOGMSG(DBGINFO, "%s GPS_DATESTAMP ret=%d", INFOHEAD,ret);
            }
        }
    }
//not support now
#if 0           
    if(NO_ERROR == ret){
        if(valstr != NULL){
            char mapDatum[100];
            strncpy(mapDatum, valstr, 100-1);
            ret = exifTable->insertElement(TAG_GPS_MAP_DATUM, mapDatum);
        }
    }
#endif      


    // fill in short and ushort tags
    if (NO_ERROR == ret) {
        char tmp_value[2];
        tmp_value[1] = '\0';

        // AWB auto
        tmp_value[0] = '0';
        exifTable->insertElement(TAG_WHITEBALANCE, tmp_value);

        // MeteringMode
        // TODO(XXX): only supporting this metering mode at the moment, may change in future
        tmp_value[0] = '2';
        exifTable->insertElement(TAG_METERING_MODE, tmp_value);

        // ExposureProgram
        // TODO(XXX): only supporting this exposure program at the moment, may change in future
        tmp_value[0] = '3';
        exifTable->insertElement(TAG_EXPOSURE_PROGRAM, tmp_value);

        // ColorSpace
        tmp_value[0] = '1';
        exifTable->insertElement(TAG_COLOR_SPACE, tmp_value);

        tmp_value[0] = '2';
        exifTable->insertElement(TAG_SENSING_METHOD, tmp_value);

        tmp_value[0] = '1';
        exifTable->insertElement(TAG_CUSTOM_RENDERED, tmp_value);
    }

    if (NO_ERROR == ret) {
        unsigned int numerator = 0, denominator = 0;
        unsigned int temp_num = 0;

		//Orientation
		int a_orient_val  = mHal->mParameters.getInt(CameraParameters::KEY_ROTATION);
		if(-1 == a_orient_val)a_orient_val = 0;
		switch(a_orient_val){
			case 0:
				temp_num = 1;
				break;
			case 90:
				temp_num = 6;
				break;
			case 180:
				temp_num = 3;
				break;
			case 270:
				temp_num = 8;
				break;
			default:
				LOGMSG(DBGERR, "%s unspported orientation! ", ERRHEAD);
				temp_num = 1;

		}

		snprintf(temp_value, sizeof(temp_value)/sizeof(char), "%u", temp_num);
        exifTable->insertElement(TAG_ORIENTATION, temp_value);
		

        // DigitalZoomRatio
		int a_zoom_val = mHal->mParameters.getInt(CameraParameters::KEY_ZOOM);
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u/%u",
                 a_zoom_val, 1024);
        exifTable->insertElement(TAG_DIGITALZOOMRATIO, temp_value);

        // ExposureTime
		int a_exp_val = mHal->mParameters.getInt(CameraParameters::KEY_EXPOSURE_COMPENSATION);
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u/%u",
                 a_exp_val, 1000000);
        exifTable->insertElement(TAG_EXPOSURETIME, temp_value);

        // ApertureValue and FNumber
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u/%u",
                 20, 100);
        exifTable->insertElement(TAG_FNUMBER, temp_value);
        exifTable->insertElement(TAG_APERTURE, temp_value);

        // ISO
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u,0,0",
                 100);
        exifTable->insertElement(TAG_ISO_EQUIVALENT, temp_value);

        // ShutterSpeed
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%f",
                 log(a_exp_val) / log(2));
        ExifElementsTable::stringToRational(temp_value, &numerator, &denominator);
        snprintf(temp_value, sizeof(temp_value)/sizeof(char), "%u/%u", numerator, denominator);
        exifTable->insertElement(TAG_SHUTTERSPEED, temp_value);

        // Flash
           temp_num = 0x18; // Flash did not fire, auto mode
        snprintf(temp_value,
                 sizeof(temp_value)/sizeof(char),
                 "%u", temp_num);
        exifTable->insertElement(TAG_FLASH, temp_value);

        unsigned int lightsource = 0;

        // stole this from framework/tools_library/src/tools_sys_exif_tags.c
        lightsource = 1; // Daylight

        snprintf(temp_value,
                sizeof(temp_value)/sizeof(char),
                "%u", lightsource);
        exifTable->insertElement(TAG_LIGHT_SOURCE, temp_value);
    }
    
    LOGMSG(DBGINFO, "%s ExifEncode::setupEXIF_libjpeg()--", INFOHEAD);

    return ret;
}

int ExifEncode::insertExifJpeg(ExifElementsTable* exifTable,int *data_len)
{
    int ret = IM_RET_OK;
    camera_memory_t* picture = NULL;
    Section_t* exif_section = NULL;
    int jpeg_size  = 0; 
    int thumb_size  = 0; 
    
    LOGMSG(DBGINFO, "%s ExifEncode::insertExifJpeg()++", INFOHEAD);

    //setup the EXIF lib
    ret = setupEXIF_libjpeg(exifTable);
	if(ret != NO_ERROR){
        LOGMSG(DBGERR, "%s ExifEncode::setupEXIF_libjpeg fail! ret= %d", ERRHEAD,ret);
		return IM_RET_FAILED;
    }
    jpeg_size = mHal->mJencParam.jfifSize;
    thumb_size = mHal->mJencCfg.thumb.dataLength;
    if(mHal->mJpegOutBuff.vir_addr){   
        exifTable->insertExifToJpeg((unsigned char*)mHal->mJpegOutBuff.vir_addr, jpeg_size);
    }
    else
    {
        LOGMSG(DBGERR, "%s No Jpeg image! ", ERRHEAD);
    }    
    
    if(mHal->r_thumb_width > 0) {
        exifTable->insertExifThumbnailImage((const char*)mHal->mThumbBuff.vir_addr,
                (int)thumb_size);
    }

    exif_section = FindSection(M_EXIF);

    if (exif_section) {
        *data_len = jpeg_size + exif_section->Size;
    }
    else
    {
        *data_len = 0;
    } 
    LOGMSG(DBGINFO, "%s ExifEncode::insertExifJpeg()--", INFOHEAD);
    return IM_RET_OK;
    
}

int ExifEncode::createExifJpeg(ExifElementsTable* exifTable,int *data,int data_len)
{     
    LOGMSG(DBGINFO, "%s ExifEncode::creatExifJpeg()++", INFOHEAD);
    if (data) {
           exifTable->saveJpeg((unsigned char*) data, data_len);
       }
    else{
        LOGMSG(DBGERR, "%s creatExifJpeg: data is  NULL!", ERRHEAD);
        return IM_RET_FAILED;
    }

    LOGMSG(DBGINFO, "%s ExifEncode::creatExifJpeg()--", INFOHEAD);
    return IM_RET_OK;
}

}
