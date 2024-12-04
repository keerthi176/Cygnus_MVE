/***************************************************************************
* File name: MM_USBHost.h
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
* First written on 12/02/19 by Phil Lane
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 12/02/19 Initial File
*
* Description:
* USB To Serial link, PC send data to Panel
*
**************************************************************************/

#ifndef _RECEIVE_SERIAL_
#define _RECEIVE_SERIAL_

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Pipe.h"
#include "MM_Application.h"
#include "MM_QuadSPI.h"

class ReceiveSerial : public Module, public PipeTo
{
public:
	ReceiveSerial(Application *app);
	
	virtual int StartReception(Pipe*) { return true; }
	virtual void EndReception(Pipe*, int success);
	virtual void Receive( Pipe*, char* data, int len );
	
	void SetBinary(int binary) {m_isBinary = binary;};
	
private:
	
  uint32_t crc32(const void* data, int length, uint32_t previousCrc32);

	QuadSPI* 	quadspi;
  char      buff[QSPI_PAGE_SIZE];

  int      m_isBinary;
  uint32_t m_size;  
  uint32_t m_checksum;
};

#endif //_RECEIVE_SERIAL_
