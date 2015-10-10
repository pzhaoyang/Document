###########################################################################
#    Copyright(c) 2008 - 2009 Leadcore Technology CO.,LTD.
#    All Rights Reserved. By using this module you agree to the terms of the
#    Leadcore Technology CO.,LTD License Agreement for it.
###########################################################################
# Filename		: gps.mk
#
# Description	: makefile for gps
#
# Notes				: 
#
#--------------------------------------------------------------------------------
# Change History: 
#--------------------------------------------------------------------------------
#          
# 2013-8-9, pengzhaoyang, create originally.
# 
#        
# 
#        
##########################################################################

ENVDIR = ../../../ws
include	$(ENVDIR)/makeenv.mk





APP_OBJDIR = ../lib
APP_SRCDIR = ..



#please add files by character asending sequnce
APP_COBJECTS = \
               $(APP_OBJDIR)/goku_gps.o \
               $(APP_OBJDIR)/goku_gps_facade.o \
               $(APP_OBJDIR)/activity/goku_gps_main_activity.o \
               $(APP_OBJDIR)/controller/goku_gps_command.o \
               $(APP_OBJDIR)/model/goku_gps_service.o \
               $(APP_OBJDIR)/gps_server/goku_gps_main.o \
               $(APP_OBJDIR)/gps_server/goku_gps_protocol_manager.o \
               $(APP_OBJDIR)/gps_server/goku_gps_socket.o \

APP_OBJS = $(APP_COBJECTS) $(APP_SOBJECTS)

#APP_INSTALL_LIBDIR = $(APP_OBJDIR)
APP_INSTALL_LIBDIR = ../../lib
APP_LOCAL_INCLUDE  = -I../ -I../activity
APP_LOCAL_ASFLAGS =
APP_LOCAL_CFLAGS  =

APP_TARGET = gps

all: prepare $(APP_COBJECTS) $(APP_SOBJECTS)

prepare:
	-mkdir $(subst /,\,$(APP_APPLICATIONS_HOME)/$(APP_TARGET)/lib/activity)
	-mkdir $(subst /,\,$(APP_APPLICATIONS_HOME)/$(APP_TARGET)/lib/controller)
	-mkdir $(subst /,\,$(APP_APPLICATIONS_HOME)/$(APP_TARGET)/lib/model)
	-mkdir $(subst /,\,$(APP_APPLICATIONS_HOME)/$(APP_TARGET)/lib/gps_server)

$(APP_COBJECTS): $(APP_OBJDIR)/%.o: $(APP_SRCDIR)/%.c
	$(APP_CC) -c $(APP_CFLAGS) $(APP_LOCAL_CFLAGS) $(APP_INCLUDES) $(APP_LOCAL_INCLUDE) $< -o $@

$(APP_SOBJECTS): $(APP_OBJDIR)/%.o: $(APP_SRCDIR)/%.s
	$(APP_AS) $(APP_ASFLAGS) $(APP_LOCAL_ASFLAGS) $(APP_INCLUDES) $(APP_LOCAL_INCLUDE) $< -o $@


include $(ENVDIR)/applib.mk
