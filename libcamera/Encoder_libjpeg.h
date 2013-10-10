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
* @file Encoder_libjpeg.h
*
* This defines API for camerahal to encode YUV using libjpeg
*
*/

#ifndef ANDROID_CAMERA_HARDWARE_ENCODER_LIBJPEG_H
#define ANDROID_CAMERA_HARDWARE_ENCODER_LIBJPEG_H

#include <utils/threads.h>
#include <utils/RefBase.h>

extern "C" {
#include "jhead.h"
}

#define CANCEL_TIMEOUT 3000000 // 3 seconds

#define EXIF_MODEL_SIZE             100
#define EXIF_MAKE_SIZE              100
#define EXIF_DATE_TIME_SIZE         20

//GPS
#define GPS_DATESTAMP_SIZE          11 
#define GPS_PROCESSING_SIZE         100

namespace android {
/**
 * libjpeg encoder class - uses libjpeg to encode yuv
 */

#define MAX_EXIF_TAGS_SUPPORTED 30

// these have to match strings defined in external/jhead/exif.c
static const char TAG_MODEL[] = "Model";
static const char TAG_MAKE[] = "Make";
static const char TAG_FOCALLENGTH[] = "FocalLength";
static const char TAG_DATETIME[] = "DateTime";
static const char TAG_IMAGE_WIDTH[] = "ImageWidth";
static const char TAG_IMAGE_LENGTH[] = "ImageLength";
static const char TAG_GPS_LAT[] = "GPSLatitude";
static const char TAG_GPS_LAT_REF[] = "GPSLatitudeRef";
static const char TAG_GPS_LONG[] = "GPSLongitude";
static const char TAG_GPS_LONG_REF[] = "GPSLongitudeRef";
static const char TAG_GPS_ALT[] = "GPSAltitude";
static const char TAG_GPS_ALT_REF[] = "GPSAltitudeRef";
static const char TAG_GPS_MAP_DATUM[] = "GPSMapDatum";
static const char TAG_GPS_PROCESSING_METHOD[] = "GPSProcessingMethod";
static const char TAG_GPS_VERSION_ID[] = "GPSVersionID";
static const char TAG_GPS_TIMESTAMP[] = "GPSTimeStamp";
static const char TAG_GPS_DATESTAMP[] = "GPSDateStamp";
static const char TAG_ORIENTATION[] = "Orientation";
static const char TAG_FLASH[] = "Flash";
static const char TAG_DIGITALZOOMRATIO[] = "DigitalZoomRatio";
static const char TAG_EXPOSURETIME[] = "ExposureTime";
static const char TAG_APERTURE[] = "ApertureValue";
static const char TAG_ISO_EQUIVALENT[] = "ISOSpeedRatings";
static const char TAG_WHITEBALANCE[] = "WhiteBalance";
static const char TAG_LIGHT_SOURCE[] = "LightSource";
static const char TAG_METERING_MODE[] = "MeteringMode";
static const char TAG_EXPOSURE_PROGRAM[] = "ExposureProgram";
static const char TAG_COLOR_SPACE[] = "ColorSpace";
static const char TAG_CPRS_BITS_PER_PIXEL[] = "CompressedBitsPerPixel";
static const char TAG_FNUMBER[] = "FNumber";
static const char TAG_SHUTTERSPEED[] = "ShutterSpeedValue";
static const char TAG_SENSING_METHOD[] = "SensingMethod";
static const char TAG_CUSTOM_RENDERED[] = "CustomRendered";

class ExifElementsTable {
    public:
        ExifElementsTable() :
           gps_tag_count(0), exif_tag_count(0), position(0),
           jpeg_opened(false), has_datetime_tag(false) { }
        ~ExifElementsTable();

        status_t insertElement(const char* tag, const char* value);
        void insertExifToJpeg(unsigned char* jpeg, size_t jpeg_size);
        status_t insertExifThumbnailImage(const char*, int);
        void saveJpeg(unsigned char* picture, size_t jpeg_size);
       
        static bool isAsciiTag(const char* tag);
        static void stringToRational(const char*, unsigned int*, unsigned int*);
    private:
        ExifElement_t table[MAX_EXIF_TAGS_SUPPORTED];
        unsigned int gps_tag_count;
        unsigned int exif_tag_count;
        unsigned int position;
        bool jpeg_opened;
        bool has_datetime_tag;
};

class ExifEncode{
    private:
        CameraHal *mHal;
    public:
        ExifEncode(CameraHal *hal){
           mHal = hal;
        };
        ~ExifEncode();

        status_t setupEXIF_libjpeg(ExifElementsTable* exifTable);
        int insertExifJpeg(ExifElementsTable* exifTablem,int *data_len);
        int createExifJpeg(ExifElementsTable* exifTable,int *data,int data_len);
    };

}

#endif
