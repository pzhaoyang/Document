#include "goku_gps_main.h"
#include "goku_notification_client.h"
#include "unipro_socket.h"
#include "dd_dbg.h"
#include "goku_message_hook.h"
#include "unipro_pdp.h"
#ifndef WIN32
#define DEBUG_MODULE_ID UNIPRO_MODULE_MAIN
#include "unipro_debug.h"
#endif

void goku_gps_thread_main_create();
void* goku_gps_onoff();
void goku_gps_create_ex();
void goku_gps_close_ex();
gboolean goku_gps_senddata_thread_create();
void goku_gps_poweroff_pdp_active();
void* goku_gps_poweroff_pdp_active_thread_proc();
VOID goku_gps_senddata_thread_proc(VOID);
gboolean goku_gps_env_init();
gboolean goku_gps_pdp_active();
gboolean goku_gps_tm_sync_timer_create();
gboolean goku_gps_beat_timer_create();
gboolean goku_gps_queue_create();
gboolean goku_gps_up_point_timer_create();
gboolean goku_gps_up_point_retry_timer_create();
gboolean goku_gps_force_upload_timer_create();
gboolean goku_gps_poweroff_upload_timer_create();
VOID goku_gps_tm_sync_timer_handler();
VOID goku_gps_beat_timer_handler();
VOID goku_gps_up_pnt_timer_handler();
VOID goku_gps_up_pnt_retry_timer_handler();
VOID goku_gps_force_upload_timer_handler();
VOID goku_gps_poweroff_upload_timer_handler();
static SINT32 goku_gps_loc_get_msg_register();
static SINT32 goku_gps_loc_get_msg_unregister();
static gint32 goku_gps_loc_get_msg_proc(guint32 msg_id, void* param, guint32 msg_size, VOID *add_data);
gboolean gps_msg_handle_notice(GPS_CMD_CMD_E cmd, VOID *buf, gint len);
gboolean goku_gps_open_network();
void goku_gps_pdp_deactive();
void goku_gps_close_network();
gboolean goku_net_query();
BOOL goku_gps_sim_check();
gboolean goku_gps_msg_send_ex(GPS_OP_PACKET* pMsg);
gboolean goku_gps_msg_send(GPS_OP_PACKET* pMsg);
void goku_gps_sema_delete();
void goku_gps_mutex_delete();
void goku_gps_queue_delete();
void goku_gps_thread_delete();
void goku_gps_timers_delete();
void goku_gps_data_reset();
void goku_gps_set_mode(UINT32 interval,UINT32 maxN);
void goku_gps_get_gps_parm(gps_fix_mode *mod,guint *up_tm_len);
void goku_gps_get_packed_data(GPS_OP_PACKET* op, gchar* cmdparm, 
                                           /*OUT*/ gchar* packeddata, /*OUT*/guint* leng);
gboolean goku_gps_send_data(GPS_OP_PACKET* pMsg,gchar* packeddata,guint leng);
gboolean goku_gps_deal_send_data(GPS_OP_PACKET* pMsg,gboolean bSendSucess);
gboolean goku_gps_queue_deal(GPS_OP_PACKET* pMsg);
void goku_gps_send_first_pnt();
void goku_gps_start_force_upload_timer();
void goku_gps_stop_force_upload_timer();
//static int goku_gps_entry_hook(HWND dst_wnd, int msg, WPARAM wparam, LPARAM lparam, gpointer add_data);
void goku_gps_upload_last_point();
static void goku_gps_get_last_point();
gboolean goku_gps_open_pdp();
void goku_gps_start_gps_chip(gboolean NeedStart,guint interval);
gboolean goku_gps_reset_timer(guint interval);
void goku_gps_start_retry_upload_timer(guint timer_len,guint delay);
void goku_gps_set_retry_upload_timer_len(guint interval, guint maxN);
guint goku_gps_get_retry_upload_timer_len();
gboolean goku_gps_is_timer_running(guint timer_id);
void goku_gps_start_poweroff_upload_timer();
void goku_gps_set_last_point_upload_mode(gboolean fast_mode);
void goku_gps_set_force_upload_timer_len(guint force_len);
guint goku_gps_get_force_upload_timer_len();
void goku_gps_set_last_point_flg(gboolean bWriten);
gboolean goku_gps_get_last_point_flg();
OS_TIMER_ID goku_gps_timer_create(CHAR *name,OS_FUNC_ENTRY expir_func, VOID* expire_para_ptr, UINT32 interval /*unit (s)*/,UINT32 flag);
void goku_gps_timer_start(OS_TIMER_ID id);
void goku_gps_timer_restart(OS_TIMER_ID id,UINT32 interval);
void goku_gps_timer_stop(OS_TIMER_ID id);
void goku_gps_timer_delete(OS_TIMER_ID *id);
void goku_gps_print_sendbytes_total_reset();
void goku_gps_print_flow_total(char* pBuffer);
void goku_gps_flow_reset();
void goku_gps_psm_request(UINT8 nDeviceId);
void goku_gps_psm_release(UINT8 nDeviceId);
gboolean goku_gps_get_current_status();
gboolean goku_gps_set_current_status(gboolean onoff);
void goku_gps_reset_factory();
gboolean goku_gps_get_running_status();
void goku_gps_set_running_status(gboolean running);
OS_STATUS goku_gps_sema_get(OS_SEMA_ID sid, UINT32 timeout);
OS_STATUS goku_gps_sema_put(OS_SEMA_ID sid);
OS_STATUS goku_gps_mutex_get(OS_MUTEX_ID mid, UINT32 timeout);
OS_STATUS goku_gps_mutex_put(OS_MUTEX_ID mid);
void goku_gps_set_file_load_flag(gboolean load_success);
gboolean goku_gps_get_file_load_flag();

//定义一个全局队列句柄
static OS_THREAD_ID MsgHandleSendDataThreadID = OS_THREAD_INVALID_ID;
static OS_QUEUE_ID  MsgHandleGPSQueueID = OS_QUEUE_INVALID_ID;

static UINT32 uTmSyncTimerId = 0;
static UINT32 uBeatTimerId = 0;
static UINT32 uUpPntRetyTimerId = 0;
static UINT32 uForceUploadTimerId = 0;
static UINT32 uPoweroffUploadTimerId = 0;
UINT32 uUpPntTimerId = 0;


static OS_SEMA_ID g_pdp_sema = OS_SEMA_INVALID_ID;
static OS_SEMA_ID g_gps_close_sema = OS_SEMA_INVALID_ID;
static OS_MUTEX_ID g_close_mutex = OS_MUTEX_INVALID_ID;
static OS_MUTEX_ID g_msg_mutex = OS_MUTEX_INVALID_ID;


static guint gps_upload_retry_timer_len = 0;
static gint cmdnum_in_queue[SET_SERVER_MAX] = {0};
static gboolean g_bFirstRuning = TRUE;
static char g_sendcmdstr[GPS_SOKET_MAX_LEN] = {0};
static guint g_sendcmdstr_len = GPS_UPLOADCMD_LEN;
static gboolean g_bFirstBeatMsg = TRUE;
static gboolean g_bFirstStartDriver = TRUE;
static gps_fix_mode g_gps_loc_mode;
static gboolean g_bForceUploadFlg = FALSE;
static gboolean g_bFastUploadLastPoint = FALSE;
static guint g_ForceUpload_timer_len = GOKU_GPS_FORCE_UPLOAD_TIMER_LEN;
static gboolean g_bWriteLastPointSuccess = FALSE;
static UINT32   g_uSendTotalByte = 0;
static gboolean g_gps_current_status = FALSE;
static gboolean g_gps_has_point = FALSE;
static guint g_uSyncRetryCnt = 0;
static guint g_uBeatRetryCnt = 0;
gboolean g_bPressPowerFlg = FALSE;
static gboolean g_bRunningStatus = FALSE;
static gboolean    g_b_first_load_file_success_flag = FALSE;
SINT32 g_sChipOK = -1;// -1:不确定，0没有芯片,1有芯片

extern UINT8 g_imei_array[6];
extern gint goku_poc_get_status();

static void goku_gps_sendbtyes_total(UINT32 sendbytes)
{
    g_uSendTotalByte += sendbytes;
    PRINT_FUNC_LINE("g_uSendTotalByte = %ld bytes",g_uSendTotalByte);
}

UINT32 goku_gps_print_sendbytes_total()
{
    return g_uSendTotalByte;
}
void goku_gps_get_print_main(char* pBuffer)
{
    sprintf(pBuffer,"%d,%d,\r\nmain:\r\nchip:%d,mode:%d,Fst:%d,poc:%d,fu:%d,fl:%d,fst:%d\r\n",
        goku_gps_get_cfg_interval(),goku_gps_get_cfg_maxN(),
        g_sChipOK,g_gps_loc_mode,g_bFirstRuning,goku_poc_get_status(),g_bForceUploadFlg,goku_gps_get_force_upload_timer_len(),g_bFastUploadLastPoint);
    PRINT_FUNC_LINE("pBuffer = %s",pBuffer);
}


void goku_gps_thread_main_create()
{
	pthread_t threadId;
	pthread_attr_t attr = {0};	
    struct sched_param sp = {0};

    // [UNIPRO-pengzhaoyang-2013-11-26] for 防止用户在没有GPS芯片的机器上再次开启或者关闭GPS，导致不必要的运行隐患 
    if(0 == g_sChipOK){
        PRINT_FUNC_LINE("No Gps Chip.");
        return;
    }
    
    //according to opreate progressing.
    if(goku_gps_get_current_status()){
       PRINT_FUNC_LINE("Operation Progressing...");
       return;
    }
    goku_gps_set_current_status(TRUE);
    
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 4*1024);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr,SCHED_RR);

    pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 25;
    pthread_attr_setschedparam(&attr, &sp);  
    
	pthread_create(&threadId,&attr,goku_gps_onoff,NULL);
	pthread_attr_destroy(&attr);
}
void* goku_gps_onoff()
{
    if(!g_bPressPowerFlg && !goku_gps_get_running_status()){
        PRINT_FUNC_LINE("=== GPS OPEN ===");
        goku_gps_create_ex(); 
    }else{
        PRINT_FUNC_LINE("=== GPS CLOSE ===");    
        tp_comm_channel_post_msg(NULL, TPM_GPS_CLOSE_GPS, 0, 0);
        PRINT_FUNC_LINE("gps_close_sema before");
        goku_gps_sema_get(g_gps_close_sema,OS_WAIT_FOREVER);
        PRINT_FUNC_LINE("gps_close_sema after");
        goku_gps_close_ex();
        if(g_bPressPowerFlg){
            PRINT_FUNC_LINE("=== Power Off ===");            
            goku_power_off(GOKU_POWER_OFF_SHUT_DOWN);
        }
    }
    
    // set gps on or off status
    goku_gps_set_current_status(FALSE);
    pthread_exit(0);
    PRINT_FUNC_LINE("=================end=======================");
}

void goku_gps_create_ex()
{
    gboolean bRet = FALSE;
    GokuNotificationClient * notification_service = goku_notification_client_new();
    
    PRINT_FUNC_LINE("=================enter=======================");
    
    while(!goku_net_query()){
        tp_os_thread_sleep(1000);
    }
    
    if(TRUE == goku_gps_env_init()){
    
		goku_notification_client_notify_status_icon(notification_service, "GOKU_GPS_NOTIFICATION_ICON_NAME","/flash/app/manager/notification/res/image-240x320/statusbar_gps.png",0);
        gps_msg_handle_notice(GET_TIME_SYNC_REQ,NULL,0);//时间同步
        goku_gps_set_running_status(TRUE);
    }else{
//        goku_deregister_message_hook(goku_gps_entry_hook);
        goku_notification_client_cancel_icon(notification_service, "GOKU_GPS_NOTIFICATION_ICON_NAME");    
        PRINT_FUNC_LINE("GPS env init Failed");
        return;
    }
}
void goku_gps_close_ex()
{
    gboolean bRet = FALSE;
	GokuNotificationClient * notification_service = goku_notification_client_new();
	
    PRINT_FUNC_LINE("=================enter=======================");
    
#ifndef WIN32
    unipro_man_gps_close();
#endif
//关闭消息
    goku_gps_timers_delete();
    
//关闭线程    
    goku_gps_thread_delete();
    
//关闭状态    
    unipro_socket_close(ENUM_SOCKET_TYPE_GPS);
    goku_gps_pdp_deactive();

//关闭通道消息    
    goku_gps_loc_get_msg_unregister();
//关闭信号量    
    goku_gps_sema_delete();
    goku_gps_mutex_delete();
    goku_gps_queue_delete();

//清除数据    
    goku_gps_buffer_free();
    goku_gps_data_reset();

//去注册hook,关闭图标    
    goku_notification_client_cancel_icon(notification_service, "GOKU_GPS_NOTIFICATION_ICON_NAME");
    
    PRINT_FUNC_LINE("=================end=======================");
}

//GPS 线程初始化
gboolean goku_gps_senddata_thread_create()
{
    PRINT_FUNC_LINE("=================enter==============");
    MsgHandleSendDataThreadID = tp_os_thread_create(GOKU_GPS_SENDDATA_THREAD_NAME,
                                             (OS_THREAD_ENTRY)goku_gps_senddata_thread_proc,
                                             GOKU_GPS_SENDDATA_THREAD_NAME_PRI,
                                             GOKU_GPS_SENDDATA_THREAD_STACK_SIZE,
                                             NULL);

    if(OS_THREAD_INVALID_ID == MsgHandleSendDataThreadID)
    {
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxx GPS senddata Thread Create Failed xxxxxxxxxxxxxxxxxxxxx");
        return FALSE;
    }

    return TRUE;
}

void goku_gps_poweroff_pdp_active()
{
	pthread_t threadId;
	pthread_attr_t attr = {0};	
    struct sched_param sp = {0};

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1*1024);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr,SCHED_RR);

    pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 30;
    pthread_attr_setschedparam(&attr, &sp);  
    
	pthread_create(&threadId,&attr,goku_gps_poweroff_pdp_active_thread_proc,NULL);
	pthread_attr_destroy(&attr);
}

void* goku_gps_poweroff_pdp_active_thread_proc()
{
    gint i = 0;
#ifndef WIN32        
        goku_gps_psm_request(DD_LOG_GPS_UPDATE);
#endif
    PRINT_FUNC_LINE("start pdp");
    // [UNIPRO-pengzhaoyang-2013-11-19] for gps 关闭 该线程pdp不进行尝试 
    while( i < 1 ){
        if(goku_gps_open_pdp()){
            break;
        }
        i++;
        tp_os_thread_sleep(100);
    }
     PRINT_FUNC_LINE("stop pdp");
#ifndef WIN32        
        goku_gps_psm_release(DD_LOG_GPS_UPDATE);
#endif
}
//专门创建一个线程来发送消息
VOID goku_gps_senddata_thread_proc(VOID)
{
    GPS_OP_PACKET*  pMsg = NULLPTR;
    guint index = 0;

    
    PRINT_FUNC_LINE("=================enter==============");
    while(1)
    {
        PRINT_FUNC_LINE("Queue Recv waitting...");
        if(OS_FAILURE == tp_os_queue_receive(MsgHandleGPSQueueID,(void**)&pMsg,OS_WAIT_FOREVER))
        {
            PRINT_FUNC_LINE("xxxxxxxxx QUEUE ERROR! xxxxxxxxxxxxxx");
            for(index =0 ;index < SET_SERVER_MAX ;index++)
                {cmdnum_in_queue[index] = 0;}
                
            continue;
        }
        cmdnum_in_queue[pMsg->opType]--;
        PRINT_FUNC_LINE("cmdnum_in_queue[%d] = %d",pMsg->opType,cmdnum_in_queue[pMsg->opType]);

        goku_gps_mutex_get(g_close_mutex,OS_WAIT_FOREVER);
#ifndef WIN32        
        goku_gps_psm_request(DD_LOG_GPS_UPDATE);
#endif
        PRINT_FUNC_LINE("queue deal before goku_gps_buffer_get_count = %d",goku_gps_buffer_get_count());
        goku_gps_queue_deal(pMsg);
        PRINT_FUNC_LINE("queue deal after goku_gps_buffer_get_count = %d",goku_gps_buffer_get_count());        
        if(0 != pMsg->len){
            GPS_FREE(pMsg->buf);
        }
        GPS_FREE(pMsg);
        goku_gps_mutex_put(g_close_mutex);
#ifndef WIN32        
        goku_gps_psm_release(DD_LOG_GPS_UPDATE);
#endif
    }
}
gboolean goku_gps_env_init()
{
    PRINT_FUNC_LINE("============enter=================");
    
    if(
#ifndef WIN32
        -1 == unipro_man_gps_open()||
#endif
        -1 == goku_gps_loc_get_msg_register() ||
        FALSE == goku_gps_buffer_init() ||
        FALSE == goku_gps_queue_create() ||
        FALSE == goku_gps_senddata_thread_create()||
        FALSE == goku_gps_up_point_timer_create()||
        FALSE == goku_gps_up_point_retry_timer_create()||
        FALSE == goku_gps_tm_sync_timer_create()||
        FALSE == goku_gps_force_upload_timer_create()||
        FALSE == goku_gps_poweroff_upload_timer_create()||
        FALSE == goku_gps_beat_timer_create() ){
        PRINT_FUNC_LINE("env init failed");
        return FALSE;
    }

    g_pdp_sema = tp_os_sema_create("pdp_active_sema",0);
    g_gps_close_sema = tp_os_sema_create("gps_close_sema",0);
    
    g_close_mutex = tp_os_mutex_create("gps_close_mutex",OS_TRUE);

    tp_os_mem_set(g_imei_array,0x00,sizeof(g_imei_array));
    goku_gps_get_imei(&g_imei_array);
    
    goku_gps_data_reset();

//    goku_register_message_hook(goku_gps_entry_hook, NULL);
    if(0 < goku_gps_buffer_get_count()){
        goku_gps_set_file_load_flag(TRUE);
    }
    return TRUE;
}
gboolean goku_gps_pdp_active()
{
    return unipro_pdp_active(ENUM_PDP_TYPE_GPS);
}
//时间同步timer
gboolean goku_gps_tm_sync_timer_create()
{
    if(uTmSyncTimerId > 0){
        PRINT_FUNC_LINE("tm_sync timer has Created");
        return TRUE;
    }
    uTmSyncTimerId = goku_gps_timer_create(GOKU_GPS_TM_SYNC_TIMER_NAME,
                                      goku_gps_tm_sync_timer_handler,
                                      NULL,
                                      GOKU_GPS_TM_SYNC_TIMER_LEN,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    if(OS_FAILURE == uTmSyncTimerId){
        PRINT_FUNC_LINE("tm_sync timer Create Failed");
        return FALSE;
    }
    return TRUE;
}
//心跳timer
gboolean goku_gps_beat_timer_create()
{
    if(uBeatTimerId > 0){
        PRINT_FUNC_LINE("Beat timer has Created");
        return TRUE;
    }
    uBeatTimerId = goku_gps_timer_create(GOKU_GPS_BEAT_TIMER_NAME,
                                      goku_gps_beat_timer_handler,
                                      NULL,
                                      GOKU_GPS_BEAT_TIMER_LEN,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    if(OS_FAILURE == uBeatTimerId){
        PRINT_FUNC_LINE("Beat timer Create Failed");
        return FALSE;
    }
    return TRUE;
}
gboolean goku_gps_queue_create()
{
    PRINT_FUNC_LINE("=================enter========================");
    g_return_val_if_fail(OS_QUEUE_INVALID_ID == MsgHandleGPSQueueID, TRUE);//当timer创建过一次后就不再创建了

    MsgHandleGPSQueueID = tp_os_queue_create(GOKU_GPS_QUEUE_NAME,
                                            sizeof(GPS_OP_PACKET),
                                            GOKU_GPS_QUEUE_MAX_LEN);
    if(OS_QUEUE_INVALID_ID == MsgHandleGPSQueueID)
    {
        PRINT_FUNC_LINE("GPS Queue Create Failed");
        return FALSE;
    }
    PRINT_FUNC_LINE("=================end========================");
    return TRUE;
}
//取点timer
gboolean goku_gps_up_point_timer_create()
{
    if(uUpPntTimerId > 0){
        PRINT_FUNC_LINE("upPnt timer has Created");
        return TRUE;
    }
    uUpPntTimerId = goku_gps_timer_create(GOKU_GPS_UP_PNT_TIMER_NAME,
                                      goku_gps_up_pnt_timer_handler,
                                      NULL,
                                      GOKU_GPS_UP_PNT_TIMER_LEN,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    if(OS_FAILURE == uUpPntTimerId){
        PRINT_FUNC_LINE("upPnt timer Create Failed");
        return FALSE;
    }
    return TRUE;
}
//上报timer
gboolean goku_gps_up_point_retry_timer_create()
{
    if(uUpPntRetyTimerId > 0){
        PRINT_FUNC_LINE("up_point_retry timer has Created");
        return TRUE;
    }
    uUpPntRetyTimerId = goku_gps_timer_create(GOKU_GPS_UP_PNT_RETRY_TIMER_NAME,
                                      goku_gps_up_pnt_retry_timer_handler,
                                      NULL,
                                      GOKU_GPS_UP_PNT_RETRY_TIMER_LEN,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    if(OS_FAILURE == uUpPntRetyTimerId){
        PRINT_FUNC_LINE("up_point_retry timer Create Failed");
        return FALSE;
    }
    return TRUE;
}
//强制上报timer
gboolean goku_gps_force_upload_timer_create()
{
    if(uForceUploadTimerId > 0){
        PRINT_FUNC_LINE("uForceUploadTimerId timer has Created");
        return TRUE;
    }
    uForceUploadTimerId = goku_gps_timer_create(GOKU_GPS_FORCE_UPLOAD_TIMER_NAME,
                                      goku_gps_force_upload_timer_handler,
                                      NULL,
                                      GOKU_GPS_FORCE_UPLOAD_TIMER_LEN,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    if(OS_FAILURE == uForceUploadTimerId){
        PRINT_FUNC_LINE("uForceUploadTimerId timer Create Failed");
        return FALSE;
    }
    return TRUE;
}
//关机上报timer
gboolean goku_gps_poweroff_upload_timer_create()
{
    if(uPoweroffUploadTimerId > 0){
        PRINT_FUNC_LINE("uPoweroffUploadTimerId timer has Created");
        return TRUE;
    }
    uPoweroffUploadTimerId = goku_gps_timer_create(GOKU_GPS_POWEROFF_UPLOAD_TIMER_NAME,
                                      goku_gps_poweroff_upload_timer_handler,
                                      NULL,
                                      GOKU_GPS_POWER_TIMEOUT_TIME,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    if(OS_FAILURE == uPoweroffUploadTimerId){
        PRINT_FUNC_LINE("uPoweroffUploadTimerId timer Create Failed");
        return FALSE;
    }
    return TRUE;
}

#pragma arm section rodata="ram_load",  code="ram_load"
VOID goku_gps_tm_sync_timer_handler()
{
    tp_comm_channel_post_msg(NULL, TPM_GPS_MSG_SEND, &uTmSyncTimerId, sizeof(UINT32));
}
#pragma arm section rodata, code

#pragma arm section rodata="ram_load",  code="ram_load"
VOID goku_gps_beat_timer_handler()
{
    tp_comm_channel_post_msg(NULL, TPM_GPS_MSG_SEND, &uBeatTimerId, sizeof(UINT32));
}
#pragma arm section rodata, code


#pragma arm section rodata="ram_load",  code="ram_load"
VOID goku_gps_up_pnt_timer_handler()
{
    tp_comm_channel_post_msg(NULL, TPM_GPS_MSG_SEND, &uUpPntTimerId, sizeof(UINT32));   
}
#pragma arm section rodata, code

#pragma arm section rodata="ram_load",  code="ram_load"
VOID goku_gps_up_pnt_retry_timer_handler()
{ 
    tp_comm_channel_post_msg(NULL, TPM_GPS_MSG_SEND, &uUpPntRetyTimerId, sizeof(UINT32));   
}
#pragma arm section rodata, code

#pragma arm section rodata="ram_load",  code="ram_load"
VOID goku_gps_force_upload_timer_handler()
{
    tp_comm_channel_post_msg(NULL, TPM_GPS_MSG_SEND, &uForceUploadTimerId, sizeof(UINT32));   
}
#pragma arm section rodata, code

#pragma arm section rodata="ram_load",  code="ram_load"
VOID goku_gps_poweroff_upload_timer_handler()
{
    tp_comm_channel_post_msg(NULL, TPM_GPS_MSG_SEND, &uPoweroffUploadTimerId, sizeof(UINT32));   
}
#pragma arm section rodata, code

static SINT32 goku_gps_loc_get_msg_register()
{
	int result = 0;
	PRINT_FUNC_LINE("=================enter========================");
	result |= tp_comm_channel_create(GOKU_GPS_LOC_GET_CHANNEL_NAME, (COMM_MSG_PROC)goku_gps_loc_get_msg_proc, NULL);
	/* register platform message */
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_FIX_OK);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_FIX_FAIL);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_PDP_ACTIVATE_RESULT);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_PDP_DEACTIVATE_INDICATION);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_ACTIVATE_RESULT_SUCCESS);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_ACTIVATE_RESULT_FAIL);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_DEACTIVATE_CONFORM);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_DEACTIVATE_INDICATION);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_ACTIVATE_ABORT);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_PS_PAUSE);
	result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_PS_RESUME);
    result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_MSG_SEND);
    result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_CHECK_FAIL);
    result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_CHECK_OK);
    result |= tp_comm_channel_reg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_CLOSE_GPS);
    
	
	return result;
}

static SINT32 goku_gps_loc_get_msg_unregister()
{
	int result = 0;
	PRINT_FUNC_LINE("=================enter========================");
	
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_FIX_OK);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_FIX_FAIL);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_PDP_ACTIVATE_RESULT);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_PDP_DEACTIVATE_INDICATION);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_ACTIVATE_RESULT_SUCCESS);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_ACTIVATE_RESULT_FAIL);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_DEACTIVATE_CONFORM);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_DEACTIVATE_INDICATION);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_ACTIVATE_ABORT);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_PS_PAUSE);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, UNIPRO_PDP_PS_RESUME);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_MSG_SEND);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_CHECK_FAIL);
	result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_CHECK_OK);
    result |= tp_comm_channel_unreg_msg(GOKU_GPS_LOC_GET_CHANNEL_NAME, TPM_GPS_CLOSE_GPS);	
	result |= tp_comm_channel_destroy(GOKU_GPS_LOC_GET_CHANNEL_NAME);
	return result;
}

static gboolean goku_gps_pdp_check(CONST VOID* pData, guint32 nDataSize)
{
    UINT8   nCurID;
    UINT32  nID;

    nID = *((UINT32*)pData);
    nCurID  = unipro_pdp_get_cid(ENUM_PDP_TYPE_GPS);
    
    PRINT_FUNC_LINE("nCurID=%d,nID=%d",nCurID,nID);
    if( nCurID == 0 || nCurID != nID )
    {
        return  FALSE;
    }
    
    return  TRUE;
}

static void goku_gps_pdp_activate_result(gboolean bSuccess,CONST VOID* pData, guint32 nDataSize)
{
    if( !goku_gps_pdp_check(pData,nDataSize) )
    {
        return;
    }
    
    goku_gps_sema_put(g_pdp_sema);
}

static void goku_gps_pdp_deactivate_result(gboolean bNetwork,CONST VOID* pData, guint32 nDataSize)
{
    if( !goku_gps_pdp_check(pData,nDataSize) )
    {
        return;
    }

    if( !bNetwork )
    {
        return;
    }

    unipro_socket_close(ENUM_SOCKET_TYPE_GPS);
    goku_gps_pdp_deactive();    
}

static void goku_gps_pdp_ps_changed(gboolean bPause,CONST VOID* pData, guint32 nDataSize)
{
    if( !bPause )
    {
        return;
    }
    //need to do
}

static gint32 goku_gps_loc_get_msg_proc(guint32 msg_id, void* param, guint32 msg_size, VOID *add_data)
{
	gint32 result = 0;
    GPSInfo  gps_info_buff = {0};
    char t_cmdstr[GPS_PROTOCOL_UPLOAD_MAX] = {0};
    
    PRINT_FUNC_LINE("msg_id = %d",msg_id);	
	switch (msg_id) 
	{    
#if 1
    case UNIPRO_PDP_ACTIVATE_RESULT_SUCCESS:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_ACTIVATE_RESULT_SUCCESS");
        goku_gps_pdp_activate_result(TRUE,param,msg_size);
        break;
    }
    case UNIPRO_PDP_ACTIVATE_RESULT_FAIL:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_ACTIVATE_RESULT_FAIL");
        goku_gps_pdp_activate_result(FALSE,param,msg_size);
        break;
    }
    case UNIPRO_PDP_DEACTIVATE_CONFORM:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_DEACTIVATE_CONFORM");
        goku_gps_pdp_deactivate_result(FALSE,param,msg_size);
        break;
    }
    case UNIPRO_PDP_DEACTIVATE_INDICATION:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_DEACTIVATE_INDICATION");
        goku_gps_pdp_deactivate_result(TRUE,param,msg_size);
        break;
    }
    case UNIPRO_PDP_ACTIVATE_ABORT:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_ACTIVATE_ABORT");
        break;
    }
    case UNIPRO_PDP_PS_PAUSE:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_PS_PAUSE");
        goku_gps_pdp_ps_changed(TRUE,param,msg_size);
        break;
    }
    case UNIPRO_PDP_PS_RESUME:
    {
        PRINT_FUNC_LINE("UNIPRO_PDP_PS_RESUME");
        goku_gps_pdp_ps_changed(FALSE,param,msg_size);
        break;
    }
#endif
    case TPM_GPS_FIX_OK:
  	case TPM_GPS_FIX_FAIL:
    	{
    	    if(  msg_id == TPM_GPS_FIX_FAIL && 
    	        g_gps_loc_mode == FIX_ONE_TIME){
    	        PRINT_FUNC_LINE("Abort This Point");
                break;
    	    }
#ifndef WIN32
        	goku_gps_psm_request(DD_LOG_GPS_UPDATE);
#endif      
            if(msg_id == TPM_GPS_FIX_OK){
                PRINT_FUNC_LINE("msg_id = %s","TPM_GPS_FIX_OK");	
                tp_os_mem_cpy(&gps_info_buff,(GPSInfo*)param,msg_size);
            }else{
                tp_os_mem_set(&gps_info_buff,0x00,sizeof(gps_info_buff));
                PRINT_FUNC_LINE("msg_id = %s","TPM_GPS_FIX_FAIL");
            }
            PRINT_FUNC_LINE("gps_info_buff.Lat = %f,gps_info_buff.Lon = %f",gps_info_buff.Lat,gps_info_buff.Lon);
            goku_gps_pac_point((char*)&gps_info_buff,t_cmdstr,GPS_UPLOADCMD_LEN);

            if(goku_gps_get_gps_close_flg()){
                tp_os_mem_set(g_sendcmdstr,0x00,sizeof(g_sendcmdstr));
                g_sendcmdstr_len = GPS_UPLOADCMD_LEN;
                
                tp_os_mem_cpy(g_sendcmdstr,t_cmdstr,GPS_UPLOADCMD_LEN);
                goku_gps_set_last_point_flg(TRUE);
            }else{
                goku_gps_buffer_write((UINT8*)t_cmdstr,GPS_UPLOADCMD_LEN);
            }

            PRINT_FUNC_LINE("TPM_GPS_FIX Write buffer");
            g_gps_has_point = TRUE;
#ifndef WIN32
        	goku_gps_psm_release(DD_LOG_GPS_UPDATE);
#endif
           	break;
    	}
#if 0
    case TPM_PDP_ACTIVATE_RESULT://pdp激活结果
        {
            SC_PDP_CID_INFO_ST*  re = (SC_PDP_CID_INFO_ST*)param;
            gboolean bRet = FALSE;
            UINT8 cur_cid = unipro_pdp_get_cid(ENUM_PDP_TYPE_GPS);
            PRINT_FUNC_LINE("msg_id = %s","TPM_PDP_ACTIVATE_RESULT");
            
            PRINT_FUNC_LINE("pdp RESULT cid = %d",re->cid_send_to_app);
            PRINT_FUNC_LINE("GPS cid = %d",cur_cid);
            if ( cur_cid != 0 ){
                if(cur_cid == re->cid_send_to_app){
                    PRINT_FUNC_LINE("PDP Active Success.");
                    goku_gps_sema_put(g_pdp_sema);
                }else if((0 == re->cid_send_to_app)&&
                        (cur_cid == re->fail_cid)){
                    PRINT_FUNC_LINE("PDP Active Fail.");
                    goku_gps_sema_put(g_pdp_sema);
                }
            }
            break;
        }
    case TPM_PDP_DEACTIVATE_INDICATION:
        {
            SC_PDP_CID_INFO_ST*  re = (SC_PDP_CID_INFO_ST*)param;
            UINT8 cur_cid = unipro_pdp_get_cid(ENUM_PDP_TYPE_GPS);        

            if ( cur_cid != 0 ){
                if(cur_cid == re->cid_send_to_app){
                    PRINT_FUNC_LINE("NetWork Side Deactive Location Need Deactive also. ");
                    unipro_socket_close(ENUM_SOCKET_TYPE_GPS);
                    goku_gps_pdp_deactive();
                }
            }
            break;
        }
#endif
    case TPM_GPS_MSG_SEND:
        {
            UINT32  timer_id = *(UINT32*)param;

#ifndef WIN32
            goku_gps_psm_request(DD_LOG_GPS_UPDATE);
#endif    
            PRINT_FUNC_LINE("g_msg_mutex before");
//            tp_os_mutex_get(g_msg_mutex,OS_WAIT_FOREVER);
            
            PRINT_FUNC_LINE("msg_id = %s","TPM_GPS_MSG_SEND");
            
            if(timer_id == uTmSyncTimerId){
                PRINT_FUNC_LINE("Start Time sync!");
                gps_msg_handle_notice(GET_TIME_SYNC_REQ,NULL,0);
            }else if(timer_id == uUpPntTimerId){
                PRINT_FUNC_LINE("UpPntTimer rcv_start");
                //if gps closing, we needn't start get-point timer,and gps chip again.
                if(!goku_gps_get_gps_close_flg()){
                    goku_gps_timer_restart(uUpPntTimerId,goku_gps_get_cfg_interval()*1000);
                    unipro_man_gps_rcv_start(FIX_ONE_TIME,1,0);//应该再下层加入唤醒
                }
                
                PRINT_FUNC_LINE("g_gps_has_point = %d",g_gps_has_point);
                if(!g_gps_has_point){
                    PRINT_FUNC_LINE("Timeout Write a Point.");
                    tp_os_mem_set(&gps_info_buff,0x00,sizeof(gps_info_buff));
                    goku_gps_pac_point((char*)&gps_info_buff,t_cmdstr,GPS_UPLOADCMD_LEN);
                    
                    if(goku_gps_get_gps_close_flg()){
                        tp_os_mem_set(g_sendcmdstr,0x00,sizeof(g_sendcmdstr));
                        g_sendcmdstr_len = GPS_UPLOADCMD_LEN;
                        
                        tp_os_mem_cpy(g_sendcmdstr,t_cmdstr,GPS_UPLOADCMD_LEN);
                        goku_gps_set_last_point_flg(TRUE);
                    }else{
                        goku_gps_buffer_write((UINT8*)t_cmdstr,GPS_UPLOADCMD_LEN);
                    }
                    
                    PRINT_FUNC_LINE("TPM_GPS_MSG_SEND Write buffer");
                }
                
                //writen point success,reset the flag.
                g_gps_has_point = FALSE;
            }else if(timer_id == uBeatTimerId){
                PRINT_FUNC_LINE("Start HeartBeat Sync!");
                gps_msg_handle_notice(SET_LOCLA_BEAT_PACK,NULL,0);
            }else if(timer_id == uUpPntRetyTimerId){
            
                PRINT_FUNC_LINE("Start Record Point");
                if(goku_gps_get_gps_close_flg()){
                    gps_msg_handle_notice(SET_LOCAL_UPLOAD_SINGLE_POINT,NULL,0);
                }else{
                    goku_gps_start_retry_upload_timer(goku_gps_get_retry_upload_timer_len(),0);
                    gps_msg_handle_notice(SET_LOCAL_UPLOAD,NULL,0);
                }
                
            }else if(timer_id == uForceUploadTimerId){
                if(goku_poc_get_status() != 0){
                    g_bForceUploadFlg = TRUE;
                }
                PRINT_FUNC_LINE("Start Force Upload!");
                gps_msg_handle_notice(SET_LOCAL_UPLOAD,NULL,0);
            }else if(timer_id == uPoweroffUploadTimerId){
                    goku_gps_sema_put(g_gps_close_sema);
            
//                if(!g_bPressPowerFlg){
//                    goku_gps_sema_put(g_gps_close_sema);
//                    PRINT_FUNC_LINE("=============== Close GPS ======================");
//                }else if(g_bPressPowerFlg){
//                    //清除数据    
//                    goku_gps_buffer_free();
//                    
//                    //关机timer 超时
//                    PRINT_FUNC_LINE("============== start shutdown ===================");
//                    goku_power_off(GOKU_POWER_OFF_SHUT_DOWN);
//                }
            }

//            tp_os_mutex_put(g_msg_mutex);
            PRINT_FUNC_LINE("g_msg_mutex after");
            goku_gps_psm_release(DD_LOG_GPS_UPDATE);
            
            break;
        }
    case TPM_GPS_CHECK_FAIL:
    case TPM_GPS_CHECK_OK:
        {
        
           if(msg_id == TPM_GPS_CHECK_FAIL){
                GokuNotificationClient * notification_service = goku_notification_client_new();
                PRINT_FUNC_LINE("msg_id = %s","TPM_GPS_CHECK_FAIL");
                g_sChipOK = 0;
                
                #ifndef WIN32
                unipro_man_gps_close();
                #endif
                
                //关闭消息
                goku_gps_timers_delete();
                //关闭线程    
                goku_gps_thread_delete();
                //关闭状态    
                unipro_socket_close(ENUM_SOCKET_TYPE_GPS);
                goku_gps_pdp_deactive();

                goku_gps_set_running_status(FALSE);
//                goku_deregister_message_hook(goku_gps_entry_hook);
                goku_notification_client_cancel_icon(notification_service, "GOKU_GPS_NOTIFICATION_ICON_NAME");
                PRINT_FUNC_LINE("================Chip Check FAIL!==================");
           }else{
                PRINT_FUNC_LINE("msg_id = %s","TPM_GPS_CHECK_OK");
                g_sChipOK = 1;
           }
        }
        break;
    case TPM_GPS_CLOSE_GPS:
        {
            goku_gps_upload_last_point();    
        }
        break;
	default:
	    PRINT_FUNC_LINE("msg_id = %s","WRONG");
		break;
	}
    
	return result;
}

gboolean gps_msg_handle_notice(GPS_CMD_CMD_E cmd, VOID *buf, gint len)
{
    GPS_OP_PACKET *pPara = NULL;

    if( (SET_SERVER_NULL >= cmd) && 
        (SET_SERVER_MAX <= cmd)){
        PRINT_FUNC_LINE("cmd ERROR!");
        return FALSE;
    }
    
    if( MsgHandleGPSQueueID == OS_QUEUE_INVALID_ID){
        PRINT_FUNC_LINE("MsgHandleGPSQueueID ERROR!");
        return FALSE;
    }
    
    if( cmdnum_in_queue[cmd] ){
        PRINT_FUNC_LINE("the queue has a same cmd!");
        return FALSE;
    }

    pPara = (GPS_OP_PACKET *)tp_os_mem_malloc(sizeof(GPS_OP_PACKET));
    if (NULL == pPara){
        PRINT_FUNC_LINE("Memory Not enough!");
        return FALSE;
    }
    tp_os_mem_set(pPara,0x00,sizeof(GPS_OP_PACKET));

    pPara->opType = cmd;
    pPara->buf = NULL;
    pPara->len = len;

    if(0 != pPara->len){
        pPara->buf = (void*)tp_os_mem_malloc(pPara->len+1);
        tp_os_mem_set(pPara->buf,0x00,pPara->len+1);
        tp_os_mem_cpy((void *)pPara->buf, (void *)buf, pPara->len);
    }
    PRINT_FUNC_LINE("tp_os_queue_send->start");
    PRINT_FUNC_LINE("cmdnum_in_queue[%d] = %d",cmd,cmdnum_in_queue[cmd]);
    cmdnum_in_queue[cmd]++;
    tp_os_queue_send(MsgHandleGPSQueueID, (void*)pPara, OS_NO_WAIT);
    return TRUE;
}

gboolean goku_gps_open_network()
{
    PRINT_FUNC_LINE("============== enter ===================");

    PRINT_FUNC_LINE("g_bForceUploadFlg = %d",g_bForceUploadFlg);
    if(!g_bForceUploadFlg && !goku_gps_get_gps_close_flg()){//强制上传标志 添加关闭GPS时POC在通话而无法上传最后一个点的情况
        if( goku_poc_get_status() != 0 ){//when poc status is not idle,must not to update gps
            return FALSE;
        }
    }

    if(!goku_gps_open_pdp()){
        return FALSE;
    }
    if(!unipro_socket_open(ENUM_SOCKET_TYPE_GPS,GOKU_GPS_SERVICE_HOST,GOKU_GPS_SERVICE_PORT)){
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxx socket open Fail xxxxxxxxxxxxxxx");
        unipro_socket_close(ENUM_SOCKET_TYPE_GPS);
        return FALSE;
    } 
    PRINT_FUNC_LINE("^^^^^^Network Open Sucess^^^^^^^");

    return TRUE;
}

void goku_gps_pdp_deactive()
{
    PRINT_FUNC_LINE("=================enter==============");
    
    if(!unipro_pdp_deactive(ENUM_PDP_TYPE_GPS))
    {
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxxxx PDP Deactive Faile xxxxxxxxxxxxxxxxx");
    }
}

void goku_gps_close_network()
{
    PRINT_FUNC_LINE("============== enter ===================");
    unipro_socket_close(ENUM_SOCKET_TYPE_GPS);
    //GPS关闭时,为发送数据则不关闭PDP。当关机时不激活pdp,当上传间隔时间*总上传时间<15s时PDP不关闭
    if( !goku_gps_get_gps_close_flg() && goku_gps_get_retry_upload_timer_len() >= GOKU_GPS_PDP_DEACTIVE_TIME){
        goku_gps_pdp_deactive();
    }
}

gboolean goku_net_query()
{
	NET_STATUS_INFO net_info = {0};
	gboolean bRet = FALSE;
	
    PRINT_FUNC_LINE("=================enter==============");

    tp_net_query_status(&net_info);

    //      0 : 未注册, 并不再搜索运营商
    //      2 : 未注册, 正在搜索运营商
    //  	3 : 注册被拒绝, 仅允许紧急呼叫
	if((0== net_info.reg_stat)
    	|| (2== net_info.reg_stat)
    	|| ( 3 == net_info.reg_stat))
	{
	    PRINT_FUNC_LINE("xxxxxxxxx NetWork ERROR xxxxxxxxxx");
	    return FALSE;
	}
	return TRUE;
}

BOOL goku_gps_sim_check()
{
	NET_STATUS_INFO net_info = {0};
	gboolean bRet = FALSE;
	
    PRINT_FUNC_LINE("=================enter==============");

    tp_net_query_status(&net_info);

    if(0 == net_info.sim_stat)
    {
        PRINT_FUNC_LINE("NO SIM CARD!");
        return FALSE;
    }
    
    return TRUE;
}

gboolean goku_gps_msg_send_ex(GPS_OP_PACKET* pMsg)
{
    gboolean bSendSucess = FALSE;
    gchar* pPackeddata = NULL;
    guint  uPackeddataLen = 0;
    gchar chTemp[GPS_SOKET_MAX_LEN] = {0};
    
    if(!goku_gps_open_network())
    {
        PRINT_FUNC_LINE("xxxxxxxxxxxxx Network Open Fail! xxxxxxxxxxxxxx");
        return FALSE;
    }
    
    goku_gps_get_packed_data(pMsg,NULL,chTemp,&uPackeddataLen);
    if(0 == uPackeddataLen)
    {
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxx packed data parm error! xxxxxxxxxxxxxxxxxx");
        goku_gps_close_network();
        return  FALSE;
    }
    
    bSendSucess = goku_gps_send_data(pMsg,chTemp,uPackeddataLen);
    goku_gps_close_network();
    return bSendSucess;
}

gboolean goku_gps_msg_send(GPS_OP_PACKET* pMsg)
{
    gint retry_times = 0;
    gboolean bRet = FALSE;

    PRINT_FUNC_LINE("================== enter ========================");
    for(retry_times = 0;retry_times < GOKU_GPS_SEND_RETRY_CNT ;retry_times++ ){
        if(goku_gps_msg_send_ex(pMsg)){
           PRINT_FUNC_LINE("^^^^^^^^^^^^ Msg Send&Recv Sucess ^^^^^^^^^^^^^^^");
           bRet = TRUE;
           break;
        }

        if(goku_gps_get_gps_close_flg()){
            // [UNIPRO-pengzhaoyang-2013-11-19] for gps关闭 不进行尝试 减少关闭时间 
            PRINT_FUNC_LINE("GPS shutingdown break");
            break;
        }
        tp_os_thread_sleep(1000);
        PRINT_FUNC_LINE("xxxxxxxxxxxxxx goku_gps_msg_send_ex retry %d xxxxxxxxxxxxxxx",retry_times);
    }

    return bRet;
}

void goku_gps_sema_delete()
{
    PRINT_FUNC_LINE("============= enter =================");
    if(OS_SEMA_INVALID_ID != g_pdp_sema){
        tp_os_sema_delete(g_pdp_sema);
    }
    if(OS_SEMA_INVALID_ID != g_gps_close_sema){
        tp_os_sema_delete(g_gps_close_sema);
    }

    g_pdp_sema = OS_SEMA_INVALID_ID;
    g_gps_close_sema = OS_SEMA_INVALID_ID;
}

void goku_gps_mutex_delete()
{
    PRINT_FUNC_LINE("============= enter =================");
    if(OS_MUTEX_INVALID_ID != g_close_mutex){
        tp_os_mutex_get(g_close_mutex,OS_WAIT_FOREVER);
        tp_os_mutex_delete(g_close_mutex);
    }
//    if(OS_MUTEX_INVALID_ID != g_msg_mutex){
//        tp_os_mutex_get(g_msg_mutex,OS_WAIT_FOREVER);
//        tp_os_mutex_delete(g_msg_mutex);
//    }
    if(OS_MUTEX_INVALID_ID != g_close_mutex){
        tp_os_mutex_get(g_close_mutex,OS_WAIT_FOREVER);
        tp_os_mutex_delete(g_close_mutex);
    }
    g_close_mutex = OS_MUTEX_INVALID_ID;
//    g_msg_mutex = OS_MUTEX_INVALID_ID;
    g_close_mutex = OS_MUTEX_INVALID_ID;
}

void goku_gps_queue_delete()
{
    g_return_if_fail(OS_QUEUE_INVALID_ID != MsgHandleGPSQueueID);
    tp_os_queue_delete(MsgHandleGPSQueueID);
    MsgHandleGPSQueueID = OS_QUEUE_INVALID_ID;
}

void goku_gps_thread_delete()
{
    PRINT_FUNC_LINE("============= enter =================");
    
    goku_gps_mutex_get(g_close_mutex,OS_WAIT_FOREVER);

    if(OS_THREAD_INVALID_ID != MsgHandleSendDataThreadID){
        tp_os_thread_delete(MsgHandleSendDataThreadID);
    }
    MsgHandleSendDataThreadID = OS_THREAD_INVALID_ID;
    
    goku_gps_mutex_put(g_close_mutex);
}

void goku_gps_timers_delete()
{
    PRINT_FUNC_LINE("============= enter =================");
    goku_gps_timer_delete(&uUpPntTimerId);
    goku_gps_timer_delete(&uBeatTimerId);
    goku_gps_timer_delete(&uTmSyncTimerId);
    goku_gps_timer_delete(&uUpPntRetyTimerId);
    goku_gps_timer_delete(&uForceUploadTimerId);
    goku_gps_timer_delete(&uPoweroffUploadTimerId);
}

void goku_gps_data_reset()
{    
    goku_gps_protocol_data_reset();
    tp_os_mem_set(g_sendcmdstr,0x00,sizeof(g_sendcmdstr));
    tp_os_mem_set(cmdnum_in_queue,0x00,sizeof(cmdnum_in_queue));//Init the array ,if not,maybe  !0 data cause send message.
    
    goku_gps_set_retry_upload_timer_len(0,1);

    goku_gps_flow_reset();
    
    goku_gps_set_gps_close_flg(FALSE);
    goku_gps_set_last_point_flg(FALSE);
    goku_gps_set_running_status(FALSE);
    
    g_bFirstRuning = TRUE;
    g_bFirstBeatMsg = TRUE;
    g_bFirstStartDriver  = TRUE;
    g_bForceUploadFlg = FALSE;
    g_bFastUploadLastPoint = FALSE;
    g_ForceUpload_timer_len = GOKU_GPS_FORCE_UPLOAD_TIMER_LEN;
    g_gps_has_point = FALSE;
    g_uSyncRetryCnt = 0;
    g_uBeatRetryCnt = 0;
    goku_gps_set_file_load_flag(FALSE);
}

void goku_gps_set_mode(UINT32 interval,UINT32 maxN)
{      
   PRINT_FUNC_LINE("=================== enter ============================");

    if(0 != tp_fexist(GPS_SETTING) ||  g_sChipOK ==0 || !goku_gps_sim_check()){
        PRINT_FUNC_LINE("GPS not Running or No Chip or No Sim");
        return;
    }

    PRINT_FUNC_LINE("interval = %d,maxN = %d",interval,maxN);
    if(interval < 2 || maxN <1 || maxN > 200){
        PRINT_FUNC_LINE("interval need >= 2s,1<= maxN need <=200,");
        return;
   }

   if(  !g_bFirstStartDriver && 
        (goku_gps_get_cfg_maxN() == maxN && goku_gps_get_cfg_interval() == interval)){
        PRINT_FUNC_LINE("Parm Not change!");
        return;
   }

   goku_gps_set_cfg_interval(interval);
   goku_gps_set_cfg_maxN(maxN);
   goku_gps_set_retry_upload_timer_len(goku_gps_get_cfg_interval(),
        goku_gps_get_cfg_maxN());

   goku_gps_start_gps_chip(goku_gps_reset_timer(interval),goku_gps_get_cfg_interval());
   
   
   goku_gps_start_retry_upload_timer(goku_gps_get_retry_upload_timer_len(),GOKU_GPS_DELAY_TIME);//改为延时500ms而不是1s
   
   PRINT_FUNC_LINE("interval = %d,maxN = %d,g_gps_loc_mode = %d",
        interval,maxN,g_gps_loc_mode);
        
}
void goku_gps_get_gps_parm(gps_fix_mode *mod,guint *up_tm_len)
{
    PRINT_FUNC_LINE("======================== enter =================================");
    PRINT_FUNC_LINE("interval = %d,maxN = %d,mode = %d",goku_gps_get_cfg_interval(),goku_gps_get_cfg_maxN(),g_gps_loc_mode);
    *up_tm_len = goku_gps_get_cfg_interval();
    *mod = g_gps_loc_mode;
}

void goku_gps_get_packed_data(GPS_OP_PACKET* op, gchar* cmdparm, 
                                           /*OUT*/ gchar* packeddata, /*OUT*/guint* leng)
{
    PRINT_FUNC_LINE("=================  enter ================================");

    switch(op->opType)
    {
    case GET_TIME_SYNC_REQ:
    case SET_LOCLA_BEAT_PACK:
        {   
            if( op->opType == GET_TIME_SYNC_REQ ){
                PRINT_FUNC_LINE("cmd = %s","GET_TIME_SYNC_REQ");
            }else{
                PRINT_FUNC_LINE("cmd = %s","SET_LOCLA_BEAT_PACK");
            }

            if(goku_gps_pack_req_cmdstr(op->opType,cmdparm,packeddata)){
                *leng = strlen(packeddata);
            }
            break;
        }
    case SET_LOCAL_UPLOAD:
    case SET_LOCAL_UPLOAD_SINGLE_POINT:
        {     
            guint t_cnt = 0;

            PRINT_FUNC_LINE("cmd = %s","SET_LOCAL_UPLOAD");
                        
            if(0x00 == *g_sendcmdstr){
                 t_cnt = goku_gps_buffer_get_count();
                if( t_cnt == 0 ){
                    return;
                }
                 g_sendcmdstr_len = t_cnt*GPS_UPLOADCMD_LEN;
                 if(g_sendcmdstr_len > GPS_SOKET_MAX_LEN){
                    g_sendcmdstr_len = GPS_SOKET_MAX_LEN;
                 }else{
                    g_sendcmdstr_len = g_bFirstRuning ? GPS_UPLOADCMD_LEN : t_cnt*GPS_UPLOADCMD_LEN;
                 }
                 goku_gps_buffer_read((UINT8*)g_sendcmdstr,g_sendcmdstr_len);
            }
            tp_os_mem_cpy(packeddata,g_sendcmdstr,g_sendcmdstr_len);
            *leng =  g_sendcmdstr_len;
            UNIPRO_DEBUG_DUMP(packeddata,g_sendcmdstr_len,"g_sendcmdstr:");
            break;
        }
    default:
        break;
    }
    PRINT_FUNC_LINE("leng = %d, packeddata = %s",*leng,packeddata);
}

gboolean goku_gps_send_data(GPS_OP_PACKET* pMsg,gchar* packeddata,guint leng)
{        
    PRINT_FUNC_LINE("pMsg->opType = %d, len = ,packeddata = %s",pMsg->opType,leng,packeddata);
    
    if(!unipro_socket_send(ENUM_SOCKET_TYPE_GPS,packeddata,leng)){
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxxxxx socket Send Fail... xxxxxxxxxxxxxxxxxxxxxx");
        return FALSE;
    }
    //发送流量统计
    goku_gps_sendbtyes_total(leng);
    
    if(!goku_gps_recv(pMsg->opType)){
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxxxxx socket Recv Fail... xxxxxxxxxxxxxxxxxxxxxx");
        return FALSE;
    }

    return TRUE;
}

gboolean goku_gps_deal_send_data(GPS_OP_PACKET* pMsg,gboolean bSendSucess)
{
    guint  base = 1;
    gboolean tbSuccessORtimeoutFlg = FALSE;
    
    PRINT_FUNC_LINE("pMsg->opType = %d,bSendSucess = %d",pMsg->opType,bSendSucess);

    if (goku_gps_get_gps_close_flg() 
        && pMsg->opType != SET_LOCAL_UPLOAD 
        && pMsg->opType != SET_LOCAL_UPLOAD_SINGLE_POINT){
        PRINT_FUNC_LINE("gps closing  noUpload commands return!");
        return FALSE;
    }
    
    switch(pMsg->opType)
    {
    case GET_TIME_SYNC_REQ:
        {
            guint t_sync_timer_len = 0;

            if(bSendSucess || g_uSyncRetryCnt > GOKU_GPS_SYNC_RETRY_CNT){
                g_uSyncRetryCnt = 0;
                t_sync_timer_len = goku_gps_get_tm_sync_len();
                tbSuccessORtimeoutFlg = TRUE;
                PRINT_FUNC_LINE("Sync Sucess or Timeout ->Start %d(s) timer",t_sync_timer_len);
            }else{
                g_uSyncRetryCnt++;
                t_sync_timer_len = GOKU_GPS_TM_SYNC_TIMER_LEN*(base<<g_uSyncRetryCnt);
            }
            
            PRINT_FUNC_LINE("g_gps_sync_timer_len = %d,syn_intval = %d,g_bFirstBeatMsg = %d",
                t_sync_timer_len,goku_gps_get_tm_sync_len(),g_bFirstBeatMsg);
            goku_gps_timer_restart(uTmSyncTimerId,t_sync_timer_len*1000);
            
            if(tbSuccessORtimeoutFlg && g_bFirstBeatMsg){
                g_bFirstBeatMsg = FALSE;
                PRINT_FUNC_LINE("Time sync just start HeartBeat timer one time");
                gps_msg_handle_notice(SET_LOCLA_BEAT_PACK,NULL,0);
            }
            
            return TRUE;
        }    
    case SET_LOCLA_BEAT_PACK:
        {
            guint  t_beat_timer_len = 0;
            
            if(bSendSucess || g_uBeatRetryCnt > GOKU_GPS_BEAT_RETRY_CNT){
                t_beat_timer_len = goku_gps_get_cfg_beattime();
                g_uBeatRetryCnt = 0;
                tbSuccessORtimeoutFlg = TRUE;
            }else{
                g_uBeatRetryCnt++;
                t_beat_timer_len = GOKU_GPS_ONE_MIN_LEN*(base<<g_uBeatRetryCnt);
            }

            PRINT_FUNC_LINE("t_beat_timer_len = %d,maxN = %d",
                t_beat_timer_len,goku_gps_get_recv_maxN());
            goku_gps_timer_restart(uBeatTimerId,t_beat_timer_len*1000);

            if(tbSuccessORtimeoutFlg){
                goku_gps_set_mode(goku_gps_get_recv_interval(),
                    goku_gps_get_recv_maxN());
                //运行第一个点
                goku_gps_send_first_pnt();

                // [UNIPRO-pengzhaoyang-2013-11-20] for 第一次加载文件成功后发送上报消息 
                if( goku_gps_get_file_load_flag()){
                    goku_gps_set_file_load_flag(FALSE);
                    gps_msg_handle_notice(SET_LOCAL_UPLOAD,NULL,0);
                }
            }
            return TRUE;
            
        }   
    case SET_LOCAL_UPLOAD:
    case SET_LOCAL_UPLOAD_SINGLE_POINT:
        {   
           guint uTtimerLen = 0;
           
           if(bSendSucess){
                goku_gps_stop_force_upload_timer();

                // closeflag & last-point-writen & buffer-empty & press-power poweroff.
                if( goku_gps_get_gps_close_flg() && 
                    goku_gps_get_last_point_flg()){
                    //gps closed,need put the sema.
                    
                        goku_gps_sema_put(g_gps_close_sema);
                    
//                    if( !g_bPressPowerFlg){
//                        goku_gps_sema_put(g_gps_close_sema);
//                    }else{
//                        goku_gps_timer_stop(uPoweroffUploadTimerId);

//                        //清除数据    
//                        goku_gps_buffer_free();
//    
//                        PRINT_FUNC_LINE("POWER Last Point Upload Success!");
//                        goku_power_off(GOKU_POWER_OFF_SHUT_DOWN);
//                    }
                }
                
           }else{
                //closing send fail , retry .
                if( goku_gps_get_gps_close_flg() &&
                    goku_gps_get_last_point_flg() ){
                    PRINT_FUNC_LINE("retry send single point!");
                    tp_os_thread_sleep(100);// [UNIPRO-pengzhaoyang-2013-11-26] for 防止上传失败后，一直尝试时完全占用CPU 
                    gps_msg_handle_notice(SET_LOCAL_UPLOAD_SINGLE_POINT,NULL,0);
                }else if(!goku_gps_is_timer_running(uForceUploadTimerId)){
                    goku_gps_start_force_upload_timer();
                }
           }
           
            if(!bSendSucess){
               PRINT_FUNC_LINE("xxxxxxxxxxxxx Upload Records Fail xxxxxxxxxxxxxxx");
               return FALSE; 
            }
            if(g_bFirstRuning){
                PRINT_FUNC_LINE("Clear First Upload Flag");
                g_bFirstRuning = FALSE;
            }
            //发送成功清空
            tp_os_mem_set(g_sendcmdstr,0x00,sizeof(g_sendcmdstr));
            g_sendcmdstr_len = 0;
            PRINT_FUNC_LINE("^^^^^^^^^^^^^ data clear ^^^^^^^^^^^^^^^^^^");
            return TRUE;
        }
    default:
        PRINT_FUNC_LINE("deal cmd ERROR!");
        return FALSE;
    }

}

gboolean goku_gps_queue_deal(GPS_OP_PACKET* pMsg)
{
    gboolean bRet = FALSE;

    PRINT_FUNC_LINE("==================== enter =========================");
    if( pMsg->opType == SET_LOCAL_UPLOAD &&
        0 == goku_gps_buffer_get_count()){
        PRINT_FUNC_LINE("Buffer Is Empty");
        return TRUE;
    }
    
    do{
        bRet = goku_gps_msg_send(pMsg);        
        goku_gps_deal_send_data(pMsg,bRet);
        if(!bRet)
        {
            PRINT_FUNC_LINE("xxxxxxxxxxxxxx Queue Msg Deal Fail xxxxxxxxxxxxxxxxx");
            break;
        }
        
        //用于判断第一个点上报
        if( g_bFirstRuning &&
            pMsg->opType == SET_LOCAL_UPLOAD){
            g_bFirstRuning = FALSE;
            PRINT_FUNC_LINE("^^^^^^^^^^^^^ First Point Success! ^^^^^^^^^^^^^^");
            break;
        }

        // [UNIPRO-pengzhaoyang-2013-11-22] for 该处是在关闭GPS时正常上报不用一直发送 
        if (goku_gps_get_gps_close_flg() && pMsg->opType == SET_LOCAL_UPLOAD){
            PRINT_FUNC_LINE("gps closing  noUpload commands return!");
            return TRUE;
        }
        tp_os_thread_sleep(1000);
    }while((SET_LOCAL_UPLOAD == pMsg->opType) && 
            0 < goku_gps_buffer_get_count());
    
    return TRUE;
}

void goku_gps_send_first_pnt()
{
    char t_cmdstr[GPS_UPLOADCMD_LEN] = {0};
    GPSInfo  gps_info_buff = {0};//用于存放上报点的buff
    if(g_bFirstRuning){
        PRINT_FUNC_LINE("^^^^^^^^^^^^ GPS First Running ^^^^^^^^^^^^");
        tp_os_mem_set(&gps_info_buff,0x00,sizeof(gps_info_buff));
        goku_gps_pac_point((char*)&gps_info_buff,t_cmdstr,GPS_UPLOADCMD_LEN);
        
        tp_os_mem_set(g_sendcmdstr,0x00,sizeof(g_sendcmdstr));
        g_sendcmdstr_len = GPS_UPLOADCMD_LEN;
        tp_os_mem_cpy(g_sendcmdstr,t_cmdstr,GPS_UPLOADCMD_LEN);
        
        gps_msg_handle_notice(SET_LOCAL_UPLOAD_SINGLE_POINT,NULL,0);
    }
}

void goku_gps_start_force_upload_timer()
{
    PRINT_FUNC_LINE("=================== enter ======================");
    goku_gps_timer_restart(uForceUploadTimerId,
                    goku_gps_get_force_upload_timer_len()*1000);
}

void goku_gps_stop_force_upload_timer()
{
    PRINT_FUNC_LINE("=================== enter ======================");
    if(0 == goku_gps_buffer_get_count()){
        g_bForceUploadFlg = FALSE;
    }
    goku_gps_timer_stop(uForceUploadTimerId);
}
/*
static int goku_gps_entry_hook(HWND dst_wnd, int msg, WPARAM wparam, LPARAM lparam, gpointer add_data)
{
    
    if (MSG_KEYLONGPRESS == msg){
        if (KEY_END == wparam || KEY_POWER == wparam){
        
            if(goku_gps_get_current_status()){
                return GOKU_HOOK_GOON;
            }

//            goku_gps_set_current_status(TRUE);
            PRINT_FUNC_LINE("POWER PRESS");
            g_bPressPowerFlg = TRUE;
//            tp_comm_channel_post_msg(NULL, TPM_GPS_CLOSE_GPS, 0, 0);
            goku_gps_thread_main_create();          
            return GOKU_HOOK_STOP;
        }
    }
    return GOKU_HOOK_GOON;
}
*/
void goku_gps_upload_last_point()
{
    goku_gps_get_last_point();
}

static void goku_gps_get_last_point()
{   
    goku_gps_start_poweroff_upload_timer();
	tp_comm_channel_post_msg(NULL, GOKU_USER_CHNL_MSG_ID_GPS_NOTIFY_IND, NULL, 0);

    if(!g_bFastUploadLastPoint){
        goku_gps_start_retry_upload_timer(GOKU_GPS_POWER_LOCATE_TIME,GOKU_GPS_DELAY_TIME);//6.5s
    }
    g_gps_has_point = FALSE;

    goku_gps_set_gps_close_flg(TRUE);

    goku_gps_poweroff_pdp_active();

    if(g_bFastUploadLastPoint){
        char t_cmdstr[GPS_UPLOADCMD_LEN] = {0};
        GPSInfo  gps_info_buff = {0};
        
        tp_os_mem_set(&gps_info_buff,0x00,sizeof(gps_info_buff));
        goku_gps_pac_point((char*)&gps_info_buff,t_cmdstr,GPS_UPLOADCMD_LEN);
        tp_os_mem_set(g_sendcmdstr,0x00,sizeof(g_sendcmdstr));
        g_sendcmdstr_len = GPS_UPLOADCMD_LEN;
        tp_os_mem_cpy(g_sendcmdstr,t_cmdstr,GPS_UPLOADCMD_LEN);
        goku_gps_set_last_point_flg(TRUE);
        
        gps_msg_handle_notice(SET_LOCAL_UPLOAD_SINGLE_POINT,NULL,0);
    }else{
        goku_gps_start_gps_chip(goku_gps_reset_timer(GOKU_GPS_POWER_LOCATE_TIME),GOKU_GPS_POWER_LOCATE_TIME);
    }
    PRINT_FUNC_LINE("==================== end =======================");    
    return;
}

gboolean goku_gps_open_pdp()
{
    if(!goku_net_query()){
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxxxxx Reg Net Fail! xxxxxxxxxxxxxxxxxx");
        return FALSE;
    }

//    if(!unipro_pdp_is_actived_indeed(ENUM_PDP_TYPE_GPS)){
    if(!goku_gps_pdp_active()){
        return FALSE;
    }

    if(unipro_pdp_is_actived_indeed(ENUM_PDP_TYPE_GPS)){
        PRINT_FUNC_LINE("^^^^^^^^^^^^^^^^^ PDP Open Success! ^^^^^^^^^^^^^^^^^^^^^");
        return TRUE;
    }
    PRINT_FUNC_LINE("PDP Result Waiting...");
    if(OS_FAILURE == goku_gps_sema_get(g_pdp_sema, GOKU_GPS_SEMA_WAIT_TIME)){//20秒超时后继续激活
       PRINT_FUNC_LINE("xxxxxxxxxxxxxxx Sema Timeout! xxxxxxxxxxxxxxxxxx");
       goku_gps_pdp_deactive();
       return FALSE;
    }

    if(!unipro_pdp_is_actived_indeed(ENUM_PDP_TYPE_GPS)){
        PRINT_FUNC_LINE("xxxxxxxxxxxxxxxx Active not Success! xxxxxxxxxxxxxxxxxx");
        goku_gps_pdp_deactive();
        return FALSE;
    }
//    }
    PRINT_FUNC_LINE("^^^^^^^^^^^^^^^^^ PDP Open Success! ^^^^^^^^^^^^^^^^^^^^^");
    return TRUE;
}

void goku_gps_start_gps_chip(gboolean NeedStart,guint interval)
{
    if(interval < 2*GOKU_GPS_ONE_SEC_LEN){
        return;
    }
    PRINT_FUNC_LINE("g_bFirstStartDriver = %d,NeedStart = %d,interval = %d,mode = %d",g_bFirstStartDriver,NeedStart,interval,g_gps_loc_mode);    
#ifndef WIN32
   if(g_bFirstStartDriver || NeedStart){
       g_bFirstStartDriver = FALSE;
       unipro_man_gps_stop();
       unipro_man_gps_rcv_start(g_gps_loc_mode,1,//关闭GPS这里可能会引起问题
        interval);
   }
#endif 
}

gboolean goku_gps_reset_timer(guint interval)
{
   gboolean bNeedStartDriver = FALSE;
   gps_fix_mode t_gps_loc_mode;

   PRINT_FUNC_LINE("goku_gps_get_gps_close_flg() = %d",goku_gps_get_gps_close_flg());

   //gps closing needn't set the mode base interval.
   if(!goku_gps_get_gps_close_flg()){
        t_gps_loc_mode = (interval >= GOKU_GPS_UP_PNT_TIMER_LIMT) ? FIX_ONE_TIME : FIX_NO_STOP;
        PRINT_FUNC_LINE("g_gps_loc_mode = %d",g_gps_loc_mode);
   }

   //according to the mode to determine whether the start gps-chip.
   if( goku_gps_get_gps_close_flg() && g_gps_loc_mode == FIX_NO_STOP){
        bNeedStartDriver = TRUE;
        g_gps_loc_mode = FIX_ONE_TIME;
   }else if(t_gps_loc_mode != g_gps_loc_mode || t_gps_loc_mode == FIX_NO_STOP){
        bNeedStartDriver = TRUE;
   }

   if(!goku_gps_get_gps_close_flg()){
        g_gps_loc_mode = t_gps_loc_mode;
   }

   if(g_gps_loc_mode == FIX_ONE_TIME){
        goku_gps_timer_restart(uUpPntTimerId,interval*1000);
        PRINT_FUNC_LINE("start FIX_ONE_TIME timer");
   }else{
        goku_gps_timer_stop(uUpPntTimerId);
        PRINT_FUNC_LINE("stop FIX_ONE_TIME timer");
   }
   
   return bNeedStartDriver;
}

void goku_gps_start_retry_upload_timer(guint timer_len,guint delay)// delay :ms
{
    guint t_tm_len = timer_len;

//    t_tm_len +=  delay;
    
    PRINT_FUNC_LINE("timer_len = %d",timer_len);
    goku_gps_timer_restart(uUpPntRetyTimerId,
        t_tm_len*1000 + delay);
}

void goku_gps_set_retry_upload_timer_len(guint interval, guint maxN)
{
    gps_upload_retry_timer_len = interval*maxN;
}

guint goku_gps_get_retry_upload_timer_len()
{
    return gps_upload_retry_timer_len;
}

gboolean goku_gps_is_timer_running(guint timer_id)
{
    gboolean bRet = FALSE;
    OS_TIMER_INFO info;
    PRINT_FUNC_LINE("================  enter =====================");
    if(timer_id == 0){
        PRINT_FUNC_LINE("TimerId Error!");
        return;
    }
    tp_os_real_timer_info_get(timer_id,&info);
    bRet = (info.timer_state == OS_TIMER_ACTIVE) ? TRUE : FALSE;

    PRINT_FUNC_LINE("bRet = %d",bRet); 
    
    return bRet;
}

void goku_gps_start_poweroff_upload_timer()
{
    PRINT_FUNC_LINE("uPoweroffUploadTimerId = %d",uPoweroffUploadTimerId);
    goku_gps_timer_start(uPoweroffUploadTimerId);
}

void goku_gps_set_last_point_upload_mode(gboolean fast_mode)
{
    g_bFastUploadLastPoint = fast_mode;
}

void goku_gps_set_force_upload_timer_len(guint force_len)
{
    if(force_len == 0){
        PRINT_FUNC_LINE("Set Fail");
        return;
    }
    g_ForceUpload_timer_len = force_len;
}

guint goku_gps_get_force_upload_timer_len()
{
    return g_ForceUpload_timer_len;
}
void goku_gps_set_last_point_flg(gboolean bWriten)
{
    g_bWriteLastPointSuccess = bWriten;
}

gboolean goku_gps_get_last_point_flg()
{
    return g_bWriteLastPointSuccess;
}

OS_TIMER_ID goku_gps_timer_create(CHAR *name,OS_FUNC_ENTRY expir_func, VOID* expire_para_ptr, UINT32 interval /*unit (s)*/,UINT32 flag)
{
    OS_TIMER_ID timer_id = OS_FAILURE;
    
    timer_id =  tp_os_real_timer_create(name,
                                      expir_func,
                                      expire_para_ptr,
                                      interval*1000,
                                      OS_DONT_LOAD | OS_DONT_ACTIVATE);
    PRINT_FUNC_LINE("timer_id = %d",timer_id);
    
    return timer_id;
}
void goku_gps_timer_start(OS_TIMER_ID id)
{
    if(id == 0){
        PRINT_FUNC_LINE("TimerId Error!");
        return;
    }
    tp_os_real_timer_start(id);
}
void goku_gps_timer_restart(OS_TIMER_ID id,UINT32 interval)
{
    if(id == 0){
        PRINT_FUNC_LINE("TimerId Error!");
        return;
    }
    tp_os_real_timer_restart(id,interval);
}

void goku_gps_timer_stop(OS_TIMER_ID id)
{
    PRINT_FUNC_LINE("============= enter =================");
    if(0 != id){
      tp_os_real_timer_stop(id);
    }
}

void goku_gps_timer_delete(OS_TIMER_ID *id)
{
    if(0 != *id){
        tp_os_real_timer_delete(*id);
        *id = 0;
    }
}
void goku_gps_print_sendbytes_total_reset()
{
    g_uSendTotalByte = 0;
}
void goku_gps_print_flow_total(char* pBuffer)
{
    sprintf(pBuffer,"Flw:\r\nSd:%ld,Rc:%ld\r\n",
            goku_gps_print_sendbytes_total(),
            goku_gps_print_recv_bytes_total());
    PRINT_FUNC_LINE("pBuffer = %s",pBuffer);
}
void goku_gps_flow_reset()
{
    goku_gps_print_sendbytes_total_reset();
    goku_gps_recv_bytes_total_reset();
}

void goku_gps_psm_request(UINT8 nDeviceId)
{
    PRINT_FUNC_LINE("GPS_UPDATE REQUEST");
    tp_psm_request_clock(nDeviceId);
}

void goku_gps_psm_release(UINT8 nDeviceId)
{
    PRINT_FUNC_LINE("GPS_UPDATE RELEASE");
    tp_psm_release_clock(nDeviceId);
}

gboolean goku_gps_get_current_status()
{
    return g_gps_current_status;
}
gboolean goku_gps_set_current_status(gboolean onoff)
{
    g_gps_current_status = onoff;
}

void goku_gps_reset_factory()
{
    if(APP_ERR_SUCCESS != tp_fexist(GPS_SETTING))
    {
        tp_fcreate(GPS_SETTING);
    }
}

gboolean goku_gps_get_running_status()
{
    return g_bRunningStatus;
}

void goku_gps_set_running_status(gboolean running)
{
    g_bRunningStatus = running;
}

OS_STATUS goku_gps_sema_get(OS_SEMA_ID sid, UINT32 timeout)
{
    if(OS_SEMA_INVALID_ID == sid){
        PRINT_FUNC_LINE("sema error");
        return OS_FAILURE;
    }
    return tp_os_sema_get(sid,timeout); 
}

OS_STATUS goku_gps_sema_put(OS_SEMA_ID sid)
{
    if(OS_SEMA_INVALID_ID == sid){
        PRINT_FUNC_LINE("sema error");
        return OS_FAILURE;
    }
    return tp_os_sema_put(sid);
}

OS_STATUS goku_gps_mutex_get(OS_MUTEX_ID mid, UINT32 timeout)
{
    if(OS_MUTEX_INVALID_ID == mid){
        PRINT_FUNC_LINE("mutex error");
        return OS_FAILURE;
    }
    return tp_os_mutex_get(mid,timeout);
}
OS_STATUS goku_gps_mutex_put(OS_MUTEX_ID mid)
{
    if(OS_MUTEX_INVALID_ID == mid){
        PRINT_FUNC_LINE("mutex error");
        return OS_FAILURE;
    }
    return tp_os_mutex_put(mid);
}

void goku_gps_set_file_load_flag(gboolean load_success)
{
    g_b_first_load_file_success_flag = load_success;
}

gboolean goku_gps_get_file_load_flag()
{
    return g_b_first_load_file_success_flag;
}
