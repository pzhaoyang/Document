#ifndef __GOKU_GPS_PROTOCOL_MANAGER_H__
#define __GOKU_GPS_PROTOCOL_MANAGER_H__
/******************************************************************************************
*pengzhaoyang GPS 协议文件
*******************************************************************************************/
#include "goku_include.h"
#include "unipro_gal_gpspub.h"
#include "date.h"
#include "battery.h"

#define GPS_FREE(ptr) \
    { \
        if (NULL != ptr) \
        { \
            tp_os_mem_free(ptr); \
            ptr = NULL; \
        } \
    }
    
//相关宏定义
#define GPS_PROTOCOL_PRODUCER           "BG"
#define GPS_PROTOCOL_HEAD(producer)     "*"# producer","
#define GPS_PROTOCOL_TAIL               '#'
#define GPS_PROTOCOL_COMMA              ','
#define GPS_PROTOCOL_PAC_HEAD           '$'

#define GPS_DATE_LENG                   7  // "YYMMDD"
#define GPS_TIME_LENG                   7  // "HHMMSS"
#define GPS_DEV_NUM_LENG                11 //设备号为11位
#define GPS_CMD_LEN                     3//cmd命令的最大长度
#define GPS_TYPE_LEN                    1//mms,gprs
#define GPS_PARAM_LEN                   400//param最大的长度
#define GPS_INTERVAL_LEN                10//最大长度为65535
#define GPS_TOTAL_CNT_LEN               10//最大长度为65535
#define GPS_LOCATEABLE_LEN              10// 只能为0 ,1
#define GPS_CMDSTR_LEN                  (1+2+1+GPS_DEV_NUM_LENG+1+GPS_CMD_LEN+GPS_TIME_LENG+GPS_PARAM_LEN+1+1)
#define GPS_SOCKET_HOST_NAME_LEN        64//主机地址"10.0.0.172"最大长度为64
#define GPS_PROTOCOL_UPLOAD_MAX         44

#define GPS_PROTOCOL_MIN_INTERVAL       2

#define GPS_PROTOCOL_MIN_MAXN           1
#define GPS_PROTOCOL_MAX_MAXN           200

#define GPS_UPLOAD_MAXCNT                           (10)

#define GOKU_GPS_ONE_SEC_LEN                        (1)
#define GOKU_GPS_ONE_MIN_LEN                        60*GOKU_GPS_ONE_SEC_LEN
#define GOKU_GPS_ONE_HOUR_LEN                       60*GOKU_GPS_ONE_MIN_LEN

#define GOKU_GPS_24_TM_SYNC_TIMER_LEN               24*GOKU_GPS_ONE_HOUR_LEN
#define GOKU_GPS_BEAT_TIMER_LEN                     GOKU_GPS_ONE_HOUR_LEN
#define GOKU_GPS_UP_PNT_TIMER_LEN                   10*GOKU_GPS_ONE_SEC_LEN
#define GOKU_GPS_FORCE_UPLOAD_TIMER_LEN             10*GOKU_GPS_ONE_MIN_LEN

//cmd 类型
typedef enum  {
    SET_SERVER_NULL = 0X00,//开始
    SET_SERVER_SMS_NUM,//设置服务器短信中心号码
    SET_SERVER_IP_PORT,//设置服务器IP地址和端口号
    SET_SERVER_LINK_NUM,//设置联系人号码(前四个固定)
    SET_DOMAIN_PORT,//设置域名和IP
    SET_SERVER_MULT_CMD,//设置监控中心号，端口，域名端口，联系人号码
    SET_SERVER_LOCATE_CONFIG,//设置定位条件(定位间隔，上传条数)
    GET_TIME_SYNC_REQ,//获取下次同步德时间间隔请求
    SET_SERVER_LOCATE_ENABLE,//设置终端是否打开，关闭定位功能
    SET_SERVER_RUNING_MODE,//设置节能运行模式
    SET_SERVER_APN_TMZONE,//设置APN和时区
    SET_SERVER_FENCE,//设置围栏
    SET_LOCAL_UPLOAD,//上传
    SET_LOCLA_BEAT_PACK,//心跳包
    SET_LOCAL_UPLOAD_SINGLE_POINT,//上传最后一个点
    SET_SERVER_MAX,//最大值
}GPS_CMD_CMD_E;
//函数定义
typedef struct {
    GPS_CMD_CMD_E cmd;
    gchar cmdstr[GPS_CMD_LEN];
}GPS_CMD_T;

typedef struct {
    guint syn_intval;
    char dev_num[GPS_DEV_NUM_LENG+1];
    char cmdstr[GPS_CMD_LEN+1];
    char dtstr[GPS_DATE_LENG+1];
    char tmstr[GPS_TIME_LENG+1];
    char parm[GPS_PARAM_LEN+1];
    guint type;
}cmdstr_info;

typedef enum {
    RADIX_DEC = 0X00,
    RADIX_HEX,
    RADIX_MAX
}RADIX_E;

typedef struct {
    guint interval;
    guint total_cnt;
}locate_config;

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

typedef struct GPS_CFG_T{
    char   renewTime[20];
    guint  interval;
    guint  queryInterval;
    guint  countPerPackage;
    guint  result;
    guint  muteEnabled;
    char   serverDomainName[100];
    guint  manuNo;
    char   link1[20];
    char   link2[20];
    char   link3[20];
    char   link4[20];
    char   controlNumber[20];
    guint  bindingFlag;
    char   serverIp[20];
    guint  limitCallEnabled;
    char   trackerSN[12];
    guint  serverPort;
    guint  agsensorGrade;
}GPS_CFG_T;

gboolean goku_gps_cmdstr_decode(gchar* cmdstr,cmdstr_info* cmdstr_info);
gchar* goku_gps_get_dev_num(void);
gchar* goku_gps_get_current_time(RADIX_E radix);
gchar* goku_gps_get_current_date(RADIX_E radix);
gboolean goku_gps_is_legal_cmdstr( gchar* cmdstr);
gboolean goku_gps_get_cmdstr_head(gchar* cmdstr );
gboolean goku_gps_get_cmdstr_tail(gchar* cmdstr,char gps_protocol_tail);
gchar* goku_gps_get_devnum_from_cmdstr(gchar* cmdstr);
gchar* goku_gps_get_cmdstr_by_cmd(GPS_CMD_CMD_E cmd);
gint goku_gps_get_cmd_by_cmdstr(gchar* cmdstr);
gchar* goku_gps_get_cmdstr_from_cmdstr(gchar* cmdstr);
gchar* goku_gps_get_date_from_cmdstr(gchar* cmdstr);
gchar* goku_gps_get_time_from_cmdstr(gchar* cmdstr);
guint  goku_gps_get_cmd_type_from_cmdstr(gchar* cmdstr);
guint  goku_gps_get_syc_interval_from_cmdstr(gchar* cmdstr);
gchar* goku_gps_get_cmdparm_from_cmdstr(gchar* cmdstr);
gboolean goku_gps_cmdparm_decode(GPS_CMD_CMD_E cmd,gchar* cmdparm);
gboolean goku_gps_locateconfig_decode(gchar* cmdparm);
gboolean goku_gps_locable_decode(gchar* cmdparm);
gboolean goku_gps_pack_req_cmdstr(GPS_CMD_CMD_E cmd, gchar* cmdparm,gchar* output);
gchar* goku_gps_tm_sync_req();
void goku_gps_get_imei();
void goku_gps_pac_point(char* parm,char out[],guint n);
gchar* goku_gps_pack_locates();
UINT8* goku_gps_get_lat(float lat);
UINT8*  goku_gps_get_lon(float lon);
UINT8* goku_gps_get_speed_direction(GPSInfo* info);
UINT8* goku_gps_get_vehicle_cbatus();
UINT8* goku_gps_get_mcc(UINT16 mcc);
UINT8* goku_gps_get_mnc(UINT16 mnc);
UINT8* goku_gps_get_lac(UINT16 lac);
UINT8* goku_gps_get_cid(UINT16 cid);
UINT8* goku_gps_convert_value_to_data(char* val,gboolean FILL);
void goku_gps_set_stat(GPSInfo* info);
UINT8 goku_gps_get_stat();
char* goku_gps_valtostring(float gps_info);
UINT8 goku_gps_get_battery();
UINT8 goku_gps_battery_low_flag();
guint goku_gps_cvt_mcc_mnc(UINT16 mcc_mnc);
gboolean goku_gps_set_config_time(char* cfg_tm);
char* goku_gps_get_config_time();
gboolean goku_gps_dcode_config(char* cmdstr);
gboolean goku_gps_get_item_from_config(char* item,char* cmdstr,char* item_val);
void goku_gps_print_LastPoint(GPSInfo * gps_info, MAN_RTC_TIME tTime, MAN_RTC_DATE tDate);
gboolean goku_gps_set_cfg_interval(gint interval);
guint goku_gps_get_cfg_interval();
gboolean goku_gps_set_cfg_maxN(gint maxN);
guint goku_gps_get_cfg_maxN();
gboolean goku_gps_set_cfg_beattime(gint beattime);
guint goku_gps_get_cfg_beattime();
void goku_gps_protocol_data_reset();
void goku_gps_date_time_set(gchar* cmdparm);
guint goku_gps_get_tm_sync_len();
gboolean goku_gps_get_gps_close_flg();
void goku_gps_set_gps_close_flg(gboolean bclose);
void goku_gps_set_recv_interval(guint recv_interval);
guint  goku_gps_get_recv_interval();
void goku_gps_set_recv_maxN(guint maxN);
guint  goku_gps_get_recv_maxN();
#endif
