/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_main_activity.c>
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

/*----------------- include files --------------------------------------------*/
#include "goku.h"
#include "goku_gps_main_activity.h"
#include "goku_include.h"
#include "../gps_res.h"
#include "../goku_gps_notifications.h"
#include "unipro_gal_gpspub.h"
#include "../../../unipro_comm/unipro_pdp.h"
#include "sc_net.h"
#include "unipro_feature.h"
/*----------------- file-local macro definition ------------------------------*/


/*----------------- file-local constant and type definition ------------------*/

struct _GokuGpsMainActivityPrivate
{
	HWND listCtrl;
	HWND checkbox_ctrl;
	GPSInfo gps_info;
	gboolean agps_enable;
	gboolean is_searching;
	gboolean apgs_prossing;
	gboolean gps_auto_start;		
	gps_fix_mode mod;
	guint   up_tm_len;
};

#define GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), GOKU_TYPE_GPS_MAIN_ACTIVITY, GokuGpsMainActivityPrivate))


/*----------------- file-local variables definition --------------------------*/
#ifdef WIN32
	GPSInfo info = {0};
	SVInfo sat_info[20] = {0};
    int i = 0;
#endif
    gboolean searching = FALSE;

extern SINT32 g_sChipOK;
extern UINT32 uUpPntTimerId;
extern void goku_gps_timer_stop(OS_TIMER_ID id);
/*----------------- file-local function prototype declaration ----------------*/
static void goku_gps_main_activity_imediator_interface_init( GokuIMediatorClass *iface );
static GList * goku_gps_main_activity_list_notification_interests(GokuIMediator *imediator);
static void goku_gps_main_activity_handle_notification( GokuIMediator *imediator, GokuINotification *inotification );
static void goku_gps_main_activity_on_create(GokuActivity *activity, GokuIntent *intent);
static void goku_gps_main_activity_on_finish(GokuActivity *activity);
static gint goku_gps_main_activity_on_layout_wnd_proc(GokuActivity *activity, HWND hwnd, gint msg, guint wparam, gulong lparam);
static void goku_gps_main_activity_on_popmenu_item_selected(GokuActivity *activity, HWND hwnd, guint wparam, gulong lparam);
static void goku_gps_main_activity_show_frame(HWND listCtrl,int nIDDlgItem,guint index,CONST CHAR* text);
static void goku_gps_main_activity_show_gps_info(GokuActivity *activity,GPSInfo *info);
static void goku_gps_main_activity_show_na(GokuActivity *activity);
static void goku_gps_main_activity_init_item_title(GokuActivity *activity);
static gboolean goku_gps_main_acitvity_pdp_active();
static void goku_gps_main_acitvity_pdp_deactive();
static void goku_gps_main_activity_update_title(GokuActivity *activity, UINT8 status);

extern void goku_gps_get_gps_parm(gps_fix_mode *mod,guint *up_tm_len);
extern BOOL goku_gps_sim_check();

G_DEFINE_TYPE_WITH_CODE(GokuGpsMainActivity, goku_gps_main_activity, GOKU_TYPE_ACTIVITY,
                        G_IMPLEMENT_INTERFACE (GOKU_TYPE_IMEDIATOR,
                        goku_gps_main_activity_imediator_interface_init))

/*----------------- function definition --------------------------------------*/
static void goku_gps_main_activity_class_init(GokuGpsMainActivityClass *klass)
{
    GOKU_ACTIVITY_CLASS(klass)->on_create = goku_gps_main_activity_on_create;
    GOKU_ACTIVITY_CLASS(klass)->on_layout_wnd_proc = goku_gps_main_activity_on_layout_wnd_proc;
    GOKU_ACTIVITY_CLASS(klass)->on_popmenu_item_selected = goku_gps_main_activity_on_popmenu_item_selected;
    GOKU_ACTIVITY_CLASS(klass)->on_finish = goku_gps_main_activity_on_finish;
    
    g_type_class_add_private(klass, sizeof(GokuGpsMainActivityPrivate));

}

static void goku_gps_main_activity_init(GokuGpsMainActivity *self)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(self);
    
	priv->listCtrl = HWND_NULL;
	priv->checkbox_ctrl = HWND_NULL;
	memset(&priv->gps_info,0x0,sizeof(GPSInfo));
	priv->agps_enable = FALSE;
	priv->is_searching = FALSE;
	priv->apgs_prossing = FALSE;
	priv->gps_auto_start = FALSE;
	priv->mod = FIX_ONE_TIME;
	priv->up_tm_len = 0;
}
/*************************************************************************
* Function name:   goku_gps_main_activity_imediator_interface_init
* Author:            Version: 1.0             Date: 2012-4-5
* Description:
* 1.                                                               
* 2.                                                               
* Input:      
*  1.   GokuIMediatorClass *iface 
* return:  static void                                                          
**************************************************************************/
static void goku_gps_main_activity_imediator_interface_init( GokuIMediatorClass *iface )
{
    g_return_if_fail(NULL != iface);
    
    iface->list_notification_interests = goku_gps_main_activity_list_notification_interests;    
    iface->handle_notification = goku_gps_main_activity_handle_notification;
}

/*************************************************************************
* Function name:   goku_gps_main_activity_list_notification_interests
* Author:            Version: 1.0             Date: 2012-4-5
* Description:
* 1.                                                               
* 2.                                                               
* Input:      
*  1.  GokuIMediator *imediator
* return:  static GList *                                                          
**************************************************************************/
static GList * goku_gps_main_activity_list_notification_interests(GokuIMediator *imediator)
{
    GList * list = NULL;
    
    list = g_list_append(list, NOTIFY_GPS_FIX_OK);
    list = g_list_append(list, NOTIFY_GPS_FIX_FAIL);
    list = g_list_append(list, NOTIFY_GPS_FIX_STATUS);
    list = g_list_append(list, NOTIFY_AGPS_SUCCESS);
    list = g_list_append(list, NOTIFY_AGPS_FAIL);
    list = g_list_append(list, NOTIFY_SAT_STATUS);
    list = g_list_append(list, NOTIFY_GPS_CHECK_FAIL);
    list = g_list_append(list, NOTIFY_GPS_CHECK_OK);
    return list;
}

/*************************************************************************
* Function name:   goku_gps_main_activity_handle_notification
* Author:            Version: 1.0             Date: 2012-4-5
* Description:
* 1.                                                               
* 2.                                                               
* Input:      
*  1.   GokuIMediator *imediator
*  2.  GokuINotification *inotification 
* return:  static void                                                          
**************************************************************************/
static void goku_gps_main_activity_handle_notification( GokuIMediator *imediator, GokuINotification *inotification )
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(imediator));
	const gchar *noti_name = goku_inotification_get_name(inotification);

    g_return_if_fail(NULL != noti_name);

    if (g_str_equal(noti_name, NOTIFY_GPS_FIX_STATUS))
    {
        UINT8 status = *(UINT8*)goku_inotification_get_body(inotification);
        goku_gps_main_activity_update_title(GOKU_ACTIVITY(imediator),status);
    }
    else if (g_str_equal(noti_name, NOTIFY_GPS_FIX_OK))
    {
        GPSInfo *info = (GPSInfo *)goku_inotification_get_body(inotification);
        goku_gps_main_activity_show_gps_info(GOKU_ACTIVITY(imediator),info);
#ifndef WIN32
        g_message("\n[%s]:%d\t gps显示完成\n",__func__,__LINE__);
#endif
        priv->is_searching = FALSE;
        goku_gps_main_activity_update_title(GOKU_ACTIVITY(imediator),1);
        goku_activity_softkybar_item_set_text_by_id(GOKU_ACTIVITY(imediator), GOKU_SOFTKEYBAR_ITEM_MIDDLE, IDS_STRING_SEARCH);   
        
    }
    else if (g_str_equal(noti_name, NOTIFY_GPS_FIX_FAIL))
    {
        goku_activity_modal_messagebox_by_id(GOKU_ACTIVITY(imediator), IDS_STRING_FAIL, 1, GOKU_MBX_ICON_INFO|GOKU_MBX_STYLE_OK, 0);
        goku_gps_main_activity_show_na(GOKU_ACTIVITY(imediator));
        priv->is_searching = FALSE;
        goku_gps_main_activity_update_title(GOKU_ACTIVITY(imediator),0);
        goku_activity_softkybar_item_set_text_by_id(GOKU_ACTIVITY(imediator), GOKU_SOFTKEYBAR_ITEM_MIDDLE, IDS_STRING_SEARCH);   
    }
/*
    else if (g_str_equal(noti_name, NOTIFY_AGPS_SUCCESS))
    {
        //激活,去激活成功
        if(control_checkbox_is_checked(priv->checkbox_ctrl))
        {
            control_checkbox_set_checked(priv->checkbox_ctrl, FALSE);
            goku_activity_softkybar_item_set_text_by_id(GOKU_ACTIVITY(imediator), GOKU_SOFTKEYBAR_ITEM_LEFT, IDS_STRING_OPEN);
            priv->agps_enable = FALSE;
        }
        else
        {
			control_checkbox_set_checked(priv->checkbox_ctrl, TRUE);
			goku_activity_softkybar_item_set_text_by_id(GOKU_ACTIVITY(imediator), GOKU_SOFTKEYBAR_ITEM_LEFT, IDS_STRING_CLOSE);
			priv->agps_enable = TRUE;
        }
		priv->apgs_prossing = FALSE;
        tp_comm_channel_post_msg(NULL, TPM_GPS_SET_AGPS, &priv->agps_enable, sizeof(gboolean));
    }
    else if (g_str_equal(noti_name, NOTIFY_AGPS_FAIL))
    {
        //激活,去激活失败
		priv->apgs_prossing = FALSE;
        goku_activity_modal_messagebox_by_id(GOKU_ACTIVITY(imediator), IDS_STRING_PDP_FAIL, 2, GOKU_MBX_ICON_ERROR|GOKU_MBX_STYLE_CANCEL, 0);
        //tp_comm_channel_post_msg(NULL, TPM_GPS_SET_AGPS, &priv->agps_enable, sizeof(gboolean));
    }*/
    else if (g_str_equal(noti_name, NOTIFY_SAT_STATUS))
    {
        SVInfo sat_info[20] = {0};
        char buff[20] ={0};
        char sat_show[256] = {0};
        gint id = 0;
        char sat_num[10] = {0};
        gint total_num = 0;
        gint curr_num = 0;
        char * svinfo = (char*)goku_inotification_get_body(inotification);
        HWND hStaticWnd_info = goku_activity_get_wnd_by_id(GOKU_ACTIVITY(imediator), IDC_STATIC_SAT_INFO); 
        HWND hStaticWnd_num = goku_activity_get_wnd_by_id(GOKU_ACTIVITY(imediator), IDC_STATIC_SAT_NUM); 
        
        tp_os_mem_cpy(sat_info,svinfo,sizeof(sat_info));

        while(sat_info[id].SVid != 0)
        {
            sprintf(buff,"%s%s%02d:%02d ",buff,resources_get_string_by_id(IDS_STRING_GPS_ID),sat_info[id].SVid,sat_info[id].SNR);
            if(sat_info[id].SNR)
            {
                curr_num++;
            }
            g_message("buff:%s\n",buff);
            id++;
            total_num++;
        }
        sprintf(sat_num,"(%d/%d)",curr_num,total_num);
    	control_static_set_text(hStaticWnd_num, sat_num);
    	control_static_set_text(hStaticWnd_info, buff);
    }
else if (g_str_equal(noti_name, NOTIFY_GPS_CHECK_FAIL))
  {
  	g_message("GPS check FAIL\n");
        g_sChipOK = 0;
	goku_activity_modal_messagebox_by_id(GOKU_ACTIVITY(imediator), IDS_STRING_GPS_OPEN_FAILE, 3, GOKU_MBX_ICON_INFO|GOKU_MBX_STYLE_OK, 0);
	
  }
else if (g_str_equal(noti_name, NOTIFY_GPS_CHECK_OK))
  {
  	g_message("GPS check OK\n");
  	g_sChipOK = 1;
	if(priv->gps_auto_start)
	{
		 control_checkbox_set_checked(priv->checkbox_ctrl, TRUE);
	         priv->is_searching = TRUE;
	        goku_activity_softkybar_item_set_text_by_id(GOKU_ACTIVITY(imediator), GOKU_SOFTKEYBAR_ITEM_MIDDLE, priv->is_searching ? IDS_STRING_STOP: IDS_STRING_SEARCH);
	}
  }
}
//[UNIPRO-duanzhanyang-2013-09-24] add amt test{

static void goku_gps_main_activity_on_create(GokuActivity *activity, GokuIntent *intent)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(activity));
    priv->gps_auto_start = goku_value_set_get_boolean(intent->extras, "gps_auto_start");

    if (!goku_activity_set_layout_id(activity, IDL_LAYOUT_GPS_MAIN))
    {
        goku_activity_finish_activity(activity);
    }
  	
    goku_activity_titlebar_set_text_by_id(activity, GOKU_TITLEBAR_TEXT_MAJOR, IDS_APP_GPS);
    goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_LEFT, priv->agps_enable ? IDS_STRING_CLOSE: IDS_STRING_OPEN);
    goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_MIDDLE, IDS_STRING_SEARCH);   
	goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_RIGHT, IDS_SYSTEM_STRING_BACK);
	if(0 == g_sChipOK)
	{
	    PRINT_FUNC_LINE("NO GPS!");
        goku_activity_modal_messagebox_by_id(activity, IDS_STRING_GPS_OPEN_FAILE, 3, GOKU_MBX_ICON_INFO|GOKU_MBX_STYLE_OK, 0);
        goku_activity_finish_activity(activity);
	}

    if(0 == tp_fexist(GPS_SETTING) && goku_gps_sim_check()){
        goku_gps_get_gps_parm(&priv->mod,&priv->up_tm_len);
        goku_gps_timer_stop(uUpPntTimerId);
        unipro_man_gps_stop();
    }
        
	if(priv->gps_auto_start)
	{
          priv->agps_enable = TRUE;
  		  priv->is_searching = TRUE;
	      control_checkbox_set_checked(priv->checkbox_ctrl, TRUE);
	      tp_comm_channel_post_msg(NULL, TPM_GPS_SET_AGPS, &priv->agps_enable, sizeof(gboolean));
		  goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_MIDDLE, priv->is_searching ? IDS_STRING_STOP: IDS_STRING_SEARCH);
		  #ifndef WIN32
          tp_comm_channel_post_msg(NULL, TPM_GPS_GET_INFO, &priv->is_searching, sizeof(gboolean));
		  #endif
	}
}
//[UNIPRO-duanzhanyang-2013-09-24] add amt test}

static void goku_gps_main_activity_on_finish(GokuActivity *activity)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(activity));
    if(unipro_pdp_is_actived_indeed(ENUM_PDP_TYPE_GPS))
    {
        goku_gps_main_acitvity_pdp_deactive();  
    }
    if(g_sChipOK == 1 && 0 == tp_fexist(GPS_SETTING) && goku_gps_sim_check())
    {
    #ifndef WIN32
        unipro_man_gps_rcv_start(priv->mod,TRUE,priv->up_tm_len);
    #endif
        goku_gps_timer_restart(uUpPntTimerId,priv->up_tm_len*1000);
    }
    priv->agps_enable = FALSE;
    tp_comm_channel_post_msg(NULL, TPM_GPS_SET_AGPS, &priv->agps_enable, sizeof(gboolean));
}
static gint goku_gps_main_activity_on_layout_wnd_proc(GokuActivity *activity, HWND hwnd, gint msg, guint wparam, gulong lparam)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(activity));
    
  	priv->listCtrl = goku_activity_get_wnd_by_id(activity, IDC_LIST_GPS_DETAILS);
 	priv->checkbox_ctrl =  goku_activity_get_wnd_by_id(activity, IDC_CHECKBOX_AGPS); 
 	
    switch (msg)
    {
    case MSG_INITDIALOG:
    {
        goku_gps_main_activity_init_item_title(activity);
//        goku_gps_main_activity_show_na(activity);
    }
    break;
    case MSG_KEYDOWN:
        switch (wparam)
        {        
            //左软键
        case KEY_LSK:
            {
                if(!priv->is_searching)
                {
    				if(control_checkbox_is_checked(priv->checkbox_ctrl))
    				{
//    				    priv->apgs_prossing = TRUE;
//                        goku_gps_main_acitvity_pdp_deactive();
                       control_checkbox_set_checked(priv->checkbox_ctrl, FALSE);
                       priv->agps_enable = FALSE;
    				}
    				else
    				{
/*    				
    				    if(FALSE == priv->apgs_prossing)
    				    {
                            if (GOKU_MBX_OK == goku_activity_messagebox_by_id(activity, IDS_STRING_QUERYING, 0, GOKU_MBX_STYLE_OKCANCEL|GOKU_MBX_ICON_ASK, 0, TRUE, NULL))
                            {
                                goku_gps_main_acitvity_pdp_deactive();
                                priv->apgs_prossing = TRUE;
            				    if(!goku_gps_main_acitvity_pdp_active())
            				    {
                                    priv->apgs_prossing = FALSE;
            				        goku_activity_modal_messagebox_by_id(activity, IDS_STRING_PDP_FAIL, 1, GOKU_MBX_ICON_NOTIFY|GOKU_MBX_STYLE_CANCEL, 0);
            				    }
            				}
        				}
        				else
        				{
        				    goku_activity_modal_messagebox_by_id(activity, IDS_STRING_WAITING, 1, GOKU_MBX_ICON_NOTIFY|GOKU_MBX_STYLE_CANCEL, 0);
        				}*/
                       control_checkbox_set_checked(priv->checkbox_ctrl, TRUE);
                       priv->agps_enable = TRUE;
    				}
                    tp_comm_channel_post_msg(NULL, TPM_GPS_SET_AGPS, &priv->agps_enable, sizeof(gboolean));
				 }
				 else
				 {
				    goku_activity_modal_messagebox_by_id(activity, IDS_STRING_SEARCHING, 1, GOKU_MBX_ICON_INFO|GOKU_MBX_STYLE_CANCEL, 0);
				 }
		    }
			break;
        case KEY_OK:
//            if(!priv->apgs_prossing)
            {
#ifdef WIN32
                info.SV_cnt = 5;
                sat_info[0].SVid = 5;
                sat_info[0].SNR = 23;
                sat_info[1].SVid = 17;
                sat_info[1].SNR = 9;
                sat_info[2].SVid = 6;
                sat_info[2].SNR = 2;
                sat_info[3].SVid = 22;
                sat_info[3].SNR = 19;
               sat_info[4].SVid = 5;
                sat_info[4].SNR = 0;
                sat_info[5].SVid = 17;
                sat_info[5].SNR = 9;
                sat_info[6].SVid = 6;
                sat_info[6].SNR = 2;
                sat_info[7].SVid = 22;
                sat_info[7].SNR = 19;
                priv->is_searching = !priv->is_searching;
  //              tp_comm_channel_post_msg(NULL, TPM_GPS_GET_INFO, &info, sizeof(GPSInfo));
//                tp_comm_channel_post_msg(NULL, TPM_GPS_SET_AGPS, &i, sizeof(int));
                tp_comm_channel_post_msg(NULL, TPM_GPS_GET_INFO, &sat_info, sizeof(sat_info));
                goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_MIDDLE, priv->is_searching ? IDS_STRING_STOP: IDS_STRING_SEARCH);
#else
                HWND hStaticWnd_info = goku_activity_get_wnd_by_id(activity, IDC_STATIC_SAT_INFO); 
                HWND hStaticWnd_num = goku_activity_get_wnd_by_id(activity, IDC_STATIC_SAT_NUM); 

                priv->is_searching = !priv->is_searching;
                goku_gps_main_activity_show_na(activity);
            	control_static_set_text(hStaticWnd_num, NULL);
            	control_static_set_text(hStaticWnd_info, NULL);
                g_message("[%s]:%d\t 搜索状态 %d",__func__,__LINE__,priv->is_searching);
                tp_comm_channel_post_msg(NULL, TPM_GPS_GET_INFO, &priv->is_searching, sizeof(gboolean));
                if(!priv->is_searching){
                    goku_activity_titlebar_set_text(activity, GOKU_TITLEBAR_TEXT_MAJOR, resources_get_string_by_id(IDS_APP_GPS));
                    goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_MIDDLE, IDS_STRING_SEARCH);
                }else{
                    goku_activity_softkybar_item_set_text_by_id(activity, GOKU_SOFTKEYBAR_ITEM_MIDDLE, IDS_STRING_STOP);   
                }
#endif 
            }
/*            else
            {
				goku_activity_modal_messagebox_by_id(activity, IDS_STRING_WAITING, 1, GOKU_MBX_ICON_WAIT|GOKU_MBX_STYLE_CANCEL, 0);
            }
*/            
            break;

            //右软键
        case KEY_RSK:
            goku_activity_finish_activity(activity);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return DefaultMainWinProc(hwnd, msg, wparam, lparam);

}

static void goku_gps_main_activity_on_popmenu_item_selected(GokuActivity *activity, HWND hwnd, guint wparam, gulong lparam)
{
    //
    // 菜单项被点击
    //
    switch (wparam)
    {

    default:
        break;
    }

}

static void goku_gps_main_activity_show_frame(HWND listCtrl,int nIDDlgItem,guint index,CONST CHAR* text)
{
	HWND hItem = control_list_get_item_handle(listCtrl, index);
	HWND hStaticWnd = GetDlgItem(hItem, nIDDlgItem); 
	control_static_set_text(hStaticWnd, text);
}
static void goku_gps_main_activity_show_gps_info(GokuActivity *activity,GPSInfo *info)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(activity));
    guint item = 0;
    char show[4][20] = {0};
    //经度
    sprintf(show[0],"%s:%f",((info->Lon> 0.000001) ? "E" : "W"),info->Lon);
    //纬度
    sprintf(show[1],"%s:%f",((info->Lat> 0.000001) ? "N" : "S"),info->Lat);
    //卫星数
    sprintf(show[2],"%d",info->SV_cnt);
    //定位时间
    sprintf(show[3],"%d'%.2f\"",info->loc_min,info->loc_sec);
    while(item < 4)
    {
        goku_gps_main_activity_show_frame(priv->listCtrl,IDC_STATIC_GPS_VALUE,item,show[item]);
        item++;
    }
}

static void goku_gps_main_activity_show_na(GokuActivity *activity)
{
   GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(activity));
    guint item = 0;

    while(item < 4)
    {
        goku_gps_main_activity_show_frame(priv->listCtrl,IDC_STATIC_GPS_VALUE,item++,resources_get_string_by_id(IDS_STRING_GPS_NA));
    }
    
    
}
static void goku_gps_main_activity_init_item_title(GokuActivity *activity)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(GOKU_GPS_MAIN_ACTIVITY(activity));
    int titles[] = {IDS_STRING_GPS_LON,IDS_STRING_GPS_LAT,IDS_STRING_GPS_NUMER,IDS_STRING_GPS_TIME};
    guint item = 0;

    while(item < 4)
    {
        goku_gps_main_activity_show_frame(priv->listCtrl,IDC_STATIC_GPS_TITLE,item,resources_get_string_by_id(titles[item]));
        item++;
    }
}

static gboolean goku_gps_main_acitvity_pdp_active()
{
    gboolean iRet = FALSE;
	NET_STATUS_INFO net_info = {0};

	
    g_message("\ngoku_gps_main_acitvity_pdp_active =========== enter ===========\n");

    
    tp_net_query_status(&net_info);
    	/*  0 : 未注册, 并不再搜索运营商
        2 : 未注册, 正在搜索运营商
    	3 : 注册被拒绝, 仅允许紧急呼叫*/
#ifdef WIN32
    net_info.sim_stat = 1;
#endif
	if (
    	( 0 == net_info.sim_stat)
        ||(0== net_info.reg_stat)
    	|| (2== net_info.reg_stat)
    	|| ( 3 == net_info.reg_stat))
	{
	    g_message("\ngoku_gps_main_acitvity_pdp_active net error\n");
        return 	iRet;
	}
	
    iRet = unipro_pdp_active(ENUM_PDP_TYPE_GPS);
    g_message("\ngoku_gps_main_acitvity_pdp_active iRet = %d\n",iRet);
    return iRet;
}

static void goku_gps_main_acitvity_pdp_deactive()
{
    gboolean iRet = FALSE;
    iRet = unipro_pdp_deactive(ENUM_PDP_TYPE_GPS);
    g_message("\ngoku_gps_main_acitvity_pdp_deactive iRet = %d\n",iRet);
}
static void goku_gps_main_activity_update_title(GokuActivity *activity, UINT8 status)
{
    GokuGpsMainActivityPrivate * priv = GOKU_GPS_MAIN_ACTIVITY_GET_PRIVATE(activity);
    char title[20] = {0};
    UINT32 status_id = 0;

    switch(status)
    {
    case 0:
        {
            status_id = priv->is_searching ? IDS_STRING_SEARCHING : IDS_STRING_FAIL;
            break;
        }
    case 1:
        {
            status_id = IDS_STRING_SUCCESS;
            break;
        }
    case 2:
        {
            status_id = IDS_STRING_DGPS;
            break;
        }
    case 6:
        {
            status_id = IDS_STRING_ESTIMATE;
            break;
        }
    default:
        break;
    }
    sprintf(title,"%s(%s)",resources_get_string_by_id(IDS_APP_GPS),resources_get_string_by_id(status_id));
    goku_activity_titlebar_set_text(activity, GOKU_TITLEBAR_TEXT_MAJOR, title);
}
/*------------------------------ End of file----------------------------------*/
