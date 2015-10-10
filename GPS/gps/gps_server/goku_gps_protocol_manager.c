#include "goku_gps_protocol_manager.h"
/******************************************************************************************
*pengzhaoyang
*******************************************************************************************/
static const GPS_CMD_T g_tGPScmdTypeInfo[]=
{
    {SET_SERVER_NULL,           ""},
    {SET_SERVER_SMS_NUM,        "S2"},
    {SET_SERVER_IP_PORT,        "S23"},
    {SET_SERVER_LINK_NUM,       "S25"},
    {SET_DOMAIN_PORT,           "S26"},
    {SET_SERVER_MULT_CMD,       "S28"},
    {SET_SERVER_APN_TMZONE,     "S29"},
    {SET_SERVER_RUNING_MODE,    "S31"},
    {SET_SERVER_LOCATE_ENABLE,  "S32"},
    {SET_SERVER_FENCE,          "S33"},
    {SET_SERVER_LOCATE_CONFIG,  "G1"},
    {GET_TIME_SYNC_REQ,         "V5"},
    {SET_LOCAL_UPLOAD,          ""},
    {SET_LOCLA_BEAT_PACK,       "V6"},
    
};

locate_config g_tLocCfg = {0};//用于存放定位配置
static UINT8 g_uGpsStatus = 0x0E;
static GPS_CFG_T g_gps_cfg = {0};
static GPS_CFG_T g_tgps_cfg = {0};
static gchar  g_gps_info_last[60] = {0};
static guint g_tm_sync_len = 0;
static gboolean g_poweroffFlg = FALSE;
static gboolean g_bCloseGpsFlg = FALSE;

UINT8  g_imei_array[6]={0};
//---------------------------------------------------------------------------

void goku_gps_get_print_cfg(char* pBuffer)
{
    sprintf(pBuffer,"\r\nCFG:\r\nCfgTM:%s Bt:%d\r\nPF:%d",
        g_gps_cfg.renewTime,g_gps_cfg.queryInterval,g_poweroffFlg);
}
gboolean goku_gps_cmdstr_decode(gchar* cmdstr,cmdstr_info* cmdstrinfo)
{
    gchar* dev_num = NULL;
    gchar* cmd = NULL;
    gchar* tmstr = NULL;
    gchar* cmdparm = NULL;
    PRINT_FUNC_LINE("=============enter=================");

    PRINT_FUNC_LINE("cmdstr = %s",cmdstr);
    g_return_val_if_fail(NULL != cmdstr, FALSE);
    
    if(!goku_gps_is_legal_cmdstr(cmdstr))
    {
        PRINT_FUNC_LINE("cmdstr illegal");
        return FALSE;
    }

    //sn
    dev_num = goku_gps_get_devnum_from_cmdstr(cmdstr);
    tp_os_mem_cpy(cmdstrinfo->dev_num,dev_num,strlen(dev_num));
    GPS_FREE(dev_num);
    
    //cmd
    cmd = goku_gps_get_cmdstr_from_cmdstr(cmdstr);
    tp_os_mem_cpy(cmdstrinfo->cmdstr,cmd,strlen(cmd));

    if(0 == strcmp(cmd,"V5"))
    {
        //date
        tmstr = goku_gps_get_date_from_cmdstr(cmdstr);
        PRINT_FUNC_LINE("date tmstr = %s",tmstr);
        tp_os_mem_cpy(cmdstrinfo->dtstr,tmstr,strlen(tmstr));
        GPS_FREE(tmstr);
    }
    
    //time
    tmstr = goku_gps_get_time_from_cmdstr(cmdstr);
    tp_os_mem_cpy(cmdstrinfo->tmstr,tmstr,strlen(tmstr));
    GPS_FREE(tmstr);


    //当为时间同步时无需在向下看后面得参数了
    if(0 == strcmp(cmd,"V5"))
    {
        cmdstrinfo->syn_intval = goku_gps_get_syc_interval_from_cmdstr(cmdstr);
        GPS_FREE(cmd);
        return TRUE;
    }
    GPS_FREE(cmd);
    //cmd parm
    cmdparm = goku_gps_get_cmdparm_from_cmdstr(cmdstr);
    tp_os_mem_cpy(cmdstrinfo->parm,cmdparm,strlen(cmdparm));
    GPS_FREE(cmdparm);
    
    //cmd type
    cmdstrinfo->type = goku_gps_get_cmd_type_from_cmdstr(cmdstr);
    return TRUE;
}

gchar* goku_gps_get_dev_num(void)
{

//    gchar *sn = "98765432101";// for test
    gchar *dev_num = NULL;
    char  imei[20] ={0};
    char  lhf[7] = {0};
    PRINT_FUNC_LINE("=============== enter ===================");    
    tp_query_imei_sync((UINT8*)imei);
    dev_num = g_strdup(&imei[4]);
    PRINT_FUNC_LINE("dev_num = %s",&imei[4]);
    
    return dev_num;
}

gchar* goku_gps_get_current_time(RADIX_E radix)
{
    MAN_RTC_TIME tTime;
    gchar curr_tm[GPS_TIME_LENG] = {0};
    gchar* t_tm = NULL;
    
    PRINT_FUNC_LINE("=============enter=================");
    tp_man_time_get(&tTime);

    switch(radix)
    {
        case RADIX_DEC:
        {
            sprintf(curr_tm,
                     "%02d%02d%02d",
                     tTime.hour,
                     tTime.min,
                     tTime.sec);
            break;
        }
        case RADIX_HEX:
        {
            sprintf(curr_tm,
                     "%c%c%c",//输出第一个字节
                     tTime.hour,
                     tTime.min,
                     tTime.sec);
            break;
        }
        default:
        {
            sprintf(curr_tm,
                     "%02d%02d%02d",
                     tTime.hour,
                     tTime.min,
                     tTime.sec);
            break;
        }
    }

    PRINT_FUNC_LINE("curr_tm = %s",curr_tm);
    t_tm = g_strdup(curr_tm);
    PRINT_FUNC_LINE("=============end=================");
    return t_tm;
}

gchar* goku_gps_get_current_date(RADIX_E radix)
{
    MAN_RTC_DATE tDate;
    gchar curr_dt[GPS_TIME_LENG] = {0};
    gchar* dt = NULL;
    
    PRINT_FUNC_LINE("=============enter=================");
    tp_man_date_get(&tDate);
    
    switch(radix)
    {
        case RADIX_DEC:
        {
        sprintf(curr_dt,
                 "%02d%02d%02d",
                 tDate.year%2000,
                 tDate.month,
                 tDate.day);
            break;
        }
        case RADIX_HEX:
        {
        sprintf(curr_dt,
                 "%02x%02x%02x",
                 tDate.year%2000,
                 tDate.month,
                 tDate.day);
            break;
        }
        default:
        {
            sprintf(curr_dt,
                     "%02d%02d%02d",
                     tDate.year%2000,
                     tDate.month,
                     tDate.day);
            break;
        }
    }

    PRINT_FUNC_LINE("curr_dt = %s",curr_dt);
    dt = g_strdup(curr_dt);
    PRINT_FUNC_LINE("=============end=================");
    return dt;
}
gboolean goku_gps_is_legal_cmdstr( gchar* cmdstr)
{
    gboolean iRet = TRUE;
    gchar*  t_str = NULL;
    
    PRINT_FUNC_LINE("=================enter================");

    iRet = goku_gps_get_cmdstr_head(cmdstr);
    if(FALSE == iRet)
    {
        PRINT_FUNC_LINE("goku_gps_get_cmdstr_head ERROR");
        return iRet;
    }

    iRet = goku_gps_get_cmdstr_tail(cmdstr,GPS_PROTOCOL_TAIL);
    if(FALSE == iRet)
    {
        PRINT_FUNC_LINE("goku_gps_get_cmdstr_tail ERROR");
        return iRet;
    }

    t_str = goku_gps_get_devnum_from_cmdstr(cmdstr);
    if(NULL == t_str)// Maybe cause memory leak?
    {
        PRINT_FUNC_LINE("goku_gps_get_cmdstr_dev_num ERROR");
        return FALSE;
    }
    GPS_FREE(t_str);

    t_str = goku_gps_get_cmdstr_from_cmdstr(cmdstr);
    if(NULL == t_str)// Maybe cause memory leak?
    {
        PRINT_FUNC_LINE("goku_gps_get_cmdstr_from_cmdstr ERROR");
        return FALSE;
    }
    PRINT_FUNC_LINE("cmd t_str = %s",t_str);
    if(0 == strstr(t_str,"V5"))
    {
        GPS_FREE(t_str);
        //date
        t_str = goku_gps_get_date_from_cmdstr(cmdstr);
        if(NULL == t_str)
        {
            PRINT_FUNC_LINE("goku_gps_get_date_from_cmdstr ERROR");
            return FALSE;
        }
    }
    GPS_FREE(t_str);

    t_str = goku_gps_get_time_from_cmdstr(cmdstr);
    if(NULL == t_str)
    {
        PRINT_FUNC_LINE("goku_gps_get_time_from_cmdstr ERROR");
        return FALSE;
    }
    GPS_FREE(t_str);

    t_str = strstr(cmdstr,"V5");
    if(NULL == t_str)
    {
        PRINT_FUNC_LINE("NO V5");
        if(0 == goku_gps_get_cmd_type_from_cmdstr(cmdstr))
        {
            PRINT_FUNC_LINE("goku_gps_get_cmd_type ERROR");
            return FALSE;
        }
    }
     
    return iRet;
}

gboolean goku_gps_get_cmdstr_head(gchar* cmdstr )
{
    gchar* str_ptr = NULL;
    gchar* t_cmdstr = NULL;
    gboolean iRet = TRUE;

    PRINT_FUNC_LINE("=================enter==============");
    g_return_val_if_fail(NULL != cmdstr, FALSE);
    
    str_ptr = strstr(cmdstr,GPS_PROTOCOL_HEAD(BG));
    PRINT_FUNC_LINE("cmdstr head = %s",str_ptr);
    if(NULL == str_ptr)
    {
        iRet = FALSE;
        PRINT_FUNC_LINE("cmdstr ERROR");
    }

    PRINT_FUNC_LINE("=================end==============");
    return iRet;
}

gboolean goku_gps_get_cmdstr_tail(gchar* cmdstr,char gps_protocol_tail)
{
    gboolean iRet = TRUE;
    gchar * tail = NULL;
    
    PRINT_FUNC_LINE("=================enter==============");
    tail = strrchr(cmdstr,gps_protocol_tail);
    if(NULL == tail)
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        iRet = FALSE;
    }
    PRINT_FUNC_LINE("=================end==============");
    return iRet;
}

gchar* goku_gps_get_devnum_from_cmdstr(gchar* cmdstr)
{
    gchar* t_dev_num_start = NULL;
    gchar* t_dev_num_end = NULL;
    gchar* t_cmdstr = NULL;
    guint dev_num_len = 0;
    gchar dev_num[GPS_DEV_NUM_LENG+1] = {0};
    gchar* dev = NULL;
    PRINT_FUNC_LINE("=================enter==============");

    g_return_val_if_fail(NULL != cmdstr, NULL);

    t_cmdstr = cmdstr;
    PRINT_FUNC_LINE("t_cmdstr = %s",t_cmdstr);
    t_dev_num_start = strchr(t_cmdstr,GPS_PROTOCOL_COMMA);
    PRINT_FUNC_LINE("t_dev_num_start = %s",t_dev_num_start);
    if(NULL == t_dev_num_start)
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }
    t_dev_num_start++;
    t_dev_num_end = strchr(t_dev_num_start,GPS_PROTOCOL_COMMA);
    PRINT_FUNC_LINE("t_dev_num_end = %s",t_dev_num_end);
    if(NULL == t_dev_num_end)
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }

    dev_num_len = t_dev_num_end - t_dev_num_start;
    PRINT_FUNC_LINE("dev_num_len = %d",dev_num_len);
    if(dev_num_len > GPS_DEV_NUM_LENG)
    {
        PRINT_FUNC_LINE("dev_num_len ERROR");
        return NULL;
    }
    tp_os_mem_cpy(dev_num,t_dev_num_start,dev_num_len);

    dev = g_strdup(dev_num);
    PRINT_FUNC_LINE("dev = %s",dev);
    return dev;
    
}

gchar* goku_gps_get_cmdstr_by_cmd(GPS_CMD_CMD_E cmd)
{
    guint index;
    gchar* cmdstr = NULL;
    PRINT_FUNC_LINE("====================enter=======================");
    if(cmd < SET_SERVER_NULL || cmd > SET_SERVER_MAX)
    {
        PRINT_FUNC_LINE("cmd ERROR");
        return NULL;
    }
    
    for(index = 0; index < SET_SERVER_MAX; index++)
    {
        if(cmd == g_tGPScmdTypeInfo[index].cmd)
        {
            cmdstr = g_strdup(g_tGPScmdTypeInfo[index].cmdstr);
            PRINT_FUNC_LINE("%s",cmdstr);
            PRINT_FUNC_LINE("====================end=======================");
            return cmdstr;
        }
    }
    return NULL;
}

gint goku_gps_get_cmd_by_cmdstr(gchar* cmdstr)
{
    guint index;
    PRINT_FUNC_LINE("====================enter=======================");

    g_return_val_if_fail(NULL != cmdstr,0);
    
    for(index = 0; index < SET_SERVER_MAX; index++)
    {
        if(0 == strcmp(g_tGPScmdTypeInfo[index].cmdstr,cmdstr))
        {
            PRINT_FUNC_LINE("%d",g_tGPScmdTypeInfo[index].cmd);
            PRINT_FUNC_LINE("====================end=======================");
            return g_tGPScmdTypeInfo[index].cmd;
        }
    }
    return 0;
}

gchar* goku_gps_get_cmdstr_from_cmdstr(gchar* cmdstr)
{
    gchar* t_cmdstr_start = NULL;
    gchar* t_cmdstr_end = NULL;
    gchar* t_cmdstr = NULL;
    guint t_cmdstr_len = 0;
    gchar acmdstr[GPS_CMD_LEN+1] = {0};
    gchar* cmd = NULL;
    
    PRINT_FUNC_LINE("=================enter==============");

    g_return_val_if_fail(NULL != cmdstr, NULL);

    t_cmdstr = cmdstr;

    if(NULL == (t_cmdstr_start = strchr(t_cmdstr,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }
    t_cmdstr_start++;
    if(NULL == (t_cmdstr_start = strchr(t_cmdstr_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }
    t_cmdstr_start++;
    if(NULL == (t_cmdstr_end = strchr(t_cmdstr_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }

    t_cmdstr_len = t_cmdstr_end - t_cmdstr_start;
    if(t_cmdstr_len > GPS_CMD_LEN)
    {
        return NULL;
    }
    
    tp_os_mem_cpy(acmdstr,t_cmdstr_start,t_cmdstr_len);

    cmd = g_strdup(acmdstr);
    PRINT_FUNC_LINE("cmd = %s",cmd);
    return cmd;
}

gchar* goku_gps_get_date_from_cmdstr(gchar* cmdstr)
{
    gchar* t_dt_start = NULL;
    gchar* t_dt_end = NULL;
    guint t_dt_len = 0;
    gchar tmstr[GPS_TIME_LENG+1] = {0};
    gchar* t_dt = NULL;
    
    PRINT_FUNC_LINE("=================enter==============");

    g_return_val_if_fail(NULL != cmdstr, NULL);

   
    if(NULL == (t_dt_start = strchr(cmdstr,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("datestr ERROR");
        return NULL;
    }
    t_dt_start++;
    PRINT_FUNC_LINE("t_dt_start = %s",t_dt_start);
    if(NULL == (t_dt_start = strchr(t_dt_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("datestr ERROR");
        return NULL;
    }
    t_dt_start++;
    PRINT_FUNC_LINE("t_dt_start = %s",t_dt_start);
    if(NULL == (t_dt_start = strchr(t_dt_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("datestr ERROR");
        return NULL;
    }
    t_dt_start++;
    PRINT_FUNC_LINE("t_dt_start = %s",t_dt_start);
    if(NULL == (t_dt_end = strchr(t_dt_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("datestr ERROR");
        return NULL;
    }
    PRINT_FUNC_LINE("t_dt_end = %s",t_dt_end);
    t_dt_len = t_dt_end - t_dt_start;
    if(t_dt_len > GPS_DATE_LENG)
    {
        return NULL;
    }

    tp_os_mem_cpy(tmstr,t_dt_start,t_dt_len);

    t_dt = g_strdup(tmstr);
    PRINT_FUNC_LINE("t_dt = %s",t_dt);
    return t_dt;

}

gchar* goku_gps_get_time_from_cmdstr(gchar* cmdstr)
{
    gchar* t_tm_start = NULL;
    gchar* t_tm_end = NULL;
    gchar* t_cmdstr = NULL;
    guint t_tm_len = 0;
    gchar tmstr[GPS_TIME_LENG+1] = {0};
    gchar* t_tm = NULL;
    
    PRINT_FUNC_LINE("=================enter==============");

    g_return_val_if_fail(NULL != cmdstr, NULL);

    t_cmdstr = cmdstr;

    if(NULL == (t_tm_start = strchr(t_cmdstr,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("get_time ERROR");
        return NULL;
    }
    t_tm_start++;
    if(NULL == (t_tm_start = strchr(t_tm_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("get_time ERROR");
        return NULL;
    }
    t_tm_start++;
    if(NULL == (t_tm_start = strchr(t_tm_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("get_time ERROR");
        return NULL;
    }
    t_tm_start++;
    if(NULL != strstr(cmdstr,"V5"))
    {
        if(NULL == (t_tm_start = strchr(t_tm_start,GPS_PROTOCOL_COMMA)))
        {
            PRINT_FUNC_LINE("get_time ERROR");
            return NULL;
        }
        t_tm_start++;
    }
    
    if(NULL == (t_tm_end = strchr(t_tm_start,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("get_time ERROR");
        return NULL;
    }

    t_tm_len = t_tm_end - t_tm_start;
    if(t_tm_len > GPS_TIME_LENG)
    {
        return NULL;
    }

    tp_os_mem_cpy(tmstr,t_tm_start,t_tm_len);

    t_tm = g_strdup(tmstr);
    PRINT_FUNC_LINE("t_tm = %s",t_tm);
    return t_tm;

}

guint  goku_gps_get_cmd_type_from_cmdstr(gchar* cmdstr)
{
    gchar* t_type_start = NULL;
    gchar* t_type_end = NULL;
    guint t_type_len = 0;
    guint t_type = 0;
    gchar* t_cmdstr  = NULL;
    gchar cmd_type[GPS_TYPE_LEN+1] = {0};
    PRINT_FUNC_LINE("=================enter==============");
    g_return_val_if_fail(NULL != cmdstr,0);
    t_cmdstr = cmdstr;

    if(NULL == (t_type_start = strrchr(t_cmdstr,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return 0;
    }
    t_type_start++;
    PRINT_FUNC_LINE("t_type_start = %s",t_type_start);
    if(NULL == (t_type_end = strrchr(t_type_start,GPS_PROTOCOL_TAIL)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return 0;
    }
    PRINT_FUNC_LINE("t_type_start = %s",t_type_start);
    
    t_type_len = t_type_end - t_type_start;
    PRINT_FUNC_LINE("t_type_len = %d",t_type_len);
    if(t_type_len > GPS_TYPE_LEN)
    {
        PRINT_FUNC_LINE("t_type_len ERROR");
        return 0;
    }
    tp_os_mem_cpy(cmd_type,t_type_start,t_type_len);
    t_type = atoi(cmd_type);
    PRINT_FUNC_LINE("t_type = %d",t_type);
    if(1 <= t_type || t_type <= 2)
    {
        PRINT_FUNC_LINE("=================end==============");
        return t_type;
    }
    PRINT_FUNC_LINE("t_type ERROR");
    return 0;     
}

guint  goku_gps_get_syc_interval_from_cmdstr(gchar* cmdstr)
{
    gchar* t_interval_start = NULL;
    gchar* t_interval_end = NULL;
    guint t_interval_len = 0;
    guint t_inerval = 0;
    gchar* t_cmdstr  = NULL;
    gchar cmd_type[GPS_INTERVAL_LEN+1] = {0};
    PRINT_FUNC_LINE("=================enter==============");
    g_return_val_if_fail(NULL != cmdstr,0);
    t_cmdstr = cmdstr;

    if(NULL == (t_interval_start = strrchr(t_cmdstr,GPS_PROTOCOL_COMMA))){
        PRINT_FUNC_LINE("cmdstr ERROR");
        return 0;
    }
    t_interval_start++;
    if(NULL == (t_interval_end = strrchr(t_interval_start,GPS_PROTOCOL_TAIL))){
        PRINT_FUNC_LINE("cmdstr ERROR");
        return 0;
    }
    
    t_interval_len = t_interval_end - t_interval_start;
    if(t_interval_len > GPS_INTERVAL_LEN){
        PRINT_FUNC_LINE("t_interval_len ERROR");
        return 0;
    }
    tp_os_mem_cpy(cmd_type,t_interval_start,t_interval_len);
    t_inerval = atoi(cmd_type);
    PRINT_FUNC_LINE("t_inerval = %d",t_inerval);
    if(1 <= t_inerval || t_inerval <= 99){
        return t_inerval;
    }
    PRINT_FUNC_LINE("t_inerval ERROR");
    return 0;     
}

gchar* goku_gps_get_cmdparm_from_cmdstr(gchar* cmdstr)
{
    gchar* t_cmdparm_start = NULL;
    gchar* t_cmdparm_end = NULL;
    guint t_cmdparm_len = 0;
    gchar* t_cmdstr  = NULL;
    gchar cmdparm[GPS_PARAM_LEN+1] = {0};
    gchar* t_parm = NULL;
    guint i = 0;
    
    PRINT_FUNC_LINE("=================enter==============");
    g_return_val_if_fail(NULL != cmdstr,NULL);
    t_cmdstr = cmdstr;

    if(NULL == (t_cmdparm_start = strrchr(t_cmdstr,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }
    t_cmdparm_start++;
    
    while(i < 3)
    {
        if(NULL == (t_cmdparm_start = strrchr(t_cmdparm_start,GPS_PROTOCOL_COMMA)))
        {
            PRINT_FUNC_LINE("cmdstr ERROR");
            return NULL;
        }
        t_cmdparm_start++;
        i++;
    }
    if(NULL == (t_cmdparm_end = strrchr(t_cmdstr,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdstr ERROR");
        return NULL;
    }
    
    t_cmdparm_len = t_cmdparm_end - t_cmdparm_start;
    if(t_cmdparm_len > GPS_PARAM_LEN)
    {
        return NULL;
    }

    tp_os_mem_cpy(cmdparm,t_cmdparm_start,t_cmdparm_len);
    t_parm = g_strdup(cmdparm);
    PRINT_FUNC_LINE("t_parm = %s",t_parm);
    return t_parm;
    
}

gboolean goku_gps_cmdparm_decode(GPS_CMD_CMD_E cmd,gchar* cmdparm)
{
    gboolean bRet = FALSE;
    switch(cmd)
    {
        case GET_TIME_SYNC_REQ:
        {
            goku_gps_date_time_set(cmdparm);

            bRet = TRUE;
            break;
        }
        case SET_SERVER_LOCATE_CONFIG://G1
        {
            if(goku_gps_locateconfig_decode(cmdparm))
            {
                //保存配置信息
                //设置配置
                //返回确认信息
            }
            break;
        }
        case SET_SERVER_MULT_CMD:
        {
            //
            break;
        }
        case SET_SERVER_LOCATE_ENABLE://S32
        {
            if(goku_gps_locable_decode(cmdparm))
            {
                //保存使能状态 
                //设置gps使能
                //返回确认信息
                PRINT_FUNC_LINE("%s","SET_SERVER_LOCATE_ENABLE");
            }
            break;
        }
        default:
        {
            PRINT_FUNC_LINE("decode cmd ERROR");
            break;
        }
    }

    return bRet;
}

//G1
gboolean goku_gps_locateconfig_decode(gchar* cmdparm)
{
    guint  t_interval_val = 0;
    guint  t_total_cnt_val = 0;
    gchar  t_interval[GPS_INTERVAL_LEN+1] = {0};
    gchar  t_total_cnt[GPS_INTERVAL_LEN+1] = {0};
    gchar* t_locateconfig_start = NULL;
    gchar* t_locateconfig_end = NULL;    
    gchar* t_cmdparm = NULL;
    guint  t_interval_len = 0;
    guint  t_total_cnt_len = 0;

    PRINT_FUNC_LINE("=================enter==============");
    g_return_val_if_fail(NULL != cmdparm,FALSE);
    
    t_cmdparm = cmdparm;
    t_locateconfig_start = t_cmdparm;
    if(NULL == (t_locateconfig_end = strchr(t_cmdparm,GPS_PROTOCOL_COMMA)))
    {
        PRINT_FUNC_LINE("cmdparm ERROR");
        return FALSE;
    }
    t_interval_len = t_locateconfig_end -t_locateconfig_start -1;
    tp_os_mem_cpy(t_interval,t_locateconfig_start,t_interval_len);

    if(-1 < strcmp(t_interval,"10") || 1 < strcmp(t_interval,"65535"))
    {
        PRINT_FUNC_LINE("t_interval ERROR");
        return FALSE;
    }
    t_interval_val = atoi(t_interval);
    PRINT_FUNC_LINE("t_interval_val = %d",t_interval_val);
    g_tLocCfg.interval = t_interval_val;

    t_locateconfig_end++;
    tp_os_mem_cpy(t_total_cnt,t_locateconfig_end,strlen(t_locateconfig_end));
    
    if(-1 < strcmp(t_total_cnt,"1") || 1 < strcmp(t_total_cnt,"65535"))
    {
        PRINT_FUNC_LINE("t_total_cnt ERROR");
        return FALSE;
    }
    t_total_cnt_val = atoi(t_total_cnt);
    PRINT_FUNC_LINE("t_total_cnt_val = %d",t_total_cnt_val);
    g_tLocCfg.total_cnt = t_total_cnt_val;

    return TRUE;
}

//S32
gboolean goku_gps_locable_decode(gchar* cmdparm)
{
    guint  t_locateable_val = 0;
    gchar  t_locable[GPS_LOCATEABLE_LEN+1] = {0}; 
    gchar* t_cmdparm = NULL;
    guint  t_locateable_len = 0;

    PRINT_FUNC_LINE("=================enter==============");
    g_return_val_if_fail(NULL != cmdparm,FALSE);
    
    t_cmdparm = cmdparm;
    t_locateable_len = strlen(t_cmdparm);
    tp_os_mem_cpy(t_locable,t_cmdparm,t_locateable_len);

    if(0 == strcmp(t_locable,"1") || 0 == strcmp(t_locable,"0"))
    {
        t_locateable_val = atoi(t_locable);
        PRINT_FUNC_LINE("t_locateable_val = %d",t_locateable_val);
        return TRUE;
    }
    
    PRINT_FUNC_LINE("t_locable ERROR");
    return FALSE;
}

//locate cmdstr package
gboolean goku_gps_pack_req_cmdstr(GPS_CMD_CMD_E cmd, gchar* cmdparm,gchar* output)
{
    gchar  t_cmdstr[GPS_CMDSTR_LEN+1] = {0};
    gchar* t_pcmdstr = NULL;
    gint index = 0;
    gchar* t_content[7] = {NULL};
    
    PRINT_FUNC_LINE("=================enter==============");
    PRINT_FUNC_LINE("cmd = %d",cmd);
    g_return_val_if_fail(cmd > SET_SERVER_NULL && cmd <= SET_SERVER_MAX,FALSE);

    t_content[0] = g_strdup(GPS_PROTOCOL_HEAD(BG));
    t_content[1] = goku_gps_get_dev_num();
    t_content[2] = goku_gps_get_cmdstr_by_cmd(cmd);
    if(cmd == GET_TIME_SYNC_REQ)
    {
        t_content[3] = goku_gps_get_current_date(RADIX_DEC);
    }
    else
    {
        t_content[3] = goku_gps_get_config_time();
    }

    t_content[4] = goku_gps_get_current_time(RADIX_DEC);
    t_content[5] = (NULL == cmdparm ) ? g_strdup("") : g_strdup(cmdparm);
    t_content[6] = g_strdup("#");

    while(index < 7)
    {
        PRINT_FUNC_LINE("t_content[%d] = %s",index ,t_content[index]);
        index++;
    }

    for(index = 0;index < 7;index++)
    {
        if((index == 3 && cmd != GET_TIME_SYNC_REQ && cmd != SET_LOCLA_BEAT_PACK) || NULL == t_content[index])
        {
            PRINT_FUNC_LINE("index = %d,cmd = %d",index,cmd);
            continue;
        }
        sprintf(t_cmdstr,"%s%s",t_cmdstr,t_content[index]);
        if(index == 1 || index == 2 || index ==3 || (index ==4 && 0 != strcmp(t_content[5],"")))
        {
            sprintf(t_cmdstr,"%s%c",t_cmdstr,GPS_PROTOCOL_COMMA);
        }
        PRINT_FUNC_LINE("t_pcmdstr[%d] = %s",index,t_cmdstr);
    }
    PRINT_FUNC_LINE("t_pcmdstr = %s",t_cmdstr);

    index = 0;
    while(index < 7)
    {
        PRINT_FUNC_LINE("mem free index = %d ", index);
        GPS_FREE(t_content[index]);
        index++;
    }

    t_pcmdstr = g_strdup(t_cmdstr);
    tp_os_mem_cpy(output,t_pcmdstr,strlen(t_pcmdstr));
    GPS_FREE(t_pcmdstr);
    PRINT_FUNC_LINE("output = %s",output);
    return TRUE;
    
}

//V5
gchar* goku_gps_tm_sync_req()
{
    GPS_CMD_CMD_E cmd = GET_TIME_SYNC_REQ;
    gchar* cmdparm = NULL;

    PRINT_FUNC_LINE("=================enter==============");
    return NULL; //goku_gps_pack_req_cmdstr(cmd,cmdparm);
}

void goku_gps_get_imei(UINT8* imei_array)
{
    UINT8  chlat[20] = {0};
    char   chTemp[6] = {0};
    char  imei[20] ={0};
    guint  i = 0;
    char  lhf[7] = {0};
#if WIN32
    UINT64 llimei = 0;
#else
    long long llimei = 0;
#endif
    PRINT_FUNC_LINE("============== enter =====================");

    if(NULL == imei_array){
        PRINT_FUNC_LINE("Input Param Error!");
        return;
    }
    tp_query_imei_sync((UINT8*)imei);
    PRINT_FUNC_LINE("&imei[4] = %s",&imei[4]);

    tp_os_mem_cpy(lhf,&imei[4],6);
#if WIN32
    llimei = 100000 * (UINT64)atol(lhf);
#else
    llimei = 100000 * (long long int)atol(lhf);
#endif
    PRINT_FUNC_LINE("llimei = %lld",llimei);
    
    tp_os_mem_set(lhf,0x00,6);
    tp_os_mem_cpy(lhf,&imei[10],5);
    llimei += atol(lhf);
    PRINT_FUNC_LINE("llimei = %lld",llimei);
    
    memcpy(chlat,&llimei,5);

    while(i <= 4)
    {
        chTemp[i] = chlat[4 - i];
        i++;
    }
    memcpy(imei_array,chTemp,5);
}

void goku_gps_pac_point(char* parm,char out[],guint num)
{ 
 
    UINT8 t_buff[GPS_PROTOCOL_UPLOAD_MAX] = {0};
    GPSInfo *t_gps_info = (GPSInfo*)parm;
    MAN_RTC_TIME tTime;
    MAN_RTC_DATE tDate;
    UINT8* pReturn = NULL;
    LOCATION_INFO location_info = {0};

    PRINT_FUNC_LINE("================= enter ========================");

    if(NULL == out || num < GPS_PROTOCOL_UPLOAD_MAX || NULL == parm){
        PRINT_FUNC_LINE("parm error!");
        return;
    }
    //包头"$"
    t_buff[0] = 0x24;
    PRINT_FUNC_LINE("buf[0] = %x",t_buff[0]);
    
    //SN

    // [UNIPRO-pengzhaoyang-2013-12-26] for 这里直接获取IMEI号会导致线程被切换，所以在初始化时直接获取后面直接使用 
    memcpy(&t_buff[1],&g_imei_array,5);
    
    PRINT_FUNC_LINE("buf[1] = %x,buf[2] = %x,buf[3] = %x,buf[4] = %x,buf[5] = %x",
        t_buff[1],t_buff[2],t_buff[3],t_buff[4],t_buff[5]);

    //time
    /*
    tTime.hour = t_gps_info->hour;
    tTime.min = t_gps_info->min;
    tTime.sec = t_gps_info->sec;
    */
    tp_man_time_get(&tTime);
    
    PRINT_FUNC_LINE("hour = %02d,min = %02d,sec = %02d",tTime.hour,tTime.min,tTime.sec);
    
    t_buff[6]   =  ((tTime.hour)/10)<<4;
    t_buff[6]   += (tTime.hour)%10;
    t_buff[7]   =  (tTime.min/10)<<4;
    t_buff[7]   += tTime.min%10;
    t_buff[8]   =  (tTime.sec/10)<<4;
    t_buff[8]   += tTime.sec%10;

    //date
    tp_man_date_get(&tDate);
    
     PRINT_FUNC_LINE("year = %02d,mon = %02d,day = %02d",tDate.year,tDate.month,tDate.day);
    t_buff[9]    =  (tDate.day/10)<<4;
    t_buff[9]   +=  tDate.day%10;
    t_buff[10]   =  (tDate.month/10)<<4;
    t_buff[10]  +=  tDate.month%10;
    t_buff[11]   =  ((tDate.year%2000)/10)<<4;
    t_buff[11]  +=  (tDate.year%2000)%10;

    goku_gps_set_stat(t_gps_info);
    //LAT
    pReturn = goku_gps_get_lat(t_gps_info->Lat);
    tp_os_mem_cpy(&t_buff[12],pReturn,4);
    GPS_FREE(pReturn);

    //保留一个字节
    t_buff[16]   = 0x00;

    //LON
    pReturn = goku_gps_get_lon(t_gps_info->Lon);
    tp_os_mem_cpy(&t_buff[17],pReturn,5);
    GPS_FREE(pReturn);

    //GPS STATUS
    t_buff[21] &= 0xF0;//低四位清零
    t_buff[21] |= goku_gps_get_stat();

    //速度&方向
    pReturn = goku_gps_get_speed_direction(t_gps_info);
    tp_os_mem_cpy(&t_buff[22],pReturn,3);
    GPS_FREE(pReturn);


    //车辆状态
    pReturn = goku_gps_get_vehicle_cbatus();
    tp_os_mem_cpy(&t_buff[25],pReturn,4);
    GPS_FREE(pReturn);


    //用户自定义警报状态
    t_buff[29] = 0xFF;

    //保留一个字节
    t_buff[30]   = 0x00;

    tp_query_location_info(&location_info);
    //MCC
    pReturn = goku_gps_get_mcc(location_info.mcc);
    tp_os_mem_cpy(&t_buff[31],pReturn,3);
    GPS_FREE(pReturn);
  
    //MNC
    pReturn = goku_gps_get_mnc(location_info.mnc);
    tp_os_mem_cpy(&t_buff[34],pReturn,3);
    GPS_FREE(pReturn);
 
    //LAC
    pReturn = goku_gps_get_lac(location_info.lac);
    tp_os_mem_cpy(&t_buff[37],pReturn,3);
    GPS_FREE(pReturn);

    //CID
    pReturn = goku_gps_get_cid(location_info.ci);
    tp_os_mem_cpy(&t_buff[40],pReturn,3);
    GPS_FREE(pReturn);

    //pack flag
    t_buff[43] = 0x00;

    tp_os_mem_cpy(out,t_buff,GPS_PROTOCOL_UPLOAD_MAX);
    PRINT_FUNC_LINE("打包完成");
    goku_gps_print_LastPoint(t_gps_info,tTime,tDate);
}

gchar* goku_gps_pack_locates()
{
    return NULL;
}

//获取纬度
UINT8* goku_gps_get_lat(float lat)
{
    char chlat[20] = {0};
    char* pRec;
    PRINT_FUNC_LINE("============== enter =====================");
    pRec = goku_gps_valtostring(lat);
    sprintf(chlat,"%s",pRec);
    GPS_FREE(pRec);
    
    return goku_gps_convert_value_to_data(chlat,FALSE);
}

//获取经度
UINT8* goku_gps_get_lon(float lon)//有点问题
{
    char chlat[20] = {0};
    char* pRec;
    
    PRINT_FUNC_LINE("============== enter =====================");
    pRec = goku_gps_valtostring(lon);
    sprintf(chlat,"%s",pRec);
    GPS_FREE(pRec);
    
    return goku_gps_convert_value_to_data(chlat,FALSE);
}
UINT8* goku_gps_get_speed_direction(GPSInfo* info)
{
//    float spd = 12.000000;
//    float dir = 28.000000;
    char chspddir[20] = {0};

    PRINT_FUNC_LINE("============== enter =====================");
//    spd = spdir->Speed;
//    dir = spdir->Track;
    sprintf(chspddir,"%03d%03d",(guint)info->Speed,(guint)info->Speed);
    
    return goku_gps_convert_value_to_data(chspddir,FALSE);
}

UINT8* goku_gps_get_vehicle_cbatus()
{
    UINT8  status = 0x00;
    UINT8  chspddir[20] = {0};
    UINT8* pretun = (UINT8*)tp_os_mem_malloc(20);
    
    status = 0x5F;
    chspddir[0] = status;

    status = goku_gps_get_battery();;
    chspddir[1] = status;
    
    status = goku_gps_battery_low_flag();
    chspddir[2] = status;
    
    status = 0xF7;
    chspddir[3] = status;

    tp_os_mem_cpy(pretun,chspddir,20);
    
    return pretun;

}

UINT8* goku_gps_get_mcc(UINT16 mcc)
{
    char chTmp[20] = {0};
    UINT8* pRetun = NULL;
    guint  uLen = 0;

    sprintf(chTmp,"%d",goku_gps_cvt_mcc_mnc(mcc));
    pRetun = goku_gps_convert_value_to_data(chTmp,TRUE);
    uLen = (strlen(chTmp)+1)/2;
    tp_os_mem_set(chTmp,0x00,20);
    tp_os_mem_cpy(chTmp,pRetun,uLen);
    GPS_FREE(pRetun);

    while(uLen < 3)
    {
        chTmp[uLen++] += 0xFF;
    }
    pRetun = (UINT8*)tp_os_mem_malloc(20);
    tp_os_mem_cpy(pRetun,chTmp,20);
    
    return pRetun;
}

UINT8* goku_gps_get_mnc(UINT16 mnc)
{
    char chTmp[20] = {0};
    UINT8* pRetun = NULL;
    guint  uLen = 0;
    
    sprintf(chTmp,"%d",goku_gps_cvt_mcc_mnc(mnc));
    pRetun = goku_gps_convert_value_to_data(chTmp,TRUE);
    uLen = (strlen(chTmp)+1)/2;
    tp_os_mem_set(chTmp,0x00,20);
    tp_os_mem_cpy(chTmp,pRetun,uLen);
    GPS_FREE(pRetun);
    
    while(uLen < 3)
    {
        chTmp[uLen++] |= 0xFF;
    }
    pRetun = (UINT8*)tp_os_mem_malloc(20);  
    tp_os_mem_cpy(pRetun,chTmp,20);
    
    return pRetun;
}

UINT8* goku_gps_get_lac(UINT16 lac)
{
    char chTmp[20] = {0};
    UINT8* pRetun = NULL;
    guint  uLen = 0;
    
    sprintf(chTmp,"%d",lac);
    pRetun = goku_gps_convert_value_to_data(chTmp,TRUE);
    uLen = (strlen(chTmp)+1)/2;
    tp_os_mem_set(chTmp,0x00,20);
    tp_os_mem_cpy(chTmp,pRetun,uLen);
    GPS_FREE(pRetun);
    
    while(uLen < 3)
    {
        chTmp[uLen++] |= 0xFF;
    }
    pRetun = (UINT8*)tp_os_mem_malloc(20);  
    tp_os_mem_cpy(pRetun,chTmp,20);
    
    return pRetun;
}

UINT8* goku_gps_get_cid(UINT16 cid)
{
    char chTmp[20] = {0};
    UINT8* pRetun = NULL;
    guint  uLen = 0;
    
    sprintf(chTmp,"%d",cid);
    pRetun = goku_gps_convert_value_to_data(chTmp,TRUE);
    uLen = (strlen(chTmp)+1)/2;
    tp_os_mem_set(chTmp,0x00,20);
    tp_os_mem_cpy(chTmp,pRetun,uLen);
    GPS_FREE(pRetun);
    
    while(uLen < 3)
    {
        chTmp[uLen++] |= 0xFF;
    }
    pRetun = (UINT8*)tp_os_mem_malloc(20);  
    tp_os_mem_cpy(pRetun,chTmp,20);
    
    return pRetun;
}

UINT8* goku_gps_convert_value_to_data(char* val,gboolean FILL)
{
    gchar  chTmp[20+1] = {0};
    gchar  chVal[20+1] = {0};
    char* pTmp = NULL;
    UINT8* pretun = (UINT8*)tp_os_mem_malloc(21);
    guint  uLen = 0;
    guint  i = 0;
    UINT8  uNum = 0;
    char  chNum[3] = {0};
    
    PRINT_FUNC_LINE("================== enter ==========================");
//    TP_OS_ASSERT(20 < strlen(val));
   g_return_val_if_fail(strlen(val) < 20, NULL);

    PRINT_FUNC_LINE("val = %s,FILL = %d",val,FILL);
    uLen = strlen(val);
    tp_os_mem_cpy(chVal,val,uLen+1);
    pTmp = chVal;
    
    while(i < uLen){
       if(chVal[i] != '.'){
           i++;
           continue;
       }
       while(i < uLen){
         chVal[i] = chVal[i+1];
         i++;
       }
    }

    i = 0;
    if(*(pTmp+1) != 0x00){//判断第二个字节是否为 '\0'
        while(*(pTmp+1) != 0x00){//下一个字节为 '\0'则退出
           tp_os_mem_cpy(chNum,pTmp,2);
           uNum = (UINT8)atoi(chNum);
           chTmp[i] = (uNum/10) <<4;
           chTmp[i] += uNum%10;
           pTmp += 2;//一次加两个字节
           i++;
        }
        if(*pTmp != 0x00){
            uNum = (UINT8)atoi(pTmp);
            chTmp[i] = uNum << 4;//直接左移4位

            if(FILL){
                chTmp[i] += 0x0F;
            }
        }

    }else{
        uNum = (UINT8)atoi(chVal);
        chTmp[0]  = (uNum/10)<<4;
        chTmp[0] += uNum%10;
        if(FILL){
            chTmp[0] += 0x0F ;
        }
    }
    
    tp_os_mem_cpy(pretun,chTmp,21);
    
    return pretun;
}

void goku_gps_set_stat(GPSInfo* info)
{
    g_uGpsStatus = 0x0E;
    PRINT_FUNC_LINE("============== enter =======================");
    if((info->Lat < 0.000001 &&  info->Lat > -0.000001)&&
       (info->Lon < 0.000001 &&  info->Lat > -0.000001)){
        g_uGpsStatus &= ~0x02;
        PRINT_FUNC_LINE("g_uGpsStatus =  %x",g_uGpsStatus);
    }
    
    if(info->Lon < -0.000001){
       g_uGpsStatus &= ~0x04;
       PRINT_FUNC_LINE("g_uGpsStatus =  %x",g_uGpsStatus);
    }
    
    if(info->Lat < -0.000001){
       g_uGpsStatus &= ~0x08;
       PRINT_FUNC_LINE("g_uGpsStatus =  %x",g_uGpsStatus);
    }

}

UINT8 goku_gps_get_stat()
{
    return g_uGpsStatus;
}

char* goku_gps_valtostring(float gps_info)
{
    float d = 0;
    float m = 0;
    char t_buf[20] = {0};
    char* pRetun;
    float tmp = 0;
    PRINT_FUNC_LINE("============= enter ================");
    d = (int)abs(gps_info);
    m = (gps_info - d)*60.0;
    sprintf(t_buf,"%.4f",d*100.0 + m);

    pRetun = g_strdup(t_buf);
    return pRetun;
}

UINT8 goku_gps_get_battery()
{
   UINT32 vol = 0;
   UINT8  uRet = 0;
   MAN_BATTERY_STATUS bat_status;

   PRINT_FUNC_LINE("============== enter ================");
   tp_man_battery_get_status(&bat_status);
   vol = bat_status.vol_level;

    switch(vol)
    {
    case 0:
        {
            uRet = 0x0F;
        }
        break;
    case 1:
        {
            uRet = 0x1F;
        }
        break;
    case 2:
        {
            uRet = 0x3F;
        }
        break;
    case 3:
        {
            uRet = 0x7F;
        }
        break;
    case 4:
        {
            uRet = 0xAF;
        }
        break;
    default:
        uRet = 0x0F;
    }
    
/*   
   if(per >= 0 && per <9){
      uRet = 0x0F;
   }else if(per >= 9 && per <19){
        uRet = 0x1F;
   }else if(per >= 19 && per <29){
        uRet = 0x2F;   
   }else if(per >= 29 && per <39){
        uRet = 0x3F;
   }else if(per >= 39 && per <49){
        uRet = 0x4F;
   }else if(per >= 49 && per <59){
        uRet = 0x5F;
   }else if(per >= 59 && per <69){
        uRet = 0x6F;
   }else if(per >= 69 && per <79){
        uRet = 0x7F;   
   }else if(per >= 79 && per <89){
        uRet = 0x8F;   
   }else if(per >= 89 && per <=99){
        uRet = 0x9F;   
   }else if(per == 100){
        uRet = 0xAF;
   }
*/   
   if(goku_gps_get_gps_close_flg()){
        uRet &= 0xFB;
   }
   PRINT_FUNC_LINE("uRet = %x",uRet);
   return uRet;  
}

UINT8 goku_gps_battery_low_flag()
{
   UINT32 per = 0;
   UINT8  uRet = 0xFF;
   MAN_BATTERY_STATUS bat_status;

   PRINT_FUNC_LINE("============== enter ================");
   tp_man_battery_get_status(&bat_status);
   if(bat_status.vol_level <= 1)
   {
        uRet = 0xF7;
   }
   return uRet;
}
guint goku_gps_cvt_mcc_mnc(UINT16 mcc_mnc)
{
    char t_tmp[10] = {0};
    char t_val[10] = {0};
    guint uReturn = 0; 
    gint i = 0;
    PRINT_FUNC_LINE("================== enter =======================");
    PRINT_FUNC_LINE("mcc_mnc = %d",mcc_mnc);
    sprintf(t_tmp,"%03x",mcc_mnc);
    i = strlen(t_tmp) -1;
    while(i >= 0)
    {
        t_val[2- i] = t_tmp[i];
        i--;
    }
    uReturn = atoi(t_val);    
    PRINT_FUNC_LINE("mcc = %d",uReturn);
    return uReturn;
}
gboolean goku_gps_set_config_time(char* cfg_tm)
{
    PRINT_FUNC_LINE("=================== enter =======================");
    
    g_return_val_if_fail(NULL != cfg_tm, FALSE);
    tp_os_mem_set(g_gps_cfg.renewTime,0x00,sizeof(g_gps_cfg.renewTime));
    
    tp_os_mem_cpy(g_gps_cfg.renewTime,cfg_tm,strlen(cfg_tm));
}
char* goku_gps_get_config_time()
{
    char* cfg_tm = NULL;
    PRINT_FUNC_LINE("=================== enter =======================");
    cfg_tm = g_strdup(g_gps_cfg.renewTime);
    return cfg_tm;
}

gboolean goku_gps_dcode_config(char* cmdstr)
{
    char pTmp_val[400];
    
    PRINT_FUNC_LINE("=================== enter =======================");
    g_return_val_if_fail(NULL != cmdstr, FALSE);

    if('{' != *cmdstr && 0 != strrchr(cmdstr,'}'))
    {
        return FALSE;
    }
    
    tp_os_mem_set(pTmp_val,0x00,400);
    if(goku_gps_get_item_from_config("renewTime",cmdstr,pTmp_val))
    {
        PRINT_FUNC_LINE("pTmp_val = %s,g_gps_cfg.renewTime = %s",pTmp_val,g_gps_cfg.renewTime);
        if(0 >= strcmp(pTmp_val,g_gps_cfg.renewTime))
        {
            PRINT_FUNC_LINE("Needn't Update!");
            return TRUE;
        }
        goku_gps_set_config_time(pTmp_val);
        tp_os_mem_set(pTmp_val,0x00,400);
        //interval
        if(!goku_gps_get_item_from_config("interval",cmdstr,pTmp_val))
        {
            PRINT_FUNC_LINE("interval error!");
            return FALSE;
        }
        goku_gps_set_recv_interval(atoi(pTmp_val));
        tp_os_mem_set(pTmp_val,0x00,400);
        //queryInterval
        if(!goku_gps_get_item_from_config("queryInterval",cmdstr,pTmp_val))
        {
            PRINT_FUNC_LINE("queryInterval error!");
            return FALSE;
        }
//        g_tgps_cfg.queryInterval = atoi(pTmp_val);
        goku_gps_set_cfg_beattime(atoi(pTmp_val));
        tp_os_mem_set(pTmp_val,0x00,400);
        //countPerPackage
        if(!goku_gps_get_item_from_config("countPerPackage",cmdstr,pTmp_val))
        {
            PRINT_FUNC_LINE("countPerPackage error!");
            return FALSE;
        }
        goku_gps_set_recv_maxN(atoi(pTmp_val));
        tp_os_mem_set(pTmp_val,0x00,400);

        return TRUE;
    }

    return FALSE;
}

gboolean goku_gps_get_item_from_config(char* item,char* cmdstr,char* item_val)
{
    char* pTitle = NULL;
    char* pStart = NULL;
    char* pEnd = NULL;
    guint item_val_len = 0;

    PRINT_FUNC_LINE("================ enter ========================");
    if(NULL == (pTitle = strstr(cmdstr,item)))
    {
        return FALSE;
    }

    pStart = strchr(pTitle,':');
    pStart++;
    pEnd = strchr(pStart,',');
    item_val_len = pEnd - pStart;

    if('"' == *pStart)
    {
        pStart++;
        item_val_len -= 2;//去掉两个引号
    }
    tp_os_mem_cpy(item_val,pStart,item_val_len);

    return TRUE;
    
}
void goku_gps_print_LastPoint(GPSInfo * gps_info, MAN_RTC_TIME tTime, MAN_RTC_DATE tDate)
{
    if((gps_info->Lat < 0.000001 &&  gps_info->Lat > -0.000001)&&
       (gps_info->Lon < 0.000001 &&  gps_info->Lat > -0.000001)){
        return;
    }
    tp_os_mem_set(&g_gps_info_last,0x00,sizeof(g_gps_info_last));
    sprintf(g_gps_info_last,"Lat:%s Lon:%s DT:%d-%02d-%02d %02d:%02d:%02d",
        goku_gps_valtostring(gps_info->Lat),goku_gps_valtostring(gps_info->Lon),
        tDate.year,tDate.month,tDate.day,
        tTime.hour+8,tTime.min,tTime.sec);
    PRINT_FUNC_LINE("g_gps_info_last = %s",g_gps_info_last);
}

void goku_gps_get_print_point(char* pBuffer)
{
    sprintf(pBuffer,"%s\r\n",g_gps_info_last);
}

gboolean goku_gps_set_cfg_interval(gint interval)
{
    if(interval < GPS_PROTOCOL_MIN_INTERVAL){
        PRINT_FUNC_LINE("Set Interval Fail");
        return FALSE;
    }
    g_gps_cfg.interval = interval;
    return TRUE;
}

guint goku_gps_get_cfg_interval()
{
    return g_gps_cfg.interval;
}

gboolean goku_gps_set_cfg_maxN(gint maxN)
{
    if(maxN < GPS_PROTOCOL_MIN_MAXN || maxN > GPS_PROTOCOL_MAX_MAXN){
        PRINT_FUNC_LINE("Set maxN Fail");
        return FALSE;
    }
    g_gps_cfg.countPerPackage = maxN;
    return TRUE;
}

guint goku_gps_get_cfg_maxN()
{
    return g_gps_cfg.countPerPackage;
}

gboolean goku_gps_set_cfg_beattime(gint beattime)
{
    if( beattime <= 0 ){
        PRINT_FUNC_LINE("Set beattime Fail");
        return FALSE;
    }
    g_gps_cfg.queryInterval = beattime;
    return TRUE;
}

guint goku_gps_get_cfg_beattime()
{
    return g_gps_cfg.queryInterval;
}
void goku_gps_protocol_data_reset()
{
    tp_os_mem_set(&g_gps_cfg,0x00,sizeof(g_gps_cfg));
    tp_os_mem_cpy(g_gps_cfg.renewTime,"20130101T080000",strlen("20130101T080000"));
    g_gps_cfg.interval = GOKU_GPS_UP_PNT_TIMER_LEN;
    g_gps_cfg.queryInterval = GOKU_GPS_BEAT_TIMER_LEN;
    g_gps_cfg.countPerPackage = GPS_UPLOAD_MAXCNT;
    tp_os_mem_set(&g_tgps_cfg,0x00,sizeof(g_tgps_cfg));
    
    g_tm_sync_len = 24;
}
void goku_gps_date_time_set(gchar* cmdparm)
{
    MAN_RTC_TIME tTime = {0};
    MAN_RTC_DATE tDate = {0};
    char chDat[4] = {0};
    guint i = 0;
    char* pDatTim = NULL;
    cmdstr_info* t_dt = (cmdstr_info*)cmdparm;
    
    pDatTim = t_dt->tmstr;

    PRINT_FUNC_LINE("pDatTim = %s",pDatTim);
    tp_os_mem_cpy(chDat,pDatTim,2);
    tTime.hour = (UINT8)atoi(chDat);
    PRINT_FUNC_LINE("tTime.hour = %d", tTime.hour);
    tp_os_mem_set(chDat,0x00,4);
    pDatTim += 2;

    PRINT_FUNC_LINE("pDatTim = %s",pDatTim);
    tp_os_mem_cpy(chDat,pDatTim,2);
    tTime.min = (UINT8)atoi(chDat);
    PRINT_FUNC_LINE("tTime.min = %d", tTime.min);
    tp_os_mem_set(chDat,0x00,4);
    pDatTim += 2;

    PRINT_FUNC_LINE("pDatTim = %s",pDatTim);
    tp_os_mem_cpy(chDat,pDatTim,2);
    tTime.sec= (UINT8)atoi(chDat);
    PRINT_FUNC_LINE("tTime.sec = %d", tTime.sec);
    tp_os_mem_set(chDat,0x00,4);
    pDatTim += 2;
    tp_man_time_set(tTime);

    pDatTim = t_dt->dtstr;
    PRINT_FUNC_LINE("pDatTim = %s",pDatTim);
    tp_os_mem_cpy(chDat,pDatTim,2);
    tDate.year = (UINT16)(atoi(chDat)+2000);
    PRINT_FUNC_LINE("tDate.year = %d", tDate.year);
    tp_os_mem_set(chDat,0x00,4);
    pDatTim += 2;

    PRINT_FUNC_LINE("pDatTim = %s",pDatTim);
    tp_os_mem_cpy(chDat,pDatTim,2);
    tDate.month = (UINT8)atoi(chDat);
    PRINT_FUNC_LINE("tDate.month = %d", tDate.month);
    tp_os_mem_set(chDat,0x00,4);
    pDatTim += 2;

    PRINT_FUNC_LINE("pDatTim = %s",pDatTim);
    tp_os_mem_cpy(chDat,pDatTim,2);
    tDate.day = (UINT8)atoi(chDat);
    PRINT_FUNC_LINE("tDate.day = %d", tDate.day);
    tp_os_mem_set(chDat,0x00,4);
    pDatTim += 2;

    tp_man_date_set(tDate);

    g_tm_sync_len = t_dt->syn_intval;
    
}

guint goku_gps_get_tm_sync_len()
{
    PRINT_FUNC_LINE("============== enter ===================");
    return g_tm_sync_len*GOKU_GPS_ONE_HOUR_LEN;
}

gboolean goku_gps_get_gps_close_flg()
{
    PRINT_FUNC_LINE("g_bCloseGpsFlg = %d",g_bCloseGpsFlg);
    return g_bCloseGpsFlg;
}

void goku_gps_set_gps_close_flg(gboolean bclose)
{
    g_bCloseGpsFlg = bclose;
    PRINT_FUNC_LINE("g_bCloseGpsFlg = %d",g_bCloseGpsFlg);
}


void goku_gps_set_recv_interval(guint recv_interval)
{
    g_tgps_cfg.interval = recv_interval ;
}

guint  goku_gps_get_recv_interval()
{
    return g_tgps_cfg.interval;
}

void goku_gps_set_recv_maxN(guint maxN)
{
    g_tgps_cfg.countPerPackage = maxN ;
}

guint  goku_gps_get_recv_maxN()
{
    return g_tgps_cfg.countPerPackage;
}
/*****************************************FILE_END*************************************************************************************************************/
