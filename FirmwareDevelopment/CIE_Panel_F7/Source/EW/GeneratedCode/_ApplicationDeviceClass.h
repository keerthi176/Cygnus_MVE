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

#ifndef _ApplicationDeviceClass_H
#define _ApplicationDeviceClass_H

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

/* Forward declaration of the class Application::DeviceClass */
#ifndef _ApplicationDeviceClass_
  EW_DECLARE_CLASS( ApplicationDeviceClass )
#define _ApplicationDeviceClass_
#endif


/* Deklaration of class : 'Application::DeviceClass' */
EW_DEFINE_FIELDS( ApplicationDeviceClass, TemplatesDeviceClass )
  EW_PROPERTY( Property,        XInt32 )
  EW_OBJECT  ( MeshListChangeEvent, CoreSystemEvent )
  EW_OBJECT  ( SiteLoadedEvent, CoreSystemEvent )
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
EW_END_OF_FIELDS( ApplicationDeviceClass )

/* Virtual Method Table (VMT) for the class : 'Application::DeviceClass' */
EW_DEFINE_METHODS( ApplicationDeviceClass, TemplatesDeviceClass )
EW_END_OF_METHODS( ApplicationDeviceClass )

/* 'C' function for method : 'Application::DeviceClass.OnSetProperty()' */
void ApplicationDeviceClass_OnSetProperty( ApplicationDeviceClass _this, XInt32 
  value );

/* This method is intended to be called by the device to notify the GUI application 
   about an alternation of its setting or state value. */
void ApplicationDeviceClass_UpdateProperty( ApplicationDeviceClass _this, XInt32 
  aNewValue );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.UpdateProperty()' */
void ApplicationDeviceClass__UpdateProperty( void* _this, XInt32 aNewValue );

/* The following define announces the presence of the method Application::DeviceClass.UpdateProperty(). */
#define _ApplicationDeviceClass__UpdateProperty_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerMeshListChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerMeshListChangeEvent()' */
void ApplicationDeviceClass__TriggerMeshListChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerMeshListChangeEvent(). */
#define _ApplicationDeviceClass__TriggerMeshListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerSiteLoadedEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerSiteLoadedEvent()' */
void ApplicationDeviceClass__TriggerSiteLoadedEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerSiteLoadedEvent(). */
#define _ApplicationDeviceClass__TriggerSiteLoadedEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerMeshStatusChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerMeshStatusChangeEvent()' */
void ApplicationDeviceClass__TriggerMeshStatusChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerMeshStatusChangeEvent(). */
#define _ApplicationDeviceClass__TriggerMeshStatusChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerFireEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerFireEvent()' */
void ApplicationDeviceClass__TriggerFireEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerFireEvent(). */
#define _ApplicationDeviceClass__TriggerFireEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerFirstAidEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerFirstAidEvent()' */
void ApplicationDeviceClass__TriggerFirstAidEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerFirstAidEvent(). */
#define _ApplicationDeviceClass__TriggerFirstAidEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerResetEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerResetEvent()' */
void ApplicationDeviceClass__TriggerResetEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerResetEvent(). */
#define _ApplicationDeviceClass__TriggerResetEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerEvacuateEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerEvacuateEvent()' */
void ApplicationDeviceClass__TriggerEvacuateEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerEvacuateEvent(). */
#define _ApplicationDeviceClass__TriggerEvacuateEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerSecurityEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerSecurityEvent()' */
void ApplicationDeviceClass__TriggerSecurityEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerSecurityEvent(). */
#define _ApplicationDeviceClass__TriggerSecurityEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerDisableEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerDisableEvent()' */
void ApplicationDeviceClass__TriggerDisableEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerDisableEvent(). */
#define _ApplicationDeviceClass__TriggerDisableEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerTestEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerTestEvent()' */
void ApplicationDeviceClass__TriggerTestEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerTestEvent(). */
#define _ApplicationDeviceClass__TriggerTestEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerFaultEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerFaultEvent()' */
void ApplicationDeviceClass__TriggerFaultEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerFaultEvent(). */
#define _ApplicationDeviceClass__TriggerFaultEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerFireListChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerFireListChangeEvent()' */
void ApplicationDeviceClass__TriggerFireListChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerFireListChangeEvent(). */
#define _ApplicationDeviceClass__TriggerFireListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerExportProgressEvent( ApplicationDeviceClass _this, 
  XInt32 amount );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerExportProgressEvent()' */
void ApplicationDeviceClass__TriggerExportProgressEvent( void* _this, XInt32 amount );

/* The following define announces the presence of the method Application::DeviceClass.TriggerExportProgressEvent(). */
#define _ApplicationDeviceClass__TriggerExportProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerStartProgressEvent( ApplicationDeviceClass _this, 
  XInt32 amount );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerStartProgressEvent()' */
void ApplicationDeviceClass__TriggerStartProgressEvent( void* _this, XInt32 amount );

/* The following define announces the presence of the method Application::DeviceClass.TriggerStartProgressEvent(). */
#define _ApplicationDeviceClass__TriggerStartProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerEndProgressEvent( ApplicationDeviceClass _this, 
  XInt32 amount );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerEndProgressEvent()' */
void ApplicationDeviceClass__TriggerEndProgressEvent( void* _this, XInt32 amount );

/* The following define announces the presence of the method Application::DeviceClass.TriggerEndProgressEvent(). */
#define _ApplicationDeviceClass__TriggerEndProgressEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerOnTestChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerOnTestChangeEvent()' */
void ApplicationDeviceClass__TriggerOnTestChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerOnTestChangeEvent(). */
#define _ApplicationDeviceClass__TriggerOnTestChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerFaultListChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerFaultListChangeEvent()' */
void ApplicationDeviceClass__TriggerFaultListChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerFaultListChangeEvent(). */
#define _ApplicationDeviceClass__TriggerFaultListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerEventListChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerEventListChangeEvent()' */
void ApplicationDeviceClass__TriggerEventListChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerEventListChangeEvent(). */
#define _ApplicationDeviceClass__TriggerEventListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerLogListChangeEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerLogListChangeEvent()' */
void ApplicationDeviceClass__TriggerLogListChangeEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerLogListChangeEvent(). */
#define _ApplicationDeviceClass__TriggerLogListChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerAccessModeChangeEvent( ApplicationDeviceClass _this, 
  XInt32 level );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerAccessModeChangeEvent()' */
void ApplicationDeviceClass__TriggerAccessModeChangeEvent( void* _this, XInt32 level );

/* The following define announces the presence of the method Application::DeviceClass.TriggerAccessModeChangeEvent(). */
#define _ApplicationDeviceClass__TriggerAccessModeChangeEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerWriteProtectedEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerWriteProtectedEvent()' */
void ApplicationDeviceClass__TriggerWriteProtectedEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerWriteProtectedEvent(). */
#define _ApplicationDeviceClass__TriggerWriteProtectedEvent_

/* This method is intended to be called by the device to notify the GUI application 
   about a particular system event. */
void ApplicationDeviceClass_TriggerDFUEvent( ApplicationDeviceClass _this );

/* Wrapper function for the non virtual method : 'Application::DeviceClass.TriggerDFUEvent()' */
void ApplicationDeviceClass__TriggerDFUEvent( void* _this );

/* The following define announces the presence of the method Application::DeviceClass.TriggerDFUEvent(). */
#define _ApplicationDeviceClass__TriggerDFUEvent_

/* Default onget method for the property 'Property' */
XInt32 ApplicationDeviceClass_OnGetProperty( ApplicationDeviceClass _this );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationDeviceClass_H */

/* Embedded Wizard */
