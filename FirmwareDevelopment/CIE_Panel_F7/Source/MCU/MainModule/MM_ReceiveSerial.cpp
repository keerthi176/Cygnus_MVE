/***************************************************************************
* File name: MM_USBSerial.cpp
* Project name: CIE_Panel
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on 18/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* USB Serial interface, PC send data to Panel
*
**************************************************************************/

#include <string.h>				// memcpy
#include "MM_Module.h"

#include "MM_Application.h"
#include "MM_ReceiveSerial.h"

ReceiveSerial::ReceiveSerial(Application *app) : Module( "ReceiveSerial", 0, app)
{
   m_isBinary = 1;
//   m_counter = 0;
	
   quadspi = (QuadSPI*) app->FindModule("QuadSPI");
}

/*
#define CAE_MAX_TEXT      40
#define MAX_CELLS        128
#define MAX_LINKS         MAX_CELLS
#define MAX_DEV_NAME      40
#define MAX_DEVSINAZONE  512
#define MAX_PANEL_NAME    40
#define MAX_EMERGENCY    200
#define MAX_ZONE          96
#define NUM_CONTACTS      10
#define SITE_NAME         40
#define SITE_DESC        256
#define OPPER_NAME        32
#define SITE_LOCATION     64
#define MAX_CAE_DATA     500
#define MAX_PANEL          5

typedef struct _tagFILEHEADER
{
   char      name[14];          // "CygnusSmartNet"   - NO trailing 0
   uint8_t   verHigh;           // File Version, high byte : 0 - 255
   uint8_t   verLow;            // File Version, Low byte : 0 - 255
   uint32_t  fileLength;        // Size in bytes including header and checksum
   char      fileCreated[18];   // "27/07/18 09:05:27\0"
   char      fileModified[18];  // "27/07/18 15:28:07\0"
   char      guiVersion[8];     // "0.00.20\0"
} FILEHEADER;

typedef struct _tagCONTACT
{
   uint16_t  opts;
   char      number[16];
} CONTACT;

// Cae GUI "Cell"
typedef struct _tagTOOLTIP_TABLE
{
   int    idb;
   char   text[40];
} TOOLTIP_TABLE;

typedef struct _tagTYPE_TABLE
{
   char   code[12];
   char   text[40];
} TYPE_TABLE;

typedef struct _tagTIME
{
   uint8_t wHour_S;      // _S = start of day
   uint8_t wMinute_S;
   uint8_t wHour_E;      // _E = End of day
   uint8_t wMinute_E;
} TIME;

// Cae GUI "Cell"
typedef struct _tagCELL
{
   uint16_t   x;
   uint16_t   y;
   union
   {
      uint16_t   device;      // 255 = unknown   : 0 based
      uint16_t   deviceMin;
      uint16_t   seconds;
   };
   uint16_t      deviceMax;
   uint8_t       type;
   uint8_t       typeRange;   // Bitmap of device types
   uint8_t       panel;       // 255 = unknown   : 0 based
   uint8_t       inputNum;
   union
   {
      uint8_t    outputNum;
      uint8_t    inclusive;
   };
   union
   {
      uint8_t    zone;        // 255 = unknown   : 0 based
      uint8_t    mins;
   };
} CELL;

typedef struct _tagLINK
{
   char cellFrom;
   char cellTo;
} LINK;

typedef struct _tagCAERECORD
{
   char      name[CAE_MAX_TEXT];      // +1 = trailing zero
   CELL      cells[MAX_CELLS];
   LINK      links[MAX_LINKS];
   uint8_t   numCells;
   uint8_t   numLinks;
   uint8_t   isComplete;
   uint8_t   PAD;
} CAERECORD;

typedef struct _tINPUTACT
{
   uint16_t  flags;
   uint8_t   action;     // 0-9, inpAction[NUM_INPACT]
   uint8_t   delay;      // 30secs - 10mins in steps of 30 secs. = 0-19
   uint8_t   dayLow;     // 0-255
   uint8_t   dayHigh;    // 0-255
   uint8_t   nightLow;   // 0-255
   uint8_t   nightHigh;  // 0-255
   uint8_t   faultLvl;   // 0-255
   uint8_t   preAlarm;   // percentage 0-100
   uint8_t   radioChannel;
   uint8_t   reserved;
} INPUTACT;

typedef struct _tOUTPUTSET
{
   uint16_t   flags;     // OUT_FIRE ... OUT_INVERTED
   uint16_t   actions;   // bitmap
   uint8_t    delay_1;   // 0 = off. 1 = 30secs...10mins in steps of 30 secs. = 0-20
   uint8_t    delay_2;   // 0 = 0ff. 1 = 30secs...10mins in steps of 30 secs. = 0-20 
   uint8_t    radioChannel;
   uint8_t    reserved;
} OUTPUTSET;

typedef struct _tDEVICE
{
   char      name[MAX_DEV_NAME];
   uint16_t  rDev;            // Radio Dev 0-511 (0xFFFF = not used / deleted)
   char      type;            // 0-43
   uint8_t   rZone;           // Radio Zone 0-95

   uint8_t   lowPower;        // 0 - 10 : Transmission low power
   uint8_t   highPower;       // 0 - 10 : Transmission high power
   uint8_t   volume;          // ul, l, m, h = 0-3
   uint8_t   flashRate;       // 0.5, 1 Hz = 0-1
   uint16_t  reserved;

   INPUTACT  inputAction[16];
   OUTPUTSET outputSetting[16];
} DEVICE;

typedef struct _tZONE
{
   char      name[40];            // Not used, kept in case
   uint16_t  numDevs;            // 0xFFFF = deleted zone
   uint16_t  devs[MAX_DEVSINAZONE];
} ZONE;

typedef struct _tPANEL
{
   char      name[MAX_PANEL_NAME];
   char      emergency[MAX_EMERGENCY];
   ZONE      zones[MAX_ZONE];
   DEVICE    devs[MAX_DEVSINAZONE];
   uint32_t  userPin;
   uint32_t  engPin;
   uint32_t  lev4Pin;
   uint8_t   calibHours;
   uint8_t   calibMins;
   TIME      dayNight[7];         // Day 0 = Monday
   uint8_t   sndProfile[10];
   uint8_t   inputFunc[4];
   uint8_t   inputType[4];
   uint32_t  systemId;
   uint16_t  numDevs;
   uint8_t   freq;
   uint8_t   numZones;            // 0xFF = Deleted panel
   uint16_t  reserved4Matt1;      // Reserved for later use
   uint16_t  reserved4Matt2;      // Reserved for later use
   DEVICE    inputs[16];          // Panel inputs
   DEVICE    outputs[16];         // Panel Outputs
   CONTACT   contacts[NUM_CONTACTS];
} PANEL;

typedef struct _tSITE
{
   // NOTE : These vartiables are written and read by name to / from the file.
   // ANY additional variables or changes to order, size etc MUST be replicated
   // in the SaveFile() and LoadFile() functions
   char      name[SITE_NAME];
   char      description[SITE_DESC];
   uint32_t  jobNumber;
   char      opperatorName[OPPER_NAME];
   char      location[SITE_LOCATION];
   uint8_t   numPanels;      // Max 5
   uint8_t   flags;
   CAERECORD caeData[MAX_CAE_DATA];
   uint16_t  numCaeData;

   // Only "used" panels are written to file
   PANEL     panels[MAX_PANEL];
} SITE;
*/

void Dump(void *tmp)
{
	 char    szBuffer[100];
	 char    szByte[10];
	 uint8_t *ptr;
	 int     c = 0;
	
	 ptr = (uint8_t *) tmp;
	 for (int r = 0; r < 4; r++)
	 {
		 for (int i = 0; i < 16; i++)
		 {
			  sprintf(szByte, "%02X ", ptr[c++]);
			  strcat(szBuffer, szByte);
		 }
	   strcat(szBuffer, "\r\n");
     app->DebOut(szBuffer);
		 szBuffer[0] = 0;
	 }
}

extern FILEHEADER *fh;
extern SITE *site;

int      qty = 0;
uint32_t m_counter;
int      buffCntr;
uint32_t dst_addr;

void ReceiveSerial::Receive( Pipe* pipe, char* data, int len )
{
	uint8_t *ptr;
	
//   if (m_isBinary == 0)
//   {
//     app->DebOut("I got %d bytes: %c\r\n", len, data[0]);
//   }
//   else if (m_isBinary == 1)
   {
     if (fh->fileLength == 0)
     {
        app->DebOut("first : %d\r\n", len);
  
        qty++;
        m_counter = len;
        memcpy(fh, data, len);
        Dump(data);
        app->DebOut("\r\n");
        Dump(fh);
        app->DebOut("\r\n");

        app->DebOut("size = %d\r\n", fh->fileLength);
        pipe->totalsize = fh->fileLength;

        m_checksum = 0;
        m_checksum = crc32(data, len, m_checksum);

/////////////////////////////////////////////////////////////////////////////////
        memcpy(buff, data, QSPI_PAGE_SIZE);
        buffCntr = 1;
        dst_addr = 0;
 
// 			  unsigned int dst_addr = 0;
//	      int          num_pages = NUM_PAGES;
//        char         buff[QSPI_PAGE_SIZE * NUM_PAGES];

//	      quadspi->SetDstAddress(dst_addr);
//	      quadspi->WritePage(data);
/////////////////////////////////////////////////////////////////////////////////

     }
     else
     {
        m_counter += len;
        qty++;
			 
			  ptr = (uint8_t *) (fh + (qty * 64)); 
			  memcpy(ptr, data, len);
			 
        if (qty >= 1000)
        {   
           qty = 0;
					app->DebOut("%d : %d (%d)\r\n", len , m_counter, pipe->totalsize);
        }
      
        // Check for end of file
        if (m_counter >= pipe->totalsize)
        {
           app->DebOut("End of file : Received %d of %d bytes\r\n", m_counter, pipe->totalsize);
           m_checksum = crc32(data, len - 3, m_checksum);
         
           uint32_t chksum;
           chksum  = data[len - 0] << 24;
           chksum += data[len - 1] << 16;
           chksum += data[len - 2] << 8;
           chksum += data[len - 3];

           app->DebOut("file checksum 0x%X\r\n", chksum);
           app->DebOut("calc checksum 0x%X\r\n", m_checksum);
         }
         else
         {
           m_checksum = crc32(data, len, m_checksum);
         }
				 
				 //////////////////////////////////////////////////////////
				 if (buffCntr == 0)
				 {
	  		   memcpy(buff, data, QSPI_PAGE_SIZE);
			     buffCntr++;
				 }
				 else if (buffCntr == 1)
				 {
	  		   memcpy(buff + 64, data, QSPI_PAGE_SIZE);
			     buffCntr++;
				 }
				 else if (buffCntr == 2)
				 {
	  		   memcpy(buff + 128, data, QSPI_PAGE_SIZE);
			     buffCntr++;
				 }
				 else if (buffCntr == 3)
				 {
	  		   memcpy(buff + 192, data, QSPI_PAGE_SIZE);
			     buffCntr = 0;
					 
 					 app->DebOut("QSPI : %d\r\n", dst_addr);
	         quadspi->SetDstAddress(dst_addr);
	         quadspi->WritePage(buff);
					 dst_addr += QSPI_PAGE_SIZE;
				 }
      }
   }
//   else
//   {
//      app->DebOut("ReceiveSerial::Receive() - Bad setup...\r\n");
//   }
}

void ReceiveSerial::EndReception(Pipe*, int success)
{
   app->DebOut("EndReception() %d\r\n", success);
}

const uint32_t Polynomial = 0x04C11DB7;           // Matts request

uint32_t ReceiveSerial::crc32(const void* data, int length, uint32_t previousCrc32)
{
   int            j;
   uint32_t       crc = ~previousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
   const uint8_t* current = (const uint8_t*)data;

   while (length-- > 0)
   {
      crc ^= *current++;

      for (j = 0; j < 8; j++)
      {
         crc = (crc >> 1) ^ (crc & 1)*Polynomial;
      }
   }

   return ~crc; // same as crc ^ 0xFFFFFFFF
}

