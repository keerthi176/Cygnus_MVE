/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software is delivered "as is" and shows the usage of other software
* components. It is provided as an example software which is intended to be
* modified and extended according to particular requirements.
*
* TARA Systems hereby disclaims all warranties and conditions with regard to the
* software, including all implied warranties and conditions of merchantability
* and non-infringement of any third party IPR or other rights which may result
* from the use or the inability to use the software.
*
********************************************************************************
*
* DESCRIPTION:
*   This file implements an interface between an Embedded Wizard generated UI
*   application and a certain device. Please consider this file only as template
*   that is intended to show the binding between an UI application and the
*   underlying system (e.g. middleware, BSP, hardware driver, protocol, ...).
*
*   This device driver is the counterpart to a device class implemented within
*   your Embedded Wizard project.
*
*   Feel free to adapt this file according your needs!
*
*   Within this sample, we demonstrate the access to the board LED and the
*   hardware button. Furthemore, the serial interface is used to print a string.
*
*   This file assumes to be the counterpart of the device class 'DeviceClass'
*   within the unit 'Application'.
*   In order to ensure that this file can be compiled for all projects with or
*   without the device class, the generated define '_AppDeviceClass_'
*   is used.
*
*******************************************************************************/

/*
   Include all necessary files to access the real device and/or to get access
   to the required operating system calls.
*/
//#include "stm32f769i_eval.h"
#include "ewrte.h"
#include "ew_bsp_inout.h"

/*
   Include the generated header file to access the device class, for example to
   access the class 'DeviceClass' from the unit 'Application' include the
   generated file 'Application.h'.
*/


#include "Application.h"

#ifdef _AppDeviceClass_

/*
   Create a static variable to keep the global instance (autoobject) of the
   device class. The type of the variable has to correspond to the device class
   and the unit name, for example the static variable of the class 'DeviceClass'
   from the unit 'Application' has the type 'AppDeviceClass'.
*/
  static AppDeviceClass DeviceObject = 0;

 

#endif



/*******************************************************************************
* FUNCTION:
*   DeviceDriver_Initialize
*
* DESCRIPTION:
*   The function DeviceDriver_Initialize() initializes the module and prepares all
*   necessary things to access or communicate with the real device.
*   The function has to be called from your main module, after the initialization
*   of your GUI application.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void DeviceDriver_Initialize( void )
{
  /*
     You can implement here the necessary code to initialize your particular
     hardware, to open needed devices, to open communication channels, etc.
  */
 

#ifdef _AppDeviceClass_

  /*
     Get access to the counterpart of this device driver: get access to the
     device class that is created as autoobject within your Embedded Wizard
     project. For this purpose you can call the function EwGetAutoObject().
     This function contains two paramters: The pointer to the autoobject and
     the class of the autoobject.
     Assuming you have implemented the class 'DeviceClass' within the unit
     'Application' and you have an autoobject with the name 'Device', make
     the following call:
     EwGetAutoObject( &AppDevice, AppDeviceClass );
  */

  DeviceObject = EwGetAutoObject( &AppDevices, AppDeviceClass );
  
  

  /*
     Once you have the access to the autoobject, lock it as long as you need
     the access (typically, until your device driver is closed). By locking
     the autoobject, you can ensure that the object of the device class will
     be kept within the memory and not freed by the Garbage Collector.
     Once the device class is locked, you can easily store it within a static
     variable to access the driver class during the runtime.
  */

  EwLockObject( DeviceObject );
   

#endif

}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_Deinitialize
*
* DESCRIPTION:
*   The function DeviceDriver_Deinitialize() deinitializes the module and
*   finalizes the access or communication with the real device.
*   The function has to be called from your main module, before the GUI
*   application will be deinitialized.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void DeviceDriver_Deinitialize( void )
{
  /*
     You can implement here the necessary code to deinitialize your particular
     hardware, to close used devices, to close communication channels, etc.
  */

#ifdef _AppDeviceClass_

  /*
     If you have access to the device class autoobject, don't forget to unlock
     it. Clear the static variable to avoid later usage.
  */

  if ( DeviceObject )
    EwUnlockObject( DeviceObject );

  DeviceObject = 0;

#endif

}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_ProcessData
*
* DESCRIPTION:
*   The function DeviceDriver_ProcessData() is called from the main UI loop, in
*   order to process data and events from your particular device.
*   This function is responisble to update properties within the device class
*   if the corresponding state or value of the real device has changed.
*   This function is also responsible to trigger system events if necessary.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   The function returns a non-zero value if a property has changed or if a
*   system event was triggered. If nothing happens, the function returns 0.
*
*******************************************************************************/
int DeviceDriver_ProcessData( void )
{
  int needUpdate = 0;

  /*
     Get the data you want to provide to the GUI application.
     In case your are working with an operating system and your device is
     controlled from a separate task/thread/process, take all information
     from your device driver out of the message queue.
     Please note, that this function is called within the context of the main
     GUI thread.
     If you control your system by direct register access or some BSP functions,
     get all necessary data you want to provide to the GUI application.
  */

#ifdef _AppDeviceClass_

  /* here we just evaluate the current hardware button state */
 
   // AppDeviceClass__UpdateHardButtonCounter( DeviceObject, (XInt32)ButtonCounter );

 #endif

   
 

  /*
     Return a value != 0 if there is at least on property changed or if a
     system event was triggered. The return value is used by the main loop, to
     decide whether the GUI application has changed or not.
  */

  return needUpdate;
}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_SetLedStatus
*
* DESCRIPTION:
*   This is a sample for a function called from the device class, when a
*   property has changed. As a result, the corresponding value of the real
*   device should be changed.
*   In this implementation simply the LED is switched on or off.
*
*******************************************************************************/
void DeviceDriver_SetLedStatus( XInt32 aValue )
{
  /*
     In case you are using an operating system to communicate with your
     device driver that is running within its own thread/task/process,
     send a message to the device driver and transmit the new value.
     Please note, that this function is called within the context of the main
     GUI thread.
  */

  /*
     Here we are accessing directly the device driver by calling a certain
     BSP / driver function.
  */

}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_PrintMessage
*
* DESCRIPTION:
*   This is a sample for a function that is called directly from a 'Command'
*   method of the device class. As a result, the corresponding action should
*   happen.
*   In this implementation the given message is printed via the serial interface.
*
*******************************************************************************/
void DeviceDriver_PrintMessage( XString aText )
{
  char    buf[ 256 ];
  char*   ptr = buf;
  int     buflen = sizeof( buf ) - 1;

  /* check for an empty string */
  if ( aText == 0 )
    return;

  /* read each 16-bit character and convert it to 8-bit character */
  while ( aText && *aText && buflen-- )
    *ptr++ = (char)(*aText++);

  /* terminate the converted string */
  *ptr = 0x00;

  /* do something with the converted string */
  EwPrint( "The string is: %s\n", buf );
}


/* msy */
