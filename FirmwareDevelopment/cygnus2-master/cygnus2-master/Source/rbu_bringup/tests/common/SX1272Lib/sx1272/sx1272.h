/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C) 2015 Semtech

Description: Actual implementation of a SX1272 radio, inherits Radio

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainers: Miguel Luis, Gregory Cristian and Nicolas Huguenin
*/
#ifndef __SX1272_H__
#define __SX1272_H__

#include "radio.h"
#include "./registers/sx1272Regs-Fsk.h"
#include "./registers/sx1272Regs-LoRa.h"
#include "./typedefs/typedefs.h"

/*!
 * Radio wakeup time from SLEEP mode
 */
#define RADIO_OSC_STARTUP                           1000 // [us]

/*!
 * Radio PLL lock and Mode Ready delay which can vary with the temperature
 */
#define RADIO_SLEEP_TO_RX                           2000 // [us]

/*!
 * Radio complete Wake-up Time with margin for temperature compensation
 */
#define RADIO_WAKEUP_TIME                           ( RADIO_OSC_STARTUP + RADIO_SLEEP_TO_RX )


/*!
 * SX1272 definitions
 */
#define XTAL_FREQ                                   32000000
#define FREQ_STEP                                   61.03515625

#define RX_BUFFER_SIZE                              256

/*!
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET                                 -139

/*! 
 * Actual implementation of a SX1272 radio, inherits Radio
 */
class SX1272 : public Radio
{
protected:
    /*!
    * SPI Interface
    */
    SPI spi; // mosi, miso, sclk
    DigitalOut nss;

    /*!
     * SX1272 Reset pin
     */
    DigitalInOut reset;

    /*!
     * SX1272 data pin DIO2 for read/write in continuous mode
     */
    DigitalInOut sx_data;

    /*!
     * SX1272 DIO pins
     */
    InterruptIn dio0;
    InterruptIn dio1;
    InterruptIn dio2; 
    InterruptIn dio3;
    InterruptIn dio4;
    DigitalIn dio5;

    bool isRadioActive;

    uint8_t boardConnected; //1 = SX1272MB1DCS; 0 = SX1272MB1DAS

    uint8_t *rxtxBuffer;
    
    uint8_t currentOpMode;
    bool lastContinuousDataBit;
    bool sendDataBit;
    bool enableContinuousDataToggle;
    uint32_t sxDataZeroCount;
    uint32_t sxDataOneCount;
    uint32_t berCount;
    bool sequencerEnabled;


    /*!
     * Hardware DIO IRQ functions
     */
    DioIrqHandler *dioIrq;

    /*!
     * Tx and Rx timers
     */
    Timeout txTimeoutTimer;
    Timeout rxTimeoutTimer;
    Timeout rxTimeoutSyncWord;

    /*!
     *  rxTx: [1: Tx, 0: Rx]
     */
    uint8_t rxTx;

    RadioSettings_t settings;

    static const FskBandwidth_t FskBandwidths[] ;
protected:

    /*!
    * Performs the Rx chain calibration for LF and HF bands
    * \remark Must be called just after the reset so all registers are at their
    *         default values
    */
    void RxChainCalibration( void );

public:
    SX1272( RadioEvents_t *events,
            PinName mosi, PinName miso, PinName sclk, PinName nss, PinName reset,
            PinName dio0, PinName dio1, PinName dio2, PinName dio3, PinName dio4, PinName dio5 ); 
    SX1272( RadioEvents_t *events );
    virtual ~SX1272( );
    
    //-------------------------------------------------------------------------
    //                        Redefined Radio functions
    //-------------------------------------------------------------------------
    /*!
     * @brief Initializes the radio
     *
     * @param [IN] events Structure containing the driver callback functions
     */
    virtual void Init( RadioEvents_t *events );
    /*!
     * Return current radio status
     *
     * @param status Radio status. [RF_IDLE, RX_RUNNING, TX_RUNNING]
     */
    virtual RadioState GetStatus( void ); 

    /*!
     * @brief Configures the SX1272 with the given modem
     *
     * @param [IN] modem Modem to be used [0: FSK, 1: LoRa] 
     */
    virtual void SetModem( RadioModems_t modem );

    /*!
     * @brief Sets the channel frequency
     *
     * @param [IN] freq         Channel RF frequency
     */
    virtual void SetChannel( uint32_t freq );

    /*!
     * @brief Sets the channels configuration
     *
     * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] freq       Channel RF frequency
     * @param [IN] rssiThresh RSSI threshold
     *
     * @retval isFree         [true: Channel is free, false: Channel is not free]
     */
    virtual bool IsChannelFree( RadioModems_t modem, uint32_t freq, int16_t rssiThresh );

    /*!
     * @brief Generates a 32 bits random value based on the RSSI readings
     *
     * \remark This function sets the radio in LoRa modem mode and disables
     *         all interrupts.
     *         After calling this function either Radio.SetRxConfig or
     *         Radio.SetTxConfig functions must be called.
     *
     * @retval randomValue    32 bits random value
     */
    virtual uint32_t Random( void );

    /*!
     * @brief Sets the reception parameters
     *
     * @param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] bandwidth    Sets the bandwidth
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * @param [IN] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * @param [IN] coderate     Sets the coding rate ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * @param [IN] bandwidthAfc Sets the AFC Bandwidth ( FSK only )
     *                          FSK : >= 2600 and <= 250000 Hz
     *                          LoRa: N/A ( set to 0 )
     * @param [IN] preambleLen  Sets the Preamble length ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Length in symbols ( the hardware adds 4 more symbols )
     * @param [IN] symbTimeout  Sets the RxSingle timeout value ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: timeout in symbols
     * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
     * @param [IN] payloadLen   Sets payload length when fixed lenght is used
     * @param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
     * @param [IN] freqHopOn    Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
     * @param [IN] hopPeriod    Number of symbols bewteen each hop (LoRa only)
     * @param [IN] iqInverted   Inverts IQ signals ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * @param [IN] rxContinuous Sets the reception in continuous mode
     *                          [false: single mode, true: continuous mode]
     * @param [IN] rxPacketMode Sets the reception in packet mode or continuous data mode (demodulated bit stream to SX_DIO2)
     *                          [false: continuous data mode, true: packet mode]
     */
    virtual void SetRxConfig ( RadioModems_t modem, uint32_t bandwidth,
                               uint32_t datarate, uint8_t coderate,
                               uint32_t bandwidthAfc, uint16_t preambleLen,
                               uint16_t symbTimeout, bool fixLen,
                               uint8_t payloadLen,
                               bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                               bool iqInverted, bool rxContinuous, bool rxPacketMode );

    /*!
     * @brief Sets the transmission parameters
     *
     * @param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] power        Sets the output power [dBm]
     * @param [IN] fdev         Sets the frequency deviation ( FSK only )
     *                          FSK : [Hz]
     *                          LoRa: 0
     * @param [IN] bandwidth    Sets the bandwidth ( LoRa only )
     *                          FSK : 0
     *                          LoRa: [0: 125 kHz, 1: 250 kHz,
     *                                 2: 500 kHz, 3: Reserved]
     * @param [IN] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * @param [IN] coderate     Sets the coding rate ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * @param [IN] preambleLen  Sets the preamble length
     * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
     * @param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
     * @param [IN] freqHopOn    Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
     * @param [IN] hopPeriod    Number of symbols bewteen each hop (LoRa only)
     * @param [IN] iqInverted   Inverts IQ signals ( LoRa only )
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * @param [IN] timeout      Transmission timeout [us]
     * @param [IN] rxPacketMode Sets the Tx in packet mode or continuous data mode (bit stream on SX_DIO2 fed to modulator)
     *                          [false: continuous data mode, true: packet mode]
     */
    virtual void SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                              uint32_t bandwidth, uint32_t datarate,
                              uint8_t coderate, uint16_t preambleLen,
                              bool fixLen, bool crcOn, bool freqHopOn,
                              uint8_t hopPeriod, bool iqInverted, uint32_t timeout, bool rxPacketMode );

    /*!
     * @brief Computes the packet time on air for the given payload
     *
     * \Remark Can only be called once SetRxConfig or SetTxConfig have been called
     *
     * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] pktLen     Packet payload length
     *
     * @retval airTime        Computed airTime for the given packet payload length
     */
    virtual double TimeOnAir ( RadioModems_t modem, uint8_t pktLen );

    /*!
     * @brief Sends the buffer of size. Prepares the packet to be sent and sets
     *        the radio in transmission
     *
     * @param [IN]: buffer     Buffer pointer
     * @param [IN]: size       Buffer size
     */
    virtual void Send( uint8_t *buffer, uint8_t size );

    /*!
     * @brief Sets the data bit on SX_DIO2 (Data) for transmission in continuous data mode.
     *
     * @param [IN]: bitValue   The value to place on SX_DIO2
     */
    virtual void SetContinouousDatabit( bool bitValue );

      /*!
      * @brief Switches on/off the continuous data toggling transmission in continuous data mode.
      *
      * @param [IN]: value   True to enable continuous toggle of output data pin.  False to disable it.
      */
      virtual void EnableContinouousDatabitToggle( bool value );
      
      /*!
      * @brief Test if continuous data toggling transmission is enabled in continuous data mode.
      *
      * @return bool   True ifcontinuous toggle is enabled
      */
      virtual bool ContinouousDatabitToggleEnabled( void );

      /*!
      * @brief Read and return the RSSI value for FSK operation.
      *
      * @return int16_t   The RSSI value.
      */
      virtual int16_t GetFskRSSI( void );
      
    /*!
     * @brief Sets the data bit on SX_DIO2 (Data) for transmission in continuous data mode.
     *
     * @param [IN] buffered      True=get last chached value, False=get raw reading from DIO2
     * @return bool   The last read value on SX_DIO2.  True='1' false='0'
     */
    virtual bool GetContinouousDatabit( const bool buffered );
    
    

    /*!
     * @brief Sets the radio in sleep mode
     */
    virtual void Sleep( void );
    
    /*!
     * @brief Sets the radio in standby mode
     */
    virtual void Standby( void );

    /*!
     * @brief Sets the radio in reception mode for the given time
     * @param [IN] timeout Reception timeout [us]
     *                     [0: continuous, others timeout]
     */
    virtual void Rx( uint32_t timeout );

    /*!
     * @brief Sets the radio in transmission mode for the given time
     * @param [IN] timeout Transmission timeout [us]
     *                     [0: continuous, others timeout]
     */
    virtual void Tx( uint32_t timeout );

    /*!
     * @brief Start a Channel Activity Detection
     */
    virtual void StartCad( void );    

    /*!
     * @brief Reads the current RSSI value
     *
     * @retval rssiValue Current RSSI value in [dBm]
     */
    virtual int16_t GetRssi ( RadioModems_t modem );

    /*!
     * @brief Writes the radio register at the specified address
     *
     * @param [IN]: addr Register address
     * @param [IN]: data New register value
     */
    virtual void Write ( uint8_t addr, uint8_t data ) = 0;

    /*!
     * @brief Reads the radio register at the specified address
     *
     * @param [IN]: addr Register address
     * @retval data Register value
     */
    virtual uint8_t Read ( uint8_t addr ) = 0;

    /*!
     * @brief Writes multiple radio registers starting at address
     *
     * @param [IN] addr   First Radio register address
     * @param [IN] buffer Buffer containing the new register's values
     * @param [IN] size   Number of registers to be written
     */
    virtual void Write( uint8_t addr, uint8_t *buffer, uint8_t size ) = 0;

    /*!
     * @brief Reads multiple radio registers starting at address
     *
     * @param [IN] addr First Radio register address
     * @param [OUT] buffer Buffer where to copy the registers data
     * @param [IN] size Number of registers to be read
     */
    virtual void Read ( uint8_t addr, uint8_t *buffer, uint8_t size ) = 0;

    /*!
     * @brief Writes the buffer contents to the SX1272 FIFO
     *
     * @param [IN] buffer Buffer containing data to be put on the FIFO.
     * @param [IN] size Number of bytes to be written to the FIFO
     */
    virtual void WriteFifo( uint8_t *buffer, uint8_t size ) = 0;

    /*!
     * @brief Reads the contents of the SX1272 FIFO
     *
     * @param [OUT] buffer Buffer where to copy the FIFO read data.
     * @param [IN] size Number of bytes to be read from the FIFO
     */
    virtual void ReadFifo( uint8_t *buffer, uint8_t size ) = 0;
    /*!
     * @brief Resets the SX1272
     */
    virtual void Reset( void ) = 0;

    /*!
     * @brief Sets the maximum payload length.
     *
     * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
     * @param [IN] max        Maximum payload length in bytes
     */
    virtual void SetMaxPayloadLength( RadioModems_t modem, uint8_t max );
    
    virtual void SetTxCW( uint32_t freq, int8_t power, uint32_t timeout );
    virtual void SetTxContinuousMode( uint32_t freq, int8_t power, uint32_t frequencyDeviation, uint32_t bandwidth, uint32_t dataRate, bool enableSequencer, uint32_t timeout );
    virtual void SetRxContinuousMode( uint32_t freq, uint32_t frequencyDeviation, uint32_t bandwidth, uint32_t dataRate, bool use_lora_crc, bool enableSequencer );
    
   /*!
   * @brief Returns the count of rx'd zeros and ones on the SX_DATA pin and resets the counts to zero.
   *
   * @param [OUT]: zeroCount   The count of rx'd '0' bits..
   * @param [OUT]: oneCount    The count of rx'd '1' bits..
   * @param [OUT]: ber         The bit error rate.
   */
   virtual void ReadAndResetSXDataCounters( uint32_t* zeroCount, uint32_t* oneCount, double* ber  );

    //-------------------------------------------------------------------------
    //                        Board relative functions
    //-------------------------------------------------------------------------

protected:
    /*!
     * @brief Initializes the radio I/Os pins interface
     */
    virtual void IoInit( void ) = 0;
    
    /*!
     *    @brief Initializes the radio registers
     */
    virtual void RadioRegistersInit( ) = 0;
    
    /*!
     * @brief Initializes the radio SPI
     */
    virtual void SpiInit( void ) = 0;

    /*!
     * @brief Initializes DIO IRQ handlers
     *
     * @param [IN] irqHandlers Array containing the IRQ callback functions
     */
    virtual void IoIrqInit( DioIrqHandler *irqHandlers ) = 0;

    /*!
     * @brief De-initializes the radio I/Os pins interface. 
     *
     * \remark Useful when going in MCU lowpower modes
     */
    virtual void IoDeInit( void ) = 0;

    /*!
     * @brief Gets the board PA selection configuration
     *
     * @param [IN] channel Channel frequency in Hz
     * @retval PaSelect RegPaConfig PaSelect value
     */
    virtual uint8_t GetPaSelect( uint32_t channel ) = 0;

    /*!
     * @brief Set the RF Switch I/Os pins in Low Power mode
     *
     * @param [IN] status enable or disable
     */
    virtual void SetAntSwLowPower( bool status ) = 0;

    /*!
     * @brief Initializes the RF Switch I/Os pins interface
     */
    virtual void AntSwInit( void ) = 0;

    /*!
     * @brief De-initializes the RF Switch I/Os pins interface 
     *
     * \remark Needed to decrease the power consumption in MCU lowpower modes
     */
    virtual void AntSwDeInit( void ) = 0;

    /*!
     * @brief Controls the antena switch if necessary.
     *
     * \remark see errata note
     *
     * @param [IN] rxTx [1: Tx, 0: Rx]
     */
    virtual void SetAntSw( uint8_t rxTx ) = 0;

    /*!
     * @brief Checks if the given RF frequency is supported by the hardware
     *
     * @param [IN] frequency RF frequency to be checked
     * @retval isSupported [true: supported, false: unsupported]
     */
    virtual bool CheckRfFrequency( uint32_t frequency ) = 0;
protected:

    /*!
     * @brief Sets the SX1272 operating mode
     *
     * @param [IN] opMode New operating mode
     */
    virtual void SetOpMode( uint8_t opMode );

    /*
     * SX1272 DIO IRQ callback functions prototype
     */

    /*!
     * @brief DIO 0 IRQ callback
     */
    virtual void OnDio0Irq( void );

    /*!
     * @brief DIO 1 IRQ callback
     */
    virtual void OnDio1Irq( void );

    /*!
     * @brief DIO 2 IRQ callback
     */
    virtual void OnDio2Irq( void );

    /*!
     * @brief DIO 3 IRQ callback
     */
    virtual void OnDio3Irq( void );

    /*!
     * @brief DIO 4 IRQ callback
     */
    virtual void OnDio4Irq( void );

    /*!
     * @brief DIO 5 IRQ callback
     */
    virtual void OnDio5Irq( void );

    /*!
     * @brief Tx & Rx timeout timer callback
     */
    virtual void OnTimeoutIrq( void );

    /*!
     * Returns the known FSK bandwidth registers value
     *
     * \param [IN] bandwidth Bandwidth value in Hz
     * \retval regValue Bandwidth register value.
     */
    static uint8_t GetFskBandwidthRegValue( uint32_t bandwidth );
};

#endif // __SX1272_H__
