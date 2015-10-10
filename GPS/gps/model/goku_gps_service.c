/*******************************************************************************
    Copyright(c) 2012 - 2013 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_service.c>
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
#include "goku_gps_service.h"
#include "goku_gps_notifications.h"
#include "unipro_gal_gpspub.h"
#include "setting_service\goku_setting_new_service_iface.h"

#define GOKU_GPS_CHANNEL_NAME "GOKU_GPS_CHANNEL_NAME"
/*----------------- file-local macro definition ------------------------------*/

static void goku_gps_service_iproxy_iface_init(GokuIProxyClass *iface);

G_DEFINE_TYPE_WITH_CODE(GokuGpsService, goku_gps_service, GOKU_TYPE_SERVICE,
                        G_IMPLEMENT_INTERFACE(GOKU_TYPE_IPROXY,
                                              goku_gps_service_iproxy_iface_init))

/*----------------- file-local constant and type definition ------------------*/
typedef struct _GokuGpsServicePrivate GokuGpsServicePrivate;

struct _GokuGpsServicePrivate
{
    GokuISetting *iSetting;
    gint32 start_tick;
    gint32 end_tick;
    gboolean is_suspend;
    gboolean agps_enable;
};
#define GOKU_GPS_SERVICE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), GOKU_TYPE_GPS_SERVICE, GokuGpsServicePrivate))


/*----------------- file-local variables definition --------------------------*/

/*----------------- file-local function prototype declaration ----------------*/

static const gchar * goku_gps_service_get_name(GokuIProxy *iproxy); 
static void goku_gps_service_finalize(GObject *object);
static void goku_gps_service_dispose(GObject *object);
static GList * goku_gps_service_get_interested_msgs(GokuService *self);
static void goku_gps_service_on_msg(GokuService *self, gint msg, gconstpointer data, guint size);
static gboolean goku_gps_service_on_start(GokuService *self);
static gboolean goku_gps_service_on_stop(GokuService *self);
static gboolean goku_gps_chnl_msg_unregister(void);
static gboolean goku_gps_chnl_msg_register(void *data);
static gint32 goku_gps_chnl_msg_proc(guint32 msg_id, void* param, guint32 msg_size);
static void goku_gps_service_get_during_time(GokuGpsServicePrivate *priv,gconstpointer data);

extern BOOL goku_gps_sim_check();
extern SINT32 g_sChipOK;

/*----------------- function definition --------------------------------------*/
static void goku_gps_service_class_init(GokuGpsServiceClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize			       = goku_gps_service_finalize;
    G_OBJECT_CLASS(klass)->dispose				   =  goku_gps_service_dispose;
    GOKU_SERVICE_CLASS(klass)->get_interested_msgs = goku_gps_service_get_interested_msgs;
    GOKU_SERVICE_CLASS(klass)->on_msg              = goku_gps_service_on_msg;
    GOKU_SERVICE_CLASS(klass)->on_start            = goku_gps_service_on_start;
    GOKU_SERVICE_CLASS(klass)->on_stop             = goku_gps_service_on_stop;

    g_type_class_add_private(klass, sizeof(GokuGpsServicePrivate));
}

/**********************************************************************************
* Function:  goku_gps_service_init
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static void goku_gps_service_init(GokuGpsService *self)
{
    GokuGpsServicePrivate *priv = GOKU_GPS_SERVICE_GET_PRIVATE(self);

    priv->iSetting = NULL;
    priv->start_tick = 0;
    priv->end_tick = 0;
    priv->is_suspend = FALSE;
    priv->agps_enable = FALSE;
}

static gboolean goku_gps_service_init_config(GokuService *self)
{
	//获取是否配置过

	return TRUE;
}


/**********************************************************************************
* Function:  goku_gps_service_on_msg
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static void goku_gps_service_on_msg(GokuService *self, gint msg, gconstpointer data, guint size)
{
	GokuGpsServicePrivate *priv = GOKU_GPS_SERVICE_GET_PRIVATE(self);
	
	g_return_if_fail(priv != NULL);
	switch (msg)
	{
    case TPM_GPS_FIX_STATUS:
    	{
#ifndef WIN32
		g_message("[%s]:%d\t gps 定位中\n",__func__,__LINE__);
#endif	    
	    goku_service_notify(GOKU_SERVICE(self), NOTIFY_GPS_FIX_STATUS, data);
        break;
    	}
    case TPM_GPS_FIX_OK:
    	{
    	    priv->end_tick = tp_os_current_tick_get();
    	    goku_gps_service_get_during_time(priv,data);
#ifndef WIN32
			g_message("[%s]:%d\t gps 定位成功\n",__func__,__LINE__);
            g_message("[%s]:%d\t priv->end_tick=%d\n",__func__,__LINE__,priv->end_tick);
//            unipro_man_gps_suspend();
#endif
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_GPS_FIX_OK, data);
           	break;
    	}

   	case TPM_GPS_FIX_FAIL:
    	{
#ifndef WIN32
//            unipro_man_gps_suspend();
#endif
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_GPS_FIX_FAIL, data);
          	break;
    	}
   	case TPM_GPS_AGPS_OK:
    	{
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_AGPS_SUCCESS, data);
          	break;
    	}
   	case TPM_GPS_AGPS_FAIL:
    	{
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_AGPS_FAIL, data);
          	break;
    	}
    case TPM_GPS_SAT_STATUS:
        {
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_SAT_STATUS, data);
            break;
        }
   case TPM_GPS_CHECK_FAIL:
        {
	    g_message("TPM_GPS_CHECK_FAIL \n");
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_GPS_CHECK_FAIL, data);
            break;
        }
    case TPM_GPS_CHECK_OK:
        {
	    g_message("TPM_GPS_CHECK_OK \n");
            goku_service_notify(GOKU_SERVICE(self), NOTIFY_GPS_CHECK_OK, data);
            break;
        }
   	default:
            return;
	}
    
	return;
}

/**********************************************************************************
* Function:  goku_gps_service_finalize
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static void goku_gps_service_finalize(GObject *object)
{
    GokuGpsService *self = GOKU_GPS_SERVICE(object);
    GokuGpsServicePrivate *priv = GOKU_GPS_SERVICE_GET_PRIVATE(self);

    // TODO: finalize private variables

    G_OBJECT_CLASS(goku_gps_service_parent_class)->finalize(object);
}

/**********************************************************************************
* Function:  goku_gps_service_iproxy_iface_init
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static void goku_gps_service_iproxy_iface_init(GokuIProxyClass *iface)
{
   iface->get_name = goku_gps_service_get_name;
}

/**********************************************************************************
* Function:  goku_gps_service_get_name
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static const gchar * goku_gps_service_get_name(GokuIProxy *iproxy)
{
    return GOKU_GPS_SERVICE_NAME;
}
 
/**********************************************************************************
* Function:  goku_gps_service_dispose
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static void goku_gps_service_dispose(GObject *object)
{
    // TODO:
	GokuGpsService *self = GOKU_GPS_SERVICE(object);
	GokuGpsServicePrivate *priv = GOKU_GPS_SERVICE_GET_PRIVATE(self);

    g_object_unref(priv->iSetting);

    G_OBJECT_CLASS(goku_gps_service_parent_class)->dispose(object);
}

/**********************************************************************************
* Function:  goku_gps_service_dispose
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static GList * goku_gps_service_get_interested_msgs(GokuService *self)
{
    // TODO:
    GList * msgs = NULL;

    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_FIX_OK));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_FIX_FAIL));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_FIX_STATUS));    
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_GET_INFO));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_SET_AGPS));    
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_AGPS_OK));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_AGPS_FAIL));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_SAT_STATUS));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_PDP_ACTIVATE_RESULT));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_PDP_DEACTIVATE_CONFORM));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_PDP_DEACTIVATE_INDICATION));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_CHECK_FAIL));
    msgs = g_list_append(msgs, GINT_TO_POINTER(TPM_GPS_CHECK_OK));
    g_message("goku_gps_service_get_interested_msgs\n");
    return msgs;

}

/**********************************************************************************
* Function:  goku_gps_service_on_start
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static gboolean goku_gps_service_on_start(GokuService *self)
{
    goku_gps_chnl_msg_register((void *)self);
#ifndef WIN32
        unipro_man_gps_open();
#endif
    return TRUE;
}

/**********************************************************************************
* Function:  goku_gps_service_on_stop
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
static gboolean goku_gps_service_on_stop(GokuService *self)
{
    goku_gps_chnl_msg_unregister();
#ifndef WIN32
    if(0 != tp_fexist(GPS_SETTING))
    {
        unipro_man_gps_close();
    }
#endif
    return TRUE;
}

/**********************************************************************************
* Function:  goku_gps_service_new
*
* Purpose:   
*
* Relation:  <describing the name, version and position of protocol involved by
*            this function>
*
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
*   
*
* Return:   
*
* Note:     <the limitations to use this function or other comments>
*
***********************************************************************************/
GokuGpsService* goku_gps_service_new(void)
{
    return g_object_new(GOKU_TYPE_GPS_SERVICE, NULL);
}

static gboolean goku_gps_chnl_msg_unregister(void)
{
	if (TRUE == tp_comm_channel_query(GOKU_GPS_CHANNEL_NAME)) {
		tp_comm_channel_destroy(GOKU_GPS_CHANNEL_NAME);
	}
	
	return TRUE;
}

static gboolean goku_gps_chnl_msg_register(void *data)
{
	int result;
	
	result = tp_comm_channel_create(GOKU_GPS_CHANNEL_NAME, (COMM_MSG_PROC)goku_gps_chnl_msg_proc, data);
	
	/* register platform message */
	result = tp_comm_channel_reg_msg(GOKU_GPS_CHANNEL_NAME, TPM_GPS_GET_INFO);
	result = tp_comm_channel_reg_msg(GOKU_GPS_CHANNEL_NAME, TPM_GPS_SET_AGPS);
	result = tp_comm_channel_reg_msg(GOKU_GPS_CHANNEL_NAME, TPM_PDP_ACTIVATE_RESULT);
	result = tp_comm_channel_reg_msg(GOKU_GPS_CHANNEL_NAME, TPM_PDP_DEACTIVATE_CONFORM);
	result = tp_comm_channel_reg_msg(GOKU_GPS_CHANNEL_NAME, TPM_PDP_DEACTIVATE_INDICATION);
	//end add
	return TRUE;
}

static gint32 goku_gps_chnl_msg_proc(guint32 msg_id, void* param, guint32 msg_size)
{
	gint32 result = 0;
	GokuGpsService *service = (GokuGpsService *)tp_comm_channel_get_add_data(GOKU_GPS_CHANNEL_NAME);
    GokuGpsServicePrivate *priv = GOKU_GPS_SERVICE_GET_PRIVATE(service);
     
	g_return_val_if_fail(service, FALSE);
	
	switch (msg_id) 
	{    
	case TPM_GPS_GET_INFO:
    	{
#ifdef WIN32
            priv->start_tick = tp_os_current_tick_get();
//            tp_os_thread_sleep(15213);
            goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_SAT_STATUS, param, msg_size);
//            goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_FIX_OK, param, msg_size);
#else
            if(FALSE == *(gboolean *)param)
            {
                unipro_man_gps_stop();
                g_message("[%s]:%d\t gps 关闭\n",__func__,__LINE__);
            }
            else
            {
                priv->start_tick = tp_os_current_tick_get();
                g_message("[%s]:%d\t priv->start_tick=%d\n",__func__,__LINE__,priv->start_tick);
/*                if(TRUE == priv->is_suspend)
                {
                    unipro_man_gps_resume();
                }
                else
                {
                    unipro_man_gps_rcv_start();
                    priv->is_suspend = TRUE;
                }
*/
                unipro_man_gps_rcv_start(0,priv->agps_enable,0);
                g_message("[%s]:%d\t gps 打开\n",__func__,__LINE__);
            }
#endif
    	}
    	break;
    	case TPM_GPS_SET_AGPS:
    	{
#ifdef WIN32
            goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_FIX_STATUS, param, msg_size);
#else
            priv->agps_enable = *(gboolean *)param;
//            unipro_man_need_agps(priv->agps_enable);
            g_message("[%s]:%d\t agps %d\n",__func__,__LINE__,*(gboolean*)param);
#endif
    	}
    	break;
/*    	
        case TPM_PDP_ACTIVATE_RESULT://pdp激活结果
        {
            SC_PDP_CID_INFO_ST*  re = (SC_PDP_CID_INFO_ST*)param;
            if(re->cid_send_to_app)
            {
                goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_AGPS_OK, NULL, 0);
            }
            else
            {
                goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_AGPS_FAIL, NULL, 0);
            }
            g_message("[goku_gps_chnl_msg_proc],TPM_PDP_ACTIVATE_RESULT,re->cid_send_to_app=%d\n",re->cid_send_to_app); 
        }
        break;
        case TPM_PDP_DEACTIVATE_CONFORM://本地发起去激活成功
        {
            UINT8 uResult = *(UINT8*)param;
            goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_AGPS_OK, NULL, 0);
            g_message("[goku_gps_chnl_msg_proc],TPM_PDP_DEACTIVATE_CONFORM,uResult=%u",uResult); 
        }
        break;
        case TPM_PDP_DEACTIVATE_INDICATION://网络发起去激活成功
        {
            UINT8 uResult = *(UINT8*)param;
            goku_service_post_message_to_self(GOKU_SERVICE(service), TPM_GPS_AGPS_OK, NULL, 0);
            g_message("[goku_gps_chnl_msg_proc],TPM_PDP_DEACTIVATE_INDICATION,uResult=%u",uResult); 
        }
        break;
*/
	default:
		break;
	}
    
	return result;
}

static void goku_gps_service_get_during_time(GokuGpsServicePrivate *priv,gconstpointer data)
{
    GPSInfo *info = (GPSInfo *)data;
    float sec = (priv->end_tick - priv->start_tick)/100.0;

    info->loc_min = sec/60;
    info->loc_sec = (sec - info->loc_min*60);
#ifndef WIN32
	g_message("[%s]:%d\t priv->min=%d\n",__func__,__LINE__,info->loc_min);
    g_message("[%s]:%d\t priv->sec=%.2f\n",__func__,__LINE__,info->loc_sec);
#endif
}
/*------------------------------ End of file----------------------------------*/
