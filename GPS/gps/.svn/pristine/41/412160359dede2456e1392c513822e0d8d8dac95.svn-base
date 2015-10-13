/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps.c>
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
#include "goku_gps.h"
#include "goku_gps_facade.h"
#include "activity/goku_gps_main_activity.h"

/*----------------- file-local macro definition ------------------------------*/


/*----------------- file-local constant and type definition ------------------*/


/*----------------- file-local variables definition --------------------------*/


/*----------------- file-local function prototype declaration ----------------*/

#ifdef GOKU_DYNAMIC_APP_SWITCH
#ifndef WIN32
#include "os_dcm_frame.h"
DCM_EXPORT_SYMBOL_BEGIN
DCM_EXPORT_SYMBOL(goku_dapp_get_type)
DCM_EXPORT_SYMBOL_END
#endif
#endif


static void goku_gps_finalize(GObject *object);
static GType goku_gps_get_facade_type(GokuApp *app);
static void goku_gps_on_activity_type_reg(GokuApp *app);
static gboolean goku_gps_on_start(GokuApp *app);
static gboolean goku_gps_on_stop(GokuApp *app);

G_DEFINE_TYPE(GokuGps, goku_gps, GOKU_TYPE_APP)

/*----------------- function definition --------------------------------------*/
static void goku_gps_class_init(GokuGpsClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = goku_gps_finalize;
    GOKU_APP_CLASS(klass)->get_facade_type = goku_gps_get_facade_type;
    GOKU_APP_CLASS(klass)->on_activity_type_reg = goku_gps_on_activity_type_reg;
    GOKU_APP_CLASS(klass)->on_start = goku_gps_on_start;
    GOKU_APP_CLASS(klass)->on_stop = goku_gps_on_stop;
}

static void goku_gps_init(GokuGps *self)
{

}


static void goku_gps_finalize(GObject *object)
{
    // TODO:

    G_OBJECT_CLASS(goku_gps_parent_class)->finalize(object);
}

static GType goku_gps_get_facade_type(GokuApp *app)
{
    //
    // 应用的facade类型
    //

    return GOKU_TYPE_GPS_FACADE;
}

static void goku_gps_on_activity_type_reg(GokuApp *app)
{
    //
    // 在此注册所有的Activity类型
    //

    GOKU_TYPE_GPS_MAIN_ACTIVITY;
}

static gboolean goku_gps_on_start(GokuApp *app)
{
    //
    // 应用初始化
    //
	return TRUE;
}

static gboolean goku_gps_on_stop(GokuApp *app)
{
    //
    // 应用退出
    //
	return TRUE;
}

#ifdef GOKU_DYNAMIC_APP_SWITCH
GType goku_dapp_get_type()
{
	return GOKU_TYPE_GPS;
}
#endif

/*------------------------------ End of file----------------------------------*/