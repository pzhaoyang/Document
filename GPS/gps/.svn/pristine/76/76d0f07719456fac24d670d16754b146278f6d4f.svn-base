/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps.h>
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
#ifndef __GOKU_GPS_H__
#define __GOKU_GPS_H__

/*----------------- include files --------------------------------------------*/
#include "glib-object.h"
#include "goku_app.h"


G_BEGIN_DECLS

/*----------------- macro definition -----------------------------------------*/
#define GOKU_TYPE_GPS                  (goku_gps_get_type())
#define GOKU_GPS(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), GOKU_TYPE_GPS, GokuGps))
#define GOKU_GPS_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass), GOKU_TYPE_GPS, GokuGpsClass))
#define GOKU_IS_GPS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOKU_TYPE_GPS))
#define GOKU_IS_GPS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), GOKU_TYPE_GPS))
#define GOKU_GPS_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj), GOKU_TYPE_GPS, GokuGpsClass))



/*----------------- constant and type definition -----------------------------*/
typedef struct _GokuGps        GokuGps;
typedef struct _GokuGpsClass   GokuGpsClass;

struct _GokuGps
{
    GokuApp parent;
    
};

struct _GokuGpsClass
{
    GokuAppClass parent_class;
};

/*----------------- function prototype declaration ---------------------------*/
GType goku_gps_get_type();


#ifdef GOKU_DYNAMIC_APP_SWITCH
#ifdef WIN32
_declspec(dllexport) GType goku_dapp_get_type();
#else
GType goku_dapp_get_type();
#endif
#endif

G_END_DECLS

#endif