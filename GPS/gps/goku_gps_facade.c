/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_facade.c>
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
#include "goku_gps_facade.h"
#include "controller/goku_gps_command.h"
#include "goku_gps_notifications.h"
#include "model/goku_gps_service.h"

/*----------------- file-local macro definition ------------------------------*/


/*----------------- file-local constant and type definition ------------------*/


/*----------------- file-local variables definition --------------------------*/


/*----------------- file-local function prototype declaration ----------------*/

static void goku_gps_facade_finalize(GObject *object);
static void goku_gps_facade_initialize_controller(GokuFacade *facade);
static void goku_gps_facade_initialize_model( GokuFacade *facade );

G_DEFINE_TYPE(GokuGpsFacade, goku_gps_facade, GOKU_TYPE_FACADE)

/*----------------- function definition --------------------------------------*/
static void goku_gps_facade_class_init(GokuGpsFacadeClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = goku_gps_facade_finalize;
    GOKU_FACADE_CLASS(klass)->initialize_controller = goku_gps_facade_initialize_controller;
	GOKU_FACADE_CLASS(klass)->initialize_model = goku_gps_facade_initialize_model;
}

static void goku_gps_facade_init(GokuGpsFacade *self)
{

}


static void goku_gps_facade_finalize(GObject *object)
{

    G_OBJECT_CLASS(goku_gps_facade_parent_class)->finalize(object);
}

static void goku_gps_facade_initialize_controller(GokuFacade *facade)
{
    GOKU_FACADE_CLASS(goku_gps_facade_parent_class)->initialize_controller(facade);

    //
    // 注册notification对应的Command类型
    // 当调用goku_mvc_send_notification(const gchar *notification)接口时，对应的Command类的execute会被执行
    //

    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_GPS_FIX_OK, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_GPS_FIX_FAIL, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_GPS_FIX_STATUS, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_AGPS_SUCCESS, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_AGPS_FAIL, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_SAT_STATUS, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_GPS_CHECK_FAIL, GOKU_TYPE_GPS_COMMAND);
    goku_ifacade_register_command(GOKU_IFACADE(facade), NOTIFY_GPS_CHECK_OK, GOKU_TYPE_GPS_COMMAND);

}

static void goku_gps_facade_initialize_model( GokuFacade *facade )
{
    GokuService *service = NULL;
    GokuIFacade *ifacade = GOKU_IFACADE(facade);
    g_message("goku_gps_facade_initialize_model begin");

	GOKU_FACADE_CLASS(goku_gps_facade_parent_class)->initialize_model(facade);

	g_message("gps_service begin");
    service = (GokuService *)goku_gps_service_new();
    if (NULL != service)
    {
        g_message("gps_service");
        goku_ifacade_register_proxy(ifacade, (GokuIProxy *)service);
        goku_service_start(service);
        g_object_unref(service);
    }
    g_message("gps_service end");
	
    g_message("goku_gps_facade_initialize_model end");
}

/*------------------------------ End of file----------------------------------*/
