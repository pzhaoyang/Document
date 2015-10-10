/*******************************************************************************
Copyright(c) 2012 - 2013 Unipro CO.,LTD.
All Rights Reserved. By using this module you agree to the terms of the
Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_service.h>
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
#ifndef __GOKU_GPS_SERVICE_H__
#define __GOKU_GPS_SERVICE_H__

/*----------------- include files --------------------------------------------*/
#include "goku_include.h"
#include "goku_setting_iface.h"

G_BEGIN_DECLS

/*----------------- macro definition -----------------------------------------*/
#define GOKU_TYPE_GPS_SERVICE                  (goku_gps_service_get_type())
#define GOKU_GPS_SERVICE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), GOKU_TYPE_GPS_SERVICE, GokuGpsService))
#define GOKU_GPS_SERVICE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass), GOKU_TYPE_GPS_SERVICE, GokuGpsServiceClass))
#define GOKU_IS_GPS_SERVICE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOKU_TYPE_GPS_SERVICE))
#define GOKU_IS_GPS_SERVICE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), GOKU_TYPE_GPS_SERVICE))
#define GOKU_GPS_SERVICE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj), GOKU_TYPE_GPS_SERVICE, GokuGpsServiceClass))

#define GOKU_GPS_SERVICE_NAME                  "goku_gps_service"

/*----------------- constant and type definition -----------------------------*/
typedef struct _GokuGpsService       GokuGpsService;
typedef struct _GokuGpsServiceClass  GokuGpsServiceClass;

struct _GokuGpsService
{
	GokuService parent;
};

struct _GokuGpsServiceClass
{
	GokuServiceClass parent_class;
};

/*----------------- function prototype declaration ---------------------------*/
GType goku_gps_service_get_type();

GokuGpsService* goku_gps_service_new(void);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
G_END_DECLS

#endif	//__GOKU_GPS_SERVICE_H__