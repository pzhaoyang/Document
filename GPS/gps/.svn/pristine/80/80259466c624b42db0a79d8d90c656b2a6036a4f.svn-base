/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_command.h>
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
#ifndef __GOKU_GPS_COMMAND_H__
#define __GOKU_GPS_COMMAND_H__

/*----------------- include files --------------------------------------------*/
#include "puremvc-gobject.h"


G_BEGIN_DECLS

/*----------------- macro definition -----------------------------------------*/
#define GOKU_TYPE_GPS_COMMAND                  (goku_gps_command_get_type())
#define GOKU_GPS_COMMAND(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), GOKU_TYPE_GPS_COMMAND, GokuGpsCommand))
#define GOKU_GPS_COMMAND_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass), GOKU_TYPE_GPS_COMMAND, GokuGpsCommandClass))
#define GOKU_IS_GPS_COMMAND(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOKU_TYPE_GPS_COMMAND))
#define GOKU_IS_GPS_COMMAND_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass), GOKU_TYPE_GPS_COMMAND))
#define GOKU_GPS_COMMAND_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj), GOKU_TYPE_GPS_COMMAND, GokuGpsCommandClass))



/*----------------- constant and type definition -----------------------------*/
typedef struct _GokuGpsCommand        GokuGpsCommand;
typedef struct _GokuGpsCommandClass   GokuGpsCommandClass;

struct _GokuGpsCommand
{
    GokuSimpleCommand parent;
    
};

struct _GokuGpsCommandClass
{
    GokuSimpleCommandClass parent_class;
};

/*----------------- function prototype declaration ---------------------------*/
GType goku_gps_command_get_type();


G_END_DECLS

#endif