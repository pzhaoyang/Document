/*******************************************************************************
    Copyright(c) 2010 - 2011 Leadcore Technology CO.,LTD.
    All Rights Reserved. By using this module you agree to the terms of the
    Leadcore Technology CO.,LTD License Agreement for it.
********************************************************************************
/******************************************************************************
* Filename    : <goku_gps_command.c>
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
#include "goku_gps_command.h"

/*----------------- file-local macro definition ------------------------------*/


/*----------------- file-local constant and type definition ------------------*/


/*----------------- file-local variables definition --------------------------*/


/*----------------- file-local function prototype declaration ----------------*/

static void goku_gps_command_execute(GokuSimpleCommand *icommand, GokuINotification *inotification);

G_DEFINE_TYPE(GokuGpsCommand, goku_gps_command, GOKU_TYPE_SIMPLE_COMMAND)

/*----------------- function definition --------------------------------------*/
static void goku_gps_command_class_init(GokuGpsCommandClass *klass)
{
    GOKU_SIMPLE_COMMAND_CLASS(klass)->execute = goku_gps_command_execute;
}

static void goku_gps_command_init(GokuGpsCommand *self) {}


static void goku_gps_command_execute(GokuSimpleCommand *icommand, GokuINotification *inotification)
{
    // TODO:

    const gchar *body = (const gchar *)goku_inotification_get_body(inotification);

    g_message("goku_gps_command_execute  notification body(%s)", body);

}


/*------------------------------ End of file----------------------------------*/