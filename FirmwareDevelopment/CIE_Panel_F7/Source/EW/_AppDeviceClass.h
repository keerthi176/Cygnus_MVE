/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This file was generated automatically by Embedded Wizard Studio.
*
* Please do not make any modifications of this file! The modifications are lost
* when the file is generated again by Embedded Wizard Studio!
*
* The template of this heading text can be found in the file 'head.ewt' in the
* directory 'Platforms' of your Embedded Wizard installation directory. If you
* wish to adapt this text, please copy the template file 'head.ewt' into your
* project directory and edit the copy only. Please avoid any modifications of
* the original template file!
*
* Version  : 9.00
* Profile  : STM32F746
* Platform : STM.STM32.RGB565
*
*******************************************************************************/

#ifndef _AppDeviceClass_H
#define _AppDeviceClass_H

#ifdef __cplusplus
  extern "C"
  {
#endif

#include "ewrte.h"
#if EW_RTE_VERSION != 0x00090000
  #error Wrong version of Embedded Wizard Runtime Environment.
#endif

#include "ewgfx.h"
#if EW_GFX_VERSION != 0x00090000
  #error Wrong version of Embedded Wizard Graphics Engine.
#endif

#include "_CoreSystemEvent.h"
#include "_TemplatesDeviceClass.h"

/* Forward declaration of the class App::DeviceClass */
#ifndef _AppDeviceClass_
  EW_DECLARE_CLASS( AppDeviceClass )
#define _AppDeviceClass_
#endif


/* Deklaration of class : 'App::DeviceClass' */
EW_DEFINE_FIELDS( AppDeviceClass, TemplatesDeviceClass )
  EW_PROPERTY( Property,        XInt32 )
  EW_OBJECT  ( MeshListChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( SiteChangedEvent, CoreSystemEvent )
  EW_OBJECT  ( MeshStatusChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( FireEvent,       CoreSystemEvent )
  EW_OBJECT  ( FirstAidEvent,   CoreSystemEvent )
  EW_OBJECT  ( ResetEvent,      CoreSystemEvent )
  EW_OBJECT  ( EvacuateEvent,   CoreSystemEvent )
  EW_OBJECT  ( SecurityEvent,   CoreSystemEvent )
  EW_OBJECT  ( DisableEvent,    CoreSystemEvent )
  EW_OBJECT  ( TestEvent,       CoreSystemEvent )
  EW_OBJECT  ( FaultEvent,      CoreSystemEvent )
  EW_OBJECT  ( FireListChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( ExportProgressEvent, CoreSystemEvent )
  EW_OBJECT  ( StartProgressEvent, CoreSystemEvent )
  EW_OBJECT  ( EndProgressEvent, CoreSystemEvent )
  EW_OBJECT  ( OnTestChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( FaultListChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( EventListChangedEvent, CoreSystemEvent )
  EW_OBJECT  ( LogListChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( AccessModeChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( WriteProtectedEvent, CoreSystemEvent )
  EW_OBJECT  ( DFUEvent,        CoreSystemEvent )
  EW_OBJECT  ( GotoFireDisplay, CoreSystemEvent )
  EW_OBJECT  ( ConfirmFireAvailable, CoreSystemEvent )
  EW_OBJECT  ( AcknowledgeFireAvailable, CoreSystemEvent )
  EW_OBJECT  ( TimeChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( ConfirmEventAvailable, CoreSystemEvent )
  EW_OBJECT  ( AcknowledgeEventAvailable, CoreSystemEvent )
  EW_OBJECT  ( SafeStateEvent,  CoreSystemEvent )
  EW_OBJECT  ( RBUDetectedEvent, CoreSystemEvent )
  EW_OBJECT  ( RBUProgressEvent, CoreSystemEvent )
  EW_OBJECT  ( DisablementChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( NewZoneAndCombi, CoreSystemEvent )
  EW_OBJECT  ( NewBitmapLogoEvent, CoreSystemEvent )
  EW_OBJECT  ( AcumeshEvent,    CoreSystemEvent )
  EW_OBJECT  ( ResoudStateEvent, CoreSystemEvent )
  EW_OBJECT  ( UpdateDeviceEvent, CoreSystemEvent )
  EW_OBJECT  ( MeshUnitChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( GotoSysPageEvent, CoreSystemEvent )
EW_END_OF_FIELDS( AppDeviceClass )

/* Virtual Method Table (VMT) for the class : 'App::DeviceClass' */
EW_DEFINE_METHODS( AppDeviceClass, TemplatesDeviceClass )
EW_END_OF_METHODS( AppDeviceClass )

/* 'C' function for method : 'App::DeviceClass.OnSetProperty()' */
void AppDeviceClass_OnSetProperty( AppDeviceClass _this, XInt32 value );

/* This method is intended to be called by the device to notify the GUI application 
   about an alternation of its setting or state value. */
void AppDeviceClass_UpdateProperty( AppDeviceClass _this, XInt32 aNewValue );

/* Wrapper function for the non virtual method : 'App::DeviceClass.UpdateProperty()' */
void AppDeviceClass__UpdateProperty( void* _this, XInt32 aNewValue );

/* The following define announces the presence of the method App::DeviceClass.UpdateProperty(). */
#define _AppDeviceClass__UpdateProperty_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerMeshListChangeEvent( AppDeviceClass _this, XInt16 total, 
  XInt16 active, XInt16 meshed, XInt16 extra );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerMeshListChangeEvent()' */
void AppDeviceClass__TriggerMeshListChangeEvent( void* _this, XInt16 total, XInt16 
  active, XInt16 meshed, XInt16 extra );

/* The following define announces the presence of the method App::DeviceClass.TriggerMeshListChangeEvent(). */
#define _AppDeviceClass__TriggerMeshListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerSiteChangedEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerSiteChangedEvent()' */
void AppDeviceClass__TriggerSiteChangedEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerSiteChangedEvent(). */
#define _AppDeviceClass__TriggerSiteChangedEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerMeshStatusChangeEvent( AppDeviceClass _this, XInt16 wantedstage, 
  XInt16 currentstage );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerMeshStatusChangeEvent()' */
void AppDeviceClass__TriggerMeshStatusChangeEvent( void* _this, XInt16 wantedstage, 
  XInt16 currentstage );

/* The following define announces the presence of the method App::DeviceClass.TriggerMeshStatusChangeEvent(). */
#define _AppDeviceClass__TriggerMeshStatusChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerFireEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerFireEvent()' */
void AppDeviceClass__TriggerFireEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerFireEvent(). */
#define _AppDeviceClass__TriggerFireEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerFirstAidEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerFirstAidEvent()' */
void AppDeviceClass__TriggerFirstAidEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerFirstAidEvent(). */
#define _AppDeviceClass__TriggerFirstAidEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerResetEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerResetEvent()' */
void AppDeviceClass__TriggerResetEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerResetEvent(). */
#define _AppDeviceClass__TriggerResetEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerEvacuateEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerEvacuateEvent()' */
void AppDeviceClass__TriggerEvacuateEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerEvacuateEvent(). */
#define _AppDeviceClass__TriggerEvacuateEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerSecurityEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerSecurityEvent()' */
void AppDeviceClass__TriggerSecurityEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerSecurityEvent(). */
#define _AppDeviceClass__TriggerSecurityEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerDisableEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerDisableEvent()' */
void AppDeviceClass__TriggerDisableEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerDisableEvent(). */
#define _AppDeviceClass__TriggerDisableEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerTestEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerTestEvent()' */
void AppDeviceClass__TriggerTestEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerTestEvent(). */
#define _AppDeviceClass__TriggerTestEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerFaultEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerFaultEvent()' */
void AppDeviceClass__TriggerFaultEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerFaultEvent(). */
#define _AppDeviceClass__TriggerFaultEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerFireListChangeEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerFireListChangeEvent()' */
void AppDeviceClass__TriggerFireListChangeEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerFireListChangeEvent(). */
#define _AppDeviceClass__TriggerFireListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerExportProgressEvent( AppDeviceClass _this, XInt32 amount );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerExportProgressEvent()' */
void AppDeviceClass__TriggerExportProgressEvent( void* _this, XInt32 amount );

/* The following define announces the presence of the method App::DeviceClass.TriggerExportProgressEvent(). */
#define _AppDeviceClass__TriggerExportProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerStartProgressEvent( AppDeviceClass _this, XInt32 amount );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerStartProgressEvent()' */
void AppDeviceClass__TriggerStartProgressEvent( void* _this, XInt32 amount );

/* The following define announces the presence of the method App::DeviceClass.TriggerStartProgressEvent(). */
#define _AppDeviceClass__TriggerStartProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerEndProgressEvent( AppDeviceClass _this, XInt32 amount );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerEndProgressEvent()' */
void AppDeviceClass__TriggerEndProgressEvent( void* _this, XInt32 amount );

/* The following define announces the presence of the method App::DeviceClass.TriggerEndProgressEvent(). */
#define _AppDeviceClass__TriggerEndProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerOnTestChangeEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerOnTestChangeEvent()' */
void AppDeviceClass__TriggerOnTestChangeEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerOnTestChangeEvent(). */
#define _AppDeviceClass__TriggerOnTestChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerFaultListChangeEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerFaultListChangeEvent()' */
void AppDeviceClass__TriggerFaultListChangeEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerFaultListChangeEvent(). */
#define _AppDeviceClass__TriggerFaultListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerEventListChangeEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerEventListChangeEvent()' */
void AppDeviceClass__TriggerEventListChangeEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerEventListChangeEvent(). */
#define _AppDeviceClass__TriggerEventListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerLogListChangeEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerLogListChangeEvent()' */
void AppDeviceClass__TriggerLogListChangeEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerLogListChangeEvent(). */
#define _AppDeviceClass__TriggerLogListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerAccessModeChangeEvent( AppDeviceClass _this, XInt32 level );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerAccessModeChangeEvent()' */
void AppDeviceClass__TriggerAccessModeChangeEvent( void* _this, XInt32 level );

/* The following define announces the presence of the method App::DeviceClass.TriggerAccessModeChangeEvent(). */
#define _AppDeviceClass__TriggerAccessModeChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerWriteProtectedEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerWriteProtectedEvent()' */
void AppDeviceClass__TriggerWriteProtectedEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerWriteProtectedEvent(). */
#define _AppDeviceClass__TriggerWriteProtectedEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerDFUEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerDFUEvent()' */
void AppDeviceClass__TriggerDFUEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerDFUEvent(). */
#define _AppDeviceClass__TriggerDFUEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerGotoFireDisplay( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerGotoFireDisplay()' */
void AppDeviceClass__TriggerGotoFireDisplay( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerGotoFireDisplay(). */
#define _AppDeviceClass__TriggerGotoFireDisplay_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerConfirmFireAvailable( AppDeviceClass _this, XBool on );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerConfirmFireAvailable()' */
void AppDeviceClass__TriggerConfirmFireAvailable( void* _this, XBool on );

/* The following define announces the presence of the method App::DeviceClass.TriggerConfirmFireAvailable(). */
#define _AppDeviceClass__TriggerConfirmFireAvailable_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerAcknowledgeFireAvailable( AppDeviceClass _this, XBool 
  on );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerAcknowledgeFireAvailable()' */
void AppDeviceClass__TriggerAcknowledgeFireAvailable( void* _this, XBool on );

/* The following define announces the presence of the method App::DeviceClass.TriggerAcknowledgeFireAvailable(). */
#define _AppDeviceClass__TriggerAcknowledgeFireAvailable_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerTimeChangeEvent( AppDeviceClass _this, XBool daytime );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerTimeChangeEvent()' */
void AppDeviceClass__TriggerTimeChangeEvent( void* _this, XBool daytime );

/* The following define announces the presence of the method App::DeviceClass.TriggerTimeChangeEvent(). */
#define _AppDeviceClass__TriggerTimeChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerConfirmEventAvailable( AppDeviceClass _this, XBool on );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerConfirmEventAvailable()' */
void AppDeviceClass__TriggerConfirmEventAvailable( void* _this, XBool on );

/* The following define announces the presence of the method App::DeviceClass.TriggerConfirmEventAvailable(). */
#define _AppDeviceClass__TriggerConfirmEventAvailable_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerAcknowledgeEventAvailable( AppDeviceClass _this, XBool 
  on );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerAcknowledgeEventAvailable()' */
void AppDeviceClass__TriggerAcknowledgeEventAvailable( void* _this, XBool on );

/* The following define announces the presence of the method App::DeviceClass.TriggerAcknowledgeEventAvailable(). */
#define _AppDeviceClass__TriggerAcknowledgeEventAvailable_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerSafeStateEvent( AppDeviceClass _this, XBool on );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerSafeStateEvent()' */
void AppDeviceClass__TriggerSafeStateEvent( void* _this, XBool on );

/* The following define announces the presence of the method App::DeviceClass.TriggerSafeStateEvent(). */
#define _AppDeviceClass__TriggerSafeStateEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerRBUDetectedEvent( AppDeviceClass _this, XInt32 device_ptr );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerRBUDetectedEvent()' */
void AppDeviceClass__TriggerRBUDetectedEvent( void* _this, XInt32 device_ptr );

/* The following define announces the presence of the method App::DeviceClass.TriggerRBUDetectedEvent(). */
#define _AppDeviceClass__TriggerRBUDetectedEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerRBUProgressEvent( AppDeviceClass _this, XInt32 progress, 
  XInt32 char_ptr );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerRBUProgressEvent()' */
void AppDeviceClass__TriggerRBUProgressEvent( void* _this, XInt32 progress, XInt32 
  char_ptr );

/* The following define announces the presence of the method App::DeviceClass.TriggerRBUProgressEvent(). */
#define _AppDeviceClass__TriggerRBUProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerDisablementChangeEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerDisablementChangeEvent()' */
void AppDeviceClass__TriggerDisablementChangeEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerDisablementChangeEvent(). */
#define _AppDeviceClass__TriggerDisablementChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerNewZoneAndCombi( AppDeviceClass _this, XChar zone, XChar 
  combination );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerNewZoneAndCombi()' */
void AppDeviceClass__TriggerNewZoneAndCombi( void* _this, XChar zone, XChar combination );

/* The following define announces the presence of the method App::DeviceClass.TriggerNewZoneAndCombi(). */
#define _AppDeviceClass__TriggerNewZoneAndCombi_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerNewBitmapLogoEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerNewBitmapLogoEvent()' */
void AppDeviceClass__TriggerNewBitmapLogoEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerNewBitmapLogoEvent(). */
#define _AppDeviceClass__TriggerNewBitmapLogoEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerAcumeshEvent( AppDeviceClass _this, XInt32 stage );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerAcumeshEvent()' */
void AppDeviceClass__TriggerAcumeshEvent( void* _this, XInt32 stage );

/* The following define announces the presence of the method App::DeviceClass.TriggerAcumeshEvent(). */
#define _AppDeviceClass__TriggerAcumeshEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerResoundStateEvent( AppDeviceClass _this, XBool enabled );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerResoundStateEvent()' */
void AppDeviceClass__TriggerResoundStateEvent( void* _this, XBool enabled );

/* The following define announces the presence of the method App::DeviceClass.TriggerResoundStateEvent(). */
#define _AppDeviceClass__TriggerResoundStateEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerUpdateDeviceEvent( AppDeviceClass _this, XInt32 val );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerUpdateDeviceEvent()' */
void AppDeviceClass__TriggerUpdateDeviceEvent( void* _this, XInt32 val );

/* The following define announces the presence of the method App::DeviceClass.TriggerUpdateDeviceEvent(). */
#define _AppDeviceClass__TriggerUpdateDeviceEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerMeshUnitChangeEvent( AppDeviceClass _this, XInt16 unit_num );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerMeshUnitChangeEvent()' */
void AppDeviceClass__TriggerMeshUnitChangeEvent( void* _this, XInt16 unit_num );

/* The following define announces the presence of the method App::DeviceClass.TriggerMeshUnitChangeEvent(). */
#define _AppDeviceClass__TriggerMeshUnitChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void AppDeviceClass_TriggerGotoSysPageEvent( AppDeviceClass _this );

/* Wrapper function for the non virtual method : 'App::DeviceClass.TriggerGotoSysPageEvent()' */
void AppDeviceClass__TriggerGotoSysPageEvent( void* _this );

/* The following define announces the presence of the method App::DeviceClass.TriggerGotoSysPageEvent(). */
#define _AppDeviceClass__TriggerGotoSysPageEvent_

/* Default onget method for the property 'Property' */
XInt32 AppDeviceClass_OnGetProperty( AppDeviceClass _this );

#ifdef __cplusplus
  }
#endif

#endif /* _AppDeviceClass_H */

/* Embedded Wizard */
