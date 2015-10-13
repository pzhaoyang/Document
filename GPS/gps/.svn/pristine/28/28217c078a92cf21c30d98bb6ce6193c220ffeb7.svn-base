#ifndef __GOKU_GPS_SOCKET_H__
#define __GOKU_GPS_SOCKET_H__
#include "goku_include.h"
#include "unipro_socket.h"
#include "goku_gps_protocol_manager.h"

#define GPS_REC_DATA_BUF_LEN                (1*1024)
#define GPS_UPLOADCMD_LEN                   (44)
#define GPS_UPLOADCMD_MAX_CNT               (512)
#define GPS_BUFFER_LEN                      (GPS_UPLOADCMD_LEN*GPS_UPLOADCMD_MAX_CNT)
#define GPS_BUFFER_MAX_COUNT                20
#define GPS_BUFFER_MAX_LEN                  (GPS_BUFFER_LEN*GPS_BUFFER_MAX_COUNT)
#define GPS_SOCKET_MAX_UPLOAD_CNT           (30)
#define GPS_SOKET_MAX_LEN                   (GPS_UPLOADCMD_LEN*GPS_SOCKET_MAX_UPLOAD_CNT)

#define GOKU_GPS_SERVICE_HOST                    "101.226.241.38"
#define GOKU_GPS_SERVICE_PORT                    7081

//通知消息包
typedef struct _GPS_OP_PACKET_
{
	GPS_CMD_CMD_E opType;
	gint len;	
	VOID *buf;
} GPS_OP_PACKET;

typedef struct _GPS_BUFFER_
{
    UINT32   uCacheLen;
    UINT8*   pCache;
    UINT8*   pWrite;
    UINT8*   pRead;
    UINT32   nTurnFlag;
    UINT32   nFileBufferSize;//存储在文件中的数据大小
    UINT32   nFileIndexRead;//起始的文件下标，从1开始，0代表无效
    UINT32   nFileIndexWrite;//结束的文件下标，从1开始，0代表无效
}GPS_FUFFER;

gboolean  goku_gps_recv(GPS_CMD_CMD_E cmd);
gboolean goku_gps_buffer_read(UINT8* pBuffer,UINT32 nBufferSize);
gboolean goku_gps_buffer_write(UINT8* pBuffer,UINT32 nBufferSize);
UINT32 goku_gps_buffer_get_count();
UINT32 goku_gps_buffer_get_used_size();
void goku_gps_buffer_free();
gboolean goku_gps_buffer_init();
UINT32 goku_gps_print_recv_bytes_total();
void goku_gps_recv_bytes_total_reset();
#endif
