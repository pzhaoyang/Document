#ifndef __GOKU_GPS_MAIN_H__
#define __GOKU_GPS_MAIN_H__
#include "goku_include.h"
#include "goku_gps_protocol_manager.h"
#include "goku_gps_socket.h"
#include "sc_net.h"
#include "sc_pdp.h"

#define GOKU_GPS_RECDATA_THREAD_NAME          "GOKU_GPS_RECDATA_THREAD_NAME"
#define GOKU_GPS_PDP_ACTIVE_THREAD_NAME       "GOKU_GPS_PDP_ACTIVE_THREAD_NAME"
#define GOKU_GPS_SENDDATA_THREAD_NAME         "GOKU_GPS_SENDDATA_THREAD_NAME"
#define GOKU_GPS_TM_SYNC_TIMER_NAME           "GOKU_GPS_TM_SYNC_TIMER_NAME"
#define GOKU_GPS_BEAT_TIMER_NAME              "GOKU_GPS_BEAT_TIMER_NAME"
#define GOKU_GPS_UP_PNT_TIMER_NAME            "GOKU_GPS_UP_PNT_TIMER_NAME"
#define GOKU_GPS_UP_PNT_RETRY_TIMER_NAME      "GOKU_GPS_UP_PNT_RETRY_TIMER_NAME"
#define GOKU_GPS_FORCE_UPLOAD_TIMER_NAME      "GOKU_GPS_FORCE_UPLOAD_TIMER_NAME"
#define GOKU_GPS_POWEROFF_UPLOAD_TIMER_NAME   "GOKU_GPS_POWEROFF_UPLOAD_TIMER_NAME"
#define GOKU_GPS_LOC_GET_CHANNEL_NAME         "GOKU_GPS_LOC_GET_CHANNEL_NAME"
#define GOKU_GPS_QUEUE_NAME                   "GOKU_GPS_QUEUE_NAME"         
//#define GPS_MSG_HANDLE_QUEUE_NAME           "GPS_MSG_HANDLE_QUEUE_NAME"

//#define GOKU_GPS_MSG_HANDLE_QUEUE_MAX_NUM           64//��������
#define GOKU_GPS_RECDATA_THREAD_NAME_PRI            200
#define GOKU_GPS_RECDATA_THREAD_STACK_SIZE          1024*4

#define GOKU_GPS_SENDDATA_THREAD_NAME_PRI           (30*8)
#define GOKU_GPS_SENDDATA_THREAD_STACK_SIZE         1024*8

#define GOKU_GPS_PDP_ACTIVE_THREAD_NAME_PRI           200
#define GOKU_GPS_PDP_ACTIVE_THREAD_STACK_SIZE         1024*1

#define GOKU_GPS_QUEUE_MAX_LEN                      64

#define GOKU_GPS_TM_SYNC_TIMER_LEN                  GOKU_GPS_ONE_MIN_LEN//ͬ�����ʱ��
#define GOKU_GPS_UP_PNT_TIMER_LIMT                  GOKU_GPS_ONE_MIN_LEN
#define GOKU_GPS_UP_PNT_RETRY_TIMER_LEN             60*GOKU_GPS_ONE_SEC_LEN
#define GOKU_GPS_DELAY_TIME                         1000 //1000ms
#define GOKU_GPS_POWER_LOCATE_TIME                  5*GOKU_GPS_ONE_SEC_LEN
#define GOKU_GPS_POWER_TIMEOUT_TIME                 10*GOKU_GPS_ONE_SEC_LEN
#define GOKU_GPS_PDP_DEACTIVE_TIME                  15*GOKU_GPS_ONE_SEC_LEN

#define GOKU_GPS_SYNC_RETRY_CNT                     3
#define GOKU_GPS_BEAT_RETRY_CNT                     3
#define GOKU_GPS_SEND_RETRY_CNT                     3

#define GOKU_GPS_SEMA_WAIT_TIME                  (20*1000)//20s

#endif
