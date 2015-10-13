/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_notifications.h>
*
* Description	: <describing what this file is to do>
*
* Notes		    : <the limitations to use this file>
*
*-------------------------------------------------------------------------------
*
* Change History:
* 
*-------------------------------------------------------------------------------        
*
*
*
*******************************************************************************/
#ifndef __GOKU_GPS_NOTIFICATIONS_H__
#define __GOKU_GPS_NOTIFICATIONS_H__

#define NOTIFY_GPS_FIX_OK    "NOTIFY_GPS_FIX_OK"
#define NOTIFY_GPS_FIX_FAIL  "NOTIFY_GPS_FIX_FAIL"
#define NOTIFY_GPS_FIX_STATUS  "NOTIFY_GPS_FIX_STATUS"
#define NOTIFY_AGPS_SUCCESS  "NOTIFY_AGPS_SUCCESS"
#define NOTIFY_AGPS_FAIL     "NOTIFY_AGPS_FAIL"
#define NOTIFY_SAT_STATUS     "NOTIFY_SAT_STATUS"
#define NOTIFY_GPS_CHECK_FAIL     "NOTIFY_GPS_CHECK_FAIL"
#define NOTIFY_GPS_CHECK_OK     "NOTIFY_GPS_CHECK_OK"

typedef struct GPSInfo
{
    int year;
    int mon;
    int day;
    int hour;
    int min;
    float sec;

    float Lat;      // 纬度 -南 +北
    float Lon;      //经度 - 西 +东
    float Alt;
    unsigned char FixService;  // NoFix:0, SPS:1, DGPS:2, Estimate:6
    unsigned char FixType;     // None:0, 2D:1, 3D:2
    float Speed;  // km/hr
    float Track;  // 0~360
    float PDOP;   //DOP
    float HDOP;
    float VDOP;

    int SV_cnt;//卫星数
    int fixSV[20];
    int loc_min;
    float loc_sec;
}GPSInfo;

typedef struct SVInfo
{
    int SVid;            // PRN
    int SNR;
    int elv;             // elevation angle : 0~90
    int azimuth;         // azimuth : 0~360
    unsigned char Fix;   // 0:None , 1:FixSV
} SVInfo;

#endif
