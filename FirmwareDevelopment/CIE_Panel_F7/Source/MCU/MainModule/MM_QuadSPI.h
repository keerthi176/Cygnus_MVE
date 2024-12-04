/***************************************************************************
* File name: MM_QuadSPI.cpp
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
* Panel settings class. Stores network, mesh, etc
*
**************************************************************************/
#ifndef __QUAD_SPI__
#define __QUAD_SPI__


/* System Include Files
**************************************************************************/



/* User Include Files
**************************************************************************/

#include "MM_Application.h"
#include "MM_Pipe.h"



/* Defines
**************************************************************************/
/* Definition for QSPI DMA */
#define QSPI_DMA_INSTANCE          DMA2_Stream7
#define QSPI_DMA_CHANNEL           DMA_CHANNEL_3
#define QSPI_DMA_IRQ               DMA2_Stream7_IRQn
#define QSPI_DMA_IRQ_HANDLER       DMA2_Stream7_IRQHandler

/* N25Q512A13GSF40E Micron memory */
/* Size of the flash */
#define QSPI_FLASH_SIZE                      25
#define QSPI_PAGE_SIZE                       256

/* Reset Operations */
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define READ_ID_CMD                          0x9E
#define READ_ID_CMD2                         0x9F
#define MULTIPLE_IO_READ_ID_CMD              0xAF
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A

/* Read Operations */
#define READ_CMD                             0x03
#define READ_4_BYTE_ADDR_CMD                 0x13

#define FAST_READ_CMD                        0x0B
#define FAST_READ_DTR_CMD                    0x0D
#define FAST_READ_4_BYTE_ADDR_CMD            0x0C

#define DUAL_OUT_FAST_READ_CMD               0x3B
#define DUAL_OUT_FAST_READ_DTR_CMD           0x3D
#define DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x3C

#define DUAL_INOUT_FAST_READ_CMD             0xBB
#define DUAL_INOUT_FAST_READ_DTR_CMD         0xBD
#define DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xBC

#define QUAD_OUT_FAST_READ_CMD               0x6B
#define QUAD_OUT_FAST_READ_DTR_CMD           0x6D
#define QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x6C

#define QUAD_INOUT_FAST_READ_CMD             0xEB
#define QUAD_INOUT_FAST_READ_DTR_CMD         0xED
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xEC

/* Write Operations */
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_DISABLE_CMD                    0x04

/* Register Operations */
#define READ_STATUS_REG_CMD                  0x05
#define WRITE_STATUS_REG_CMD                 0x01

#define READ_LOCK_REG_CMD                    0xE8
#define WRITE_LOCK_REG_CMD                   0xE5

#define READ_FLAG_STATUS_REG_CMD             0x70
#define CLEAR_FLAG_STATUS_REG_CMD            0x50

#define READ_NONVOL_CFG_REG_CMD              0xB5
#define WRITE_NONVOL_CFG_REG_CMD             0xB1

#define READ_VOL_CFG_REG_CMD                 0x85
#define WRITE_VOL_CFG_REG_CMD                0x81

#define READ_ENHANCED_VOL_CFG_REG_CMD        0x65
#define WRITE_ENHANCED_VOL_CFG_REG_CMD       0x61

#define READ_EXT_ADDR_REG_CMD                0xC8
#define WRITE_EXT_ADDR_REG_CMD               0xC5

/* Program Operations */
#define PAGE_PROG_CMD                        0x02
#define PAGE_PROG_4_BYTE_ADDR_CMD            0x12

#define DUAL_IN_FAST_PROG_CMD                0xA2
#define EXT_DUAL_IN_FAST_PROG_CMD            0xD2

#define QUAD_IN_FAST_PROG_CMD                0x32
#define EXT_QUAD_IN_FAST_PROG_CMD            0x12 /*0x38*/
#define QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD    0x34

/* Erase Operations */
#define SUBSECTOR_ERASE_CMD                  0x20
#define SUBSECTOR_ERASE_4_BYTE_ADDR_CMD      0x21

#define SECTOR_ERASE_CMD                     0xD8
#define SECTOR_ERASE_4_BYTE_ADDR_CMD         0xDC

#define BULK_ERASE_CMD                       0xC7

#define PROG_ERASE_RESUME_CMD                0x7A
#define PROG_ERASE_SUSPEND_CMD               0x75

/* One-Time Programmable Operations */
#define READ_OTP_ARRAY_CMD                   0x4B
#define PROG_OTP_ARRAY_CMD                   0x42

/* 4-byte Address Mode Operations */
#define ENTER_4_BYTE_ADDR_MODE_CMD           0xB7
#define EXIT_4_BYTE_ADDR_MODE_CMD            0xE9

/* Quad Operations */
#define ENTER_QUAD_CMD                       0x35
#define EXIT_QUAD_CMD                        0xF5

/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ              8
#define DUMMY_CLOCK_CYCLES_READ_QUAD         10

#define DUMMY_CLOCK_CYCLES_READ_DTR          6
#define DUMMY_CLOCK_CYCLES_READ_QUAD_DTR     8

/* End address of the QSPI memory */
#define QSPI_END_ADDR              (1 << QSPI_FLASH_SIZE)

#define QSPI_SECTOR_SIZE 				(64*1024)

#define QSPI_SETTINGS_ADDRESS			0xC00000
#define QSPI_BITMAP_ADDRESS			0xD00000
#define QSPI_VIDEO_ADDRESS				0xDC0000


enum QSPI_Result
{
	QSPI_RESULT_OK,
	QSPI_RESULT_WRITE_PROTECTED,
	QSPI_RESULT_WRITE_FAIL,
	QSPI_RESULT_READ_FAIL,
	QSPI_RESULT_ERASE_FAIL,
};
	

// Helper macro
#define WriteToPanel( data, dest )  WriteToFlash( (char*) &(data), ((int)&app.panel->dest) - SETTINGS_ROM_ADDRESS, sizeof(data) ) 
#define WriteToSite( data, dest )  WriteToFlash( (char*) &(data), ((int)&app.site->dest) - SETTINGS_ROM_ADDRESS, sizeof(data) ) 
	
 

class QuadSPI : public Module 
{
	public:
	
	QuadSPI( );
		
	virtual int Receive( Event );
	virtual int Receive( Command* );
	virtual int Receive( Message* );
	
	virtual int Init( );
		
	bool IsWriteProtected( );
	
	int EraseSector( int addr );
	int ProgramPage( int addr, char* buff, int len );
	int WritePage( char* data );
	int WaitFor( volatile int& global );
	int EraseSubSector( int addr );
	
	QSPI_Result WriteToFlash( char* data, int dest_addr, int size );
	QSPI_Result WriteDontEraseToFlash( char* data, int dest_addr, int size );
	QSPI_Result MoveFlashLower( int src, int dest, int size );
	
		
	volatile char* 			 qspi_addr;
	QSPI_CommandTypeDef      eraseCommand;
	QSPI_CommandTypeDef      txCommand;
	QSPI_CommandTypeDef      rxCommand;
   QSPI_CommandTypeDef      mapCommand;
	QSPI_CommandTypeDef      subEraseCommand;
   QSPI_MemoryMappedTypeDef sMemMappedCfg;	
	
	unsigned int 	src_addr;
	unsigned int 	dst_addr;
	 
	
	private:
		
	EmbeddedWizard* ew;
	
	int DummyCyclesCfg( QSPI_HandleTypeDef *QSPIHandle );
	int WriteEnable( QSPI_HandleTypeDef *hqspi );
	
	HAL_StatusTypeDef AutoPollingMemReady( QSPI_HandleTypeDef *hqspi );
	HAL_StatusTypeDef ResetMemory( QSPI_HandleTypeDef* hqspi );
	 
	
};







#endif
