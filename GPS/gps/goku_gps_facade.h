/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_facade.h>
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
#ifndef __GOKU_GPS_FACADE_H__
#define __GOKU_GPS_FACADE_H__

/*----------------- include files --------------------------------------------*/
#include "puremvc-gobject.h"


G_BEGIN_DECLS

/*----------------- macro definition -----------------------------------------*/
#define GOKU_TYPE_GPS_FACADE                  (goku_gps_facade_get_type())
#define GOKU_GPS_FACADE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), GOKU_TYPE_GPS_FACADE, GokuGpsFacade))
#define GOKU_GPS_FACADE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass), GOKU_TYPE_GPS_FACADE, GokuGpsFacadeClass))
#define GOKU_IS_GPS_FACADE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOKU_TYPE_GPS_FACADE))
#define GOKU_IS_GPS_FACADE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), GOKU_TYPE_GPS_FACADE))
#define GOKU_GPS_FACADE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj), GOKU_TYPE_GPS_FACADE, GokuGpsFacadeClass))



/*----------------- constant and type definition -----------------------------*/
typedef struct _GokuGpsFacade        GokuGpsFacade;
typedef struct _GokuGpsFacadeClass   GokuGpsFacadeClass;

struct _GokuGpsFacade
{
    GokuFacade parent;
    
};

struct _GokuGpsFacadeClass
{
    GokuFacadeClass parent_class;
};

/*----------------- function prototype declaration ---------------------------*/
GType goku_gps_facade_get_type();


G_END_DECLS

#endif