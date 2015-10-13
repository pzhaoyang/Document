/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_main_activity.h>
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
#ifndef __GOKU_GPS_MAIN_ACTIVITY_H__
#define __GOKU_GPS_MAIN_ACTIVITY_H__

/*----------------- include files --------------------------------------------*/
#include "goku_activity.h"


G_BEGIN_DECLS

/*----------------- macro definition -----------------------------------------*/
#define GOKU_TYPE_GPS_MAIN_ACTIVITY                  (goku_gps_main_activity_get_type())
#define GOKU_GPS_MAIN_ACTIVITY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), GOKU_TYPE_GPS_MAIN_ACTIVITY, GokuGpsMainActivity))
#define GOKU_GPS_MAIN_ACTIVITY_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass), GOKU_TYPE_GPS_MAIN_ACTIVITY, GokuGpsMainActivityClass))
#define GOKU_IS_GPS_MAIN_ACTIVITY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOKU_TYPE_GPS_MAIN_ACTIVITY))
#define GOKU_IS_GPS_MAIN_ACTIVITY_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), GOKU_TYPE_GPS_MAIN_ACTIVITY))
#define GOKU_GPS_MAIN_ACTIVITY_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj), GOKU_TYPE_GPS_MAIN_ACTIVITY, GokuGpsMainActivityClass))



/*----------------- constant and type definition -----------------------------*/
typedef struct _GokuGpsMainActivity        GokuGpsMainActivity;
typedef struct _GokuGpsMainActivityClass   GokuGpsMainActivityClass;
typedef struct _GokuGpsMainActivityPrivate GokuGpsMainActivityPrivate;

struct _GokuGpsMainActivity
{
    GokuActivity parent;
    GokuGpsMainActivityPrivate *priv;
};

struct _GokuGpsMainActivityClass
{
    GokuActivityClass parent_class;
};

/*----------------- function prototype declaration ---------------------------*/
GType goku_gps_main_activity_get_type();


G_END_DECLS

#endif