#ifndef spi_rfm95_h
#define spi_rfm95_h

#include <SPI.h>

/** ######################################################################

Driver for the RFM95W LongRange Tranceiver (Samtech sx1276)

 CONSUMPTION:
    sleep       0.2-1 µA
    idle        1.5 µA
    standby     1.8 µA
    RX          12 mA
    TX          20 (7dBm) - 120mA (20dBm)

 Details:

TODO:
- uses digitalWrite-Stuff
-

########################################################################  */



class RFM95
{

#define RFM_FIFO_SIZE               256
#define RFM_PACKET_SIZE_MAX         128
#define RFM_FOSC                    32000000.0
#define RFM_FSTEP                   (RFM_FOSC / 524288)
#define RFM_FINV                    (524288.0 / RFM_FOSC)
#define RFM_CS                      10 // PIN

#define SPI_WRITE(a)                (a|0x80)
#define SPI_READ(a)                 (a&0x7F)


    /** ######### Register-Map ################################################################# */

#define REG_FIFO                    0x00    // cleared when in SleepMode
#define REG_OPMODE                  0x01
#define     MSK_OPMODE_LORA         (B10000000) // settable only in sleep mode
#define     MSK_OPMODE_ACCSHARED    (B01000000)
#define     MSK_OPMODE_LOFREQ       (B00001000)
#define     MSK_OPMODE_MODE         (B00000111)
#define         VAL_MODE_SLEEP      0x00    // only mode to allow switching FSK/OOK/LORA
#define         VAL_MODE_STANDBY    0x01
#define         VAL_MODE_FSTX       0x02
#define         VAL_MODE_TX         0x03
#define         VAL_MODE_FSRX       0x04
#define         VAL_MODE_RX_CONT    0x05
#define         VAL_MODE_RX_SINGLE  0x06
#define         VAL_MODE_CAD        0x07

#define REG_REGFRFMSB               0x06    // RF Carrier Freq --> access in sleep/stdby
#define REG_REGFRFMID               0x07
#define REG_REGFRFLSB               0x08

#define REG_PA_CONFIG               0x09    // PA selection, output power control
#define     MSK_PA_SELECT           (B10000000)  // 0:RFO, 1:BOOST
#define     MSK_PA_MAX_POWER        (B01110000)  // pmax = 10.8+0.6*REG dBm
#define     MSK_PA_OUT_POWER        (B00001111)  // pout = pmax - (15-REG) OR (boost) pout = 2 + REG
#define REG_PA_RAMP                 0x0A    // low phase noise

#define     MSK_PA_RAMP             (B00001111)
#define REG_OCP                     0x0B    // Over current Protection
#define     MSK_OCP_ON              (B00100000)
#define     MSK_OCP_TRIM            (B00011111) // max 240mA, default: 100mA
#define REG_LNA                     0x0C    // LNA settings
#define     MSK_LNA_GAIN            (B11100000) // possible: 1-6 from 0 to -48dB
#define     MSK_LNA_BOOST_LF        (B00011000) // do not change
#define     MSK_LNA_BOOST_HF        (B00000011)
#define         VAL_LNA_BOOST_HF_ON (B00000011) // boost 150%
#define         VAL_LNA_BOOST_HF_DE (B00000000)

#define REG_FIFO_ADDR_PTR           0x0D    // write position to read/write (autoinc)
#define REG_FIFO_TX_BASE_AD         0x0E    // startup: 0x80
#define REG_FIFO_RX_BASE_AD         0x0F    // startup: 0x00
#define REG_FIFO_RX_CURRENT_ADDR    0x10    // start of last packet received

#define REG_IRQ_FLAGS_MASK          0x11
#define REG_IRQ_FLAGS               0x12
#define     MSK_IRQ_RX_TIMEOUT      (B10000000)
#define     MSK_IRQ_RX_DONE         (B01000000)
#define     MSK_IRQ_PAYLOAD_CRC_ERR (B00100000)
#define     MSK_IRQ_VALID_HEADER    (B00010000)
#define     MSK_IRQ_TX_DONE         (B00001000)
#define     MSK_IRQ_CAD_DONE        (B00000100)
#define     MSK_IRQ_FHSS_CHANGE     (B00000010)
#define     MSK_IRQ_CAD_DETECTED    (B00000001)


#define REG_RX_NB_BYTES             0x13    // successful receive will write this bytecount

#define REG_RX_HEADER_CNT_MSB       0x14
#define REG_RX_HEADER_CNT_LSB       0x15
#define REG_RX_PACKET_CNT_MSB       0x16
#define REG_RX_PACKET_CNT_LSB       0x17

#define REG_MODEM_STAT              0x18
#define     MSK_MODEM_RX_CODINGRATE (B11100000)
#define     MSK_MODEM_STATUS        (B00011111)
#define     MSK_MODEM_STATUS_CLEAR  (B00010000)
#define     MSK_MODEM_STATUS_HDRVAL (B00001000)
#define     MSK_MODEM_STATUS_RXON   (B00000100)
#define     MSK_MODEM_STATUS_SYNC   (B00000010)
#define     MSK_MODEM_STATUS_SIGDET (B00000001)

#define REG_PKT_SNR_VALUE           0x19    // last Packets SNR dB = int8_REG/4
#define REG_PKT_RSSI_VALUE          0x1A    // last Packets RSSI dBm = REG - 127
#define REG_RSSI_VALUE              0x1B    // current RSSI dBm = REG - 127

#define REG_HOP_CHANNEL             0x1C
#define     MSK_HOP_PLL_TIMEOUT     (B10000000)
#define     MSK_HOP_RX_CRCON        (B01000000)
#define     MSK_HOP_FHSS_CHANNEL    (B00111111)

#define REG_MODEM_CONFIG1           0x1D        ///
#define     MSK_MODEM_BW            (B11110000)
#define         VAL_MODEM_BW008     (B00000000) // in kHz
#define         VAL_MODEM_BW010     (B00010000)  // in kHz
#define         VAL_MODEM_BW016     (B00100000)  // in kHz
#define         VAL_MODEM_BW021     (B00110000)  // in kHz
#define         VAL_MODEM_BW031     (B01000000)  // in kHz
#define         VAL_MODEM_BW042     (B01010000)  // in kHz
#define         VAL_MODEM_BW063     (B01100000)  // in kHz
#define         VAL_MODEM_BW125     (B01110000)  // in kHz
#define         VAL_MODEM_BW250     (B10000000)  // in kHz
#define         VAL_MODEM_BW500     (B10010000)  // in kHz
#define     MSK_MODEM_CR            (B00001110)
#define         VAL_MODEM_CR1       (B00000010) // Coding Rate 4/5
#define         VAL_MODEM_CR2       (B00000100) // Coding Rate 4/6
#define         VAL_MODEM_CR3       (B00000110) // Coding Rate 4/7
#define         VAL_MODEM_CR4       (B00001000) // Coding Rate 4/8
#define     MSK_MODEM_IMPLICITHDR   (B00000001) // implicit header stores length

#define REG_MODEM_CONFIG2           0x1E
#define     MSK_MODEM_SF            (B11110000)
#define         VAL_MODEM_SF06      (B01100000) // 64 chips/symbol --> SNR -5 dB
#define         VAL_MODEM_SF07      (B01110000) // 128 chips/symbol --> SNR -7.5 dB
#define         VAL_MODEM_SF08      (B10000000) // 256 chips/symbol --> SNR -10 dB
#define         VAL_MODEM_SF09      (B10010000) // 512 chips/symbol --> SNR -12.5 dB
#define         VAL_MODEM_SF10      (B10100000) // 1024 chips/symbol --> SNR -15 dB
#define         VAL_MODEM_SF11      (B10110000) // 2048 chips/symbol --> SNR -17.5 dB
#define         VAL_MODEM_SF12      (B11000000) // 4096 chips/symbol --> SNR -20 dB
#define     MSK_TX_CONTINOUOS       (B00001000)
#define     MSK_RX_PAYLOAD_CRC_ON   (B00000100)
#define     MSK_SYMB_TIMEOUTMSB     (B00000011)
#define REG_SYMB_TIMEOUTLSB         0x1F    // number of symbols; timeout = REG * Ts
#define REG_PREAMBLE_MSB            0x20    // Length = REG + 4.25 Symbols
#define REG_PREAMBLE_LSB            0x21

#define REG_PAYLOAD_LENGTH          0x22    // size of bytes to be transmitted
#define REG_MAX_PAYLOAD_LENGTH      0x23

#define REG_HOP_PERIOD              0x24    // 0: disable

#define REG_FIFO_RX_BYTE_ADDR       0x25    // Addr of last byte written

#define REG_MODEM_CONFIG3           0x26
#define     MSK_LOW_DATARATE_OPTI   (B00001000) // Mandatory when symbollength > 16ms
#define     MSK_AGC_AUTO_ON         (B00000100)

/// 0x27 - 3F not for LoRa

#define REG_DIO_MAPPING_1           0x40
#define     MSK_DIO0_MAPPING        (B11000000)
#define     MSK_DIO1_MAPPING        (B00110000)
#define     MSK_DIO2_MAPPING        (B00001100)
#define     MSK_DIO3_MAPPING        (B00000011)
#define REG_DIO_MAPPING_2           0x41
#define     MSK_DIO4_MAPPING        (B11000000)
#define     MSK_DIO5_MAPPING        (B00110000)
#define     MSK_DIO_MAP_PREAMB_DET  (B00000001)
#define REG_VERSION                 0x42
#define     VAL_V1B                 0x12 // has errors --> extra document (errata)

#define REG_TCXO                    0x4B    // TCXO or XTAL input
#define     MSK_TCXO_ON             (B00010000)
#define REG_PA_DAC                  0x4D    // Power setting of PA
#define     MSK_PA_DAC              (B00000111)
#define         VAL_PA_DAC_DEFAULT  0x04
#define         VAL_PA_DAC_20DBM    0x07    // when outputpower = 111
#define REG_FORMER_TEMP             0x5B    // -1°C per LSB

#define REG_AGC_REF                 0x61
#define     MSK_AGC_REF_LEVEL       (B00111111) // def=0x19
#define REG_AGC_THRESH1             0x62
#define     MSK_AGC_STEP1           0x0F
#define REG_AGC_THRESH2             0x63
#define     MSK_AGC_STEP2           0xF0
#define     MSK_AGC_STEP3           0x0F
#define REG_AGC_THRESH3             0x64
#define     MSK_AGC_STEP4           0xF0
#define     MSK_AGC_STEP5           0x0F
#define REG_PLL                     0x70
#define     MSK_PLL_BW              (B11000000)
#define         VAL_PLL_BW075KHZ    (B00000000)
#define         VAL_PLL_BW150KHZ    (B01000000)
#define         VAL_PLL_BW225KHZ    (B10000000)
#define         VAL_PLL_BW300KHZ    (B11000000)

/// static configuration:
// CRCon
// Variable packet: preamble B, SyncWord/NW-ID 1B, length 1B, addr 1B, Data , CRC 2B
// AGC: automatic gain control

private:

    uint8_t _filterBadCRC;
    uint8_t _mode;
    uint8_t _idleState, _receiveContinouos, _awaitAck;

public:



    /** ######### SPI ################################################################# */

    void spiExchange(uint8_t regExchange, uint8_t buffer[], uint8_t length=1) // uint8_t CS,
    {
        if (!length) return;
        digitalWrite(RFM_CS, LOW);
        SPI.transfer(regExchange);
        for (uint8_t counter=0; counter < length; counter++)
        {
            buffer[counter] = SPI.transfer(buffer[counter]);
        };
        // todo: better while (length--) *buffer++ = SPI.transfer(*buffer);
        digitalWrite(RFM_CS, HIGH);
    };

    uint8_t spiRead(uint8_t regValue)
    {
        digitalWrite(RFM_CS, LOW);
        SPI.transfer(SPI_READ(regValue));
        uint8_t _value = SPI.transfer(0);
        digitalWrite(RFM_CS, HIGH);
        return _value;
    };

    void spiWrite(uint8_t regValue, uint8_t value = 0)
    {
        digitalWrite(RFM_CS, LOW);
        SPI.transfer(SPI_WRITE(regValue));
        SPI.transfer(value);
        digitalWrite(RFM_CS, HIGH);
    };

    void setRegister(uint8_t regValue, uint8_t mask, uint8_t value = 0)
    {
        uint8_t _toWrite = spiRead(regValue);
        _toWrite = (_toWrite & (!mask)) | value;
        spiWrite(regValue, _toWrite);
    };

    /** ######### function definition ################################################################# */

    RFM95(void): _filterBadCRC(0), _mode (0), _idleState(0), _receiveContinouos(0), _awaitAck(0)
    {
        pinMode(RFM_CS, OUTPUT);
        SPI.begin();
        SPI.setClockDivider(SPI_CLOCK_DIV8); // RFM95W can handle 10MHz
        SPI.setDataMode(SPI_MODE0);
        SPI.setBitOrder(MSBFIRST);
    };

    uint8_t initialize()
    {
        if (spiRead(REG_VERSION) != VAL_V1B) return 1; // FAIL

        /// activate sleep and lora
        setEnabled(1);
        delay(20);
        setRegister(REG_OPMODE, MSK_OPMODE_LORA, 255); // setLoRaMode()
        setIdleState(VAL_MODE_RX_CONT);

        /// config FIFO
        spiWrite(REG_FIFO_TX_BASE_AD, 0x00);
        spiWrite(REG_FIFO_RX_BASE_AD, 0x00);
        spiWrite(REG_FIFO_ADDR_PTR, 0x00);
        spiWrite(REG_MAX_PAYLOAD_LENGTH, RFM_PACKET_SIZE_MAX); // longer packets trigger CRC-Error
        /// config IRQs
        // TODO
        spiWrite(REG_IRQ_FLAGS_MASK, 255); // All ON

        /// configure Message / Tranceiver
        setFrequency(868000);
        setPreambleLength(8);
        setPMax(10);
        setIMax(100);
        setLNA();
        filterCRC(0);

        setBandwidth(100);
        setRegister(REG_MODEM_CONFIG1, MSK_MODEM_CR, VAL_MODEM_CR1); // set CodingRate
        setRegister(REG_MODEM_CONFIG1, MSK_MODEM_IMPLICITHDR, 0); // Explicit
        setRegister(REG_MODEM_CONFIG2, MSK_MODEM_SF, VAL_MODEM_SF06); // set SpreadingFactor
        // setRegister(REG_MODEM_CONFIG3, MSK_LOW_DATARATE_OPTI, 255); // Mandatory when symbollength > 16ms
        setRegister(REG_MODEM_CONFIG2, MSK_TX_CONTINOUOS, 0); // single Packet sending
        setRegister(REG_MODEM_CONFIG2, MSK_RX_PAYLOAD_CRC_ON,MSK_RX_PAYLOAD_CRC_ON); // CRC on
        spiWrite(REG_HOP_PERIOD, 0); // 0: disable
        setRegister(REG_TCXO, MSK_TCXO_ON, 0);

        ///
        return 0; // all OK
    };


    /**< Go into powersaving standby */
    void setEnabled(uint8_t enabled = 1)
    {
        if (enabled)    _mode = VAL_MODE_SLEEP;
        else            _mode = VAL_MODE_STANDBY;
        setRegister(REG_OPMODE, MSK_OPMODE_MODE, _mode);
    };

    void setPreambleLength(uint16_t length = 8)
    {
        spiWrite(REG_PREAMBLE_MSB, (length >> 8)&0xFF);
        spiWrite(REG_PREAMBLE_LSB, (length&0xFF));
    };

    void setFrequency(uint32_t kHz = 868000)
    {
        kHz = uint32_t(RFM_FINV * float(kHz) * 1000.0);
        spiWrite(REG_REGFRFLSB, kHz & 0xFF);
        kHz = kHz >> 8;
        spiWrite(REG_REGFRFMID, kHz & 0xFF);
        kHz = kHz >> 8;
        spiWrite(REG_REGFRFMSB, kHz & 0xFF);
    };

    uint32_t getFrequency()
    {
        uint32_t frf;
        frf = spiRead(REG_REGFRFMSB);
        frf = (frf<<8) | spiRead(REG_REGFRFMID);
        frf = (frf<<8) | spiRead(REG_REGFRFLSB);
        frf = uint32_t(RFM_FSTEP * float(frf) / 1000.0);
        return frf;
    };

    void setIMax(uint8_t mA = 50) /// zero turns current protection off
    {
        if (mA > 120)   mA = ((mA - 45) / 5);
        else if (mA)    mA = ((mA + 30) / 10);
        else
        {
            setRegister(REG_OCP, MSK_OCP_ON, 0);
            return;
        }
        setRegister(REG_OCP, MSK_OCP_ON, 255);
        setRegister(REG_OCP, MSK_OCP_TRIM,mA);
    };

    void setPMax(uint8_t dBm = 10)
    {
        if (dBm > 20) dBm = 20;
        uint8_t paBoost = 0;
        uint8_t paMax = 0;

        if (dBm > 14)
        {
            paBoost = 255;
            paMax = 255;
            dBm -= 5;
        }
        else if (dBm > 11)  paMax = 255;
        else                dBm += 4;

        setRegister(REG_PA_CONFIG,MSK_PA_SELECT, paBoost);
        setRegister(REG_PA_CONFIG,MSK_PA_MAX_POWER,paMax);
        setRegister(REG_PA_CONFIG,MSK_PA_OUT_POWER, dBm&MSK_PA_OUT_POWER);

        if (dBm == 15)  dBm = VAL_PA_DAC_20DBM; /// when outputpower = 111
        else            dBm = VAL_PA_DAC_DEFAULT;
        setRegister(REG_PA_DAC, MSK_PA_DAC, dBm);
    };

    void setLNA()
    {
        setRegister(REG_LNA, MSK_LNA_GAIN, B00100000);
        setRegister(REG_LNA, MSK_LNA_BOOST_HF, VAL_LNA_BOOST_HF_ON);
        setRegister(REG_MODEM_CONFIG3, MSK_AGC_AUTO_ON, MSK_AGC_AUTO_ON);
    };

    void setBandwidth(uint16_t kHz = 500)
    {
        if      (kHz > 300) kHz = VAL_MODEM_BW500;
        else if (kHz > 200) kHz = VAL_MODEM_BW250;
        else if (kHz > 100) kHz = VAL_MODEM_BW125;
        else if (kHz >  50) kHz = VAL_MODEM_BW063;
        else if (kHz >  35) kHz = VAL_MODEM_BW042;
        else if (kHz >  25) kHz = VAL_MODEM_BW031;
        else if (kHz >  18) kHz = VAL_MODEM_BW021;
        else if (kHz >  13) kHz = VAL_MODEM_BW016;
        else if (kHz >   9) kHz = VAL_MODEM_BW010;
        else                kHz = VAL_MODEM_BW008;
        setRegister(REG_MODEM_CONFIG1, MSK_MODEM_BW, kHz);
    };

    void handleIRQ()
    {
        uint8_t flags = spiRead(REG_IRQ_FLAGS);
        if (flags & MSK_IRQ_RX_TIMEOUT)
        {
            flags &= !MSK_IRQ_RX_TIMEOUT;
            Serial.println("  RX_Timeout");
        }
        if (flags & MSK_IRQ_RX_DONE)
        {
            flags &= !MSK_IRQ_RX_DONE;    /// valid header CRC  --> set the RxDone interrupt
            Serial.println("  RX_Done");
            startIdleState();
        }
        if (flags & MSK_IRQ_PAYLOAD_CRC_ERR) ///
        {
            flags &= !MSK_IRQ_PAYLOAD_CRC_ERR;
            Serial.println("  CRC_Error");
            startIdleState();
        }
        if (flags & MSK_IRQ_VALID_HEADER)
        {
            flags &= !MSK_IRQ_VALID_HEADER;
            Serial.println("  Valid HDR");
        }
        if (flags & MSK_IRQ_TX_DONE)    /// Packet is out
        {
            flags &= !MSK_IRQ_TX_DONE;
            Serial.println("  TX Done");
            _mode = VAL_MODE_SLEEP;
            startIdleState();
        }
        if (flags & MSK_IRQ_CAD_DONE)
        {
            flags &= !MSK_IRQ_CAD_DONE;
            Serial.println("  CAD Done");
        }
        if (flags & MSK_IRQ_FHSS_CHANGE)
        {
            flags &= !MSK_IRQ_FHSS_CHANGE;
            Serial.println("  FHSS Change");
        }
        if (flags & MSK_IRQ_CAD_DETECTED)
        {
            flags &= !MSK_IRQ_CAD_DETECTED;
            Serial.println("  CAD Detected");
        }
        spiWrite(REG_IRQ_FLAGS, flags);
    };

    void setIdleState(uint8_t state = VAL_MODE_SLEEP)
    {
        _idleState = state;
    };

    void startIdleState()
    {
        if      (_idleState == VAL_MODE_STANDBY)    setEnabled(0);
        else if (_idleState == VAL_MODE_SLEEP)      setEnabled(1);
        else if (_idleState == VAL_MODE_RX_SINGLE)  receiveDataSingle();
        else if (_idleState == VAL_MODE_RX_CONT)    receiveDataCont();
    };


// setMode()
// setRXTimeout()

    uint8_t canSend()
    {
        if (_mode == VAL_MODE_TX)   return 0;
        else                        return 1;
    };

    void sendData()
    {
        if (_mode == VAL_MODE_TX) return; /// early bailout

        setEnabled(0); // goto Standby
        spiWrite(REG_FIFO_ADDR_PTR, 0); // Set FifoPtrAddr to FifoTxPtrBase.
        spiWrite(REG_FIFO_TX_BASE_AD, 0x00); // ToDo: only here to test
        Serial.println("Enter Send-Mode");
        /// TODO
        uint8_t length = 10;
        spiWrite(REG_PAYLOAD_LENGTH, length); // Write PayloadLength bytes to the FIFO (RegFifo)
        while (length)
        {
            spiWrite(REG_FIFO, length--); //put content TODO
        }

        setRegister(REG_DIO_MAPPING_1, MSK_DIO0_MAPPING, 0x00); // Packet Sent IRQ
        /// TODO: activate IRQ
        setRegister(REG_OPMODE, MSK_OPMODE_MODE, VAL_MODE_TX);
        _mode = VAL_MODE_TX;
    };

    void filterCRC(uint8_t enable)
    {
        _filterBadCRC = enable && 1;
    }; /// TODO: do something with it

    void receiveDataSingle()
    {
        if ((_mode == VAL_MODE_RX_CONT) || (_mode == VAL_MODE_RX_SINGLE)) return;
        setEnabled(0);
        Serial.println("Enter Rec-Mode"); /// TODO: only test
        spiWrite(REG_FIFO_ADDR_PTR, 0); // Set FifoAddrPtr to FifoRxBaseAddr.
        //2 Static configuration register device can be written in either Sleep mode, Stand-by mode or FSRX mode.
        //3 A single packet receive operation is initiated by selecting the operating mode RXSINGLE.
        //4 The receiver will then await the reception of a valid preamble. Once received, the gain of the receive chain is set. Following the ensuing reception of a valid header, indicated by the ValidHeader interrupt in explicit mode. The packet reception process commences. Once the reception process is complete the RxDone interrupt is set. The radio then returns automatically to Stand-by mode to reduce power consumption.
        //5 The receiver status register PayloadCrcError should be checked for packet payload integrity.
        //6 If a valid packet payload has been received then the FIFO should be read (See Payload Data Extraction below). Should a subsequent single packet reception need to be triggered, then the RXSINGLE operating mode must be re-selected to launch the receive process again - taking care to reset the SPI pointer (FifoAddrPtr) to the base location in memory (FifoRxBaseAddr).
        if (_filterBadCRC)   setRegister(REG_DIO_MAPPING_1, MSK_DIO0_MAPPING, B01000000); // CRC OK
        else                setRegister(REG_DIO_MAPPING_1, MSK_DIO0_MAPPING, B00000000); // Payload Ready

        setRegister(REG_OPMODE, MSK_OPMODE_MODE, VAL_MODE_RX_SINGLE);
        _mode = VAL_MODE_RX_CONT;
    };

    void receiveDataCont()
    {
        if ((_mode == VAL_MODE_RX_CONT) || (_mode == VAL_MODE_RX_SINGLE)) return;

        Serial.println("Enter Rec-Mode"); /// TODO: only test
        setEnabled(0);
        spiWrite(REG_FIFO_RX_BASE_AD, 0x00);
        spiWrite(REG_FIFO_ADDR_PTR, 0x00);

        if (_filterBadCRC)  setRegister(REG_DIO_MAPPING_1, MSK_DIO0_MAPPING, B01000000); // CRC OK
        else                setRegister(REG_DIO_MAPPING_1, MSK_DIO0_MAPPING, B00000000); // Payload Ready

        /// activate IRQ
        setRegister(REG_OPMODE, MSK_OPMODE_MODE, VAL_MODE_RX_CONT);
        _mode = VAL_MODE_RX_CONT;
    };

    void extractFifo()
    {
        // ValidHeader, PayloadCrcError, RxDone and RxTimeout should not be set
        // RegRxNbBytes Indicates the number of bytes that have been received thus far.
        // RegFifoAddrPtr is a dynamic pointer that indicates precisely where the Lora modem received data has been written up
        // to.
        // Set RegFifoAddrPtr to RegFifoRxCurrentAddr. This sets the FIFO pointer to the location of the last packet received in
        // the FIFO. The payload can then be extracted by reading the register RegFifo, RegRxNbBytes times.
        // Alternatively, it is possible to manually point to the location of the last packet received, from the start of the current
        // packet, by setting RegFifoAddrPtr to RegFifoRxByteAddr minus RegRxNbBytes. The payload bytes can then be read
        // from the FIFO by reading the RegFifo address RegRxNbBytes times.
    };

    /*
    #define REG_FIFO                    0x00    // cleared when in SleepMode
    #define REG_FIFO_RX_CURRENT_ADDR    0x10    // start of last packet received

    #define REG_IRQ_FLAGS_MASK          0x11
    #define REG_IRQ_FLAGS               0x12
    #define     MSK_IRQ_RX_TIMEOUT      (B10000000)
    #define     MSK_IRQ_RX_DONE         (B01000000)
    #define     MSK_IRQ_PAYLOAD_CRC_ERR (B00100000)
    #define     MSK_IRQ_VALID_HEADER    (B00010000)
    #define     MSK_IRQ_TX_DONE         (B00001000)
    #define     MSK_IRQ_CAD_DONE        (B00000100)
    #define     MSK_IRQ_FHSS_CHANGE     (B00000010)
    #define     MSK_IRQ_CAT_DETECTED    (B00000001)


    #define REG_RX_NB_BYTES             0x13    // successful receive will write this bytecount

    #define REG_RX_HEADER_CNT_MSB       0x14
    #define REG_RX_HEADER_CNT_LSB       0x15
    #define REG_RX_PACKET_CNT_MSB       0x16
    #define REG_RX_PACKET_CNT_LSB       0x17

    #define REG_MODEM_STAT              0x18
    #define     MSK_MODEM_RX_CODINGRATE (B11100000)
    #define     MSK_MODEM_STATUS        (B00011111)
    #define     MSK_MODEM_STATUS_CLEAR  (B00010000)
    #define     MSK_MODEM_STATUS_HDRVAL (B00001000)
    #define     MSK_MODEM_STATUS_RXON   (B00000100)
    #define     MSK_MODEM_STATUS_SYNC   (B00000010)
    #define     MSK_MODEM_STATUS_SIGDET (B00000001)

    #define REG_PKT_SNR_VALUE           0x19    // last Packets SNR dB = int8_REG/4
    #define REG_PKT_RSSI_VALUE          0x1A    // last Packets RSSI dBm = REG - 127
    #define REG_RSSI_VALUE              0x1B    // current RSSI dBm = REG - 127

    #define REG_HOP_CHANNEL             0x1C
    #define     MSK_HOP_PLL_TIMEOUT     (B10000000)
    #define     MSK_HOP_RX_CRCON        (B01000000)
    #define     MSK_HOP_FHSS_CHANNEL    (B00111111)


    #define REG_SYMB_TIMEOUTLSB         0x1F    // number of symbols; timeout = REG * Ts

    #define REG_FIFO_RX_BYTE_ADDR       0x25    // Addr of last byte written

    /// 0x27 - 3F not for LoRa

    #define REG_DIO_MAPPING_1           0x40
    #define     MSK_DIO0_MAPPING        (B11000000)
    #define     MSK_DIO1_MAPPING        (B00110000)
    #define     MSK_DIO2_MAPPING        (B00001100)
    #define     MSK_DIO3_MAPPING        (B00000011)
    #define REG_DIO_MAPPING_2           0x41
    #define     MSK_DIO4_MAPPING        (B11000000)
    #define     MSK_DIO5_MAPPING        (B00110000)
    #define     MSK_DIO_MAP_PREAMB_DET  (B00000001)
    #define REG_VERSION                 0x42
    #define     VAL_V1B                 0x12 // has errors --> extra document (errata)

    #define REG_FORMER_TEMP             0x5B    // -1°C per LSB

    #define REG_AGC_REF                 0x61
    #define     MSK_AGC_REF_LEVEL       (B00111111) // def=0x19
    #define REG_AGC_THRESH1             0x62
    #define     MSK_AGC_STEP1           0x0F
    #define REG_AGC_THRESH2             0x63
    #define     MSK_AGC_STEP2           0xF0
    #define     MSK_AGC_STEP3           0x0F
    #define REG_AGC_THRESH3             0x64
    #define     MSK_AGC_STEP4           0xF0
    #define     MSK_AGC_STEP5           0x0F
    #define REG_PLL                     0x70
    #define     MSK_PLL_BW              (B11000000)
    #define         VAL_PLL_BW075KHZ    (B00000000)
    #define         VAL_PLL_BW150KHZ    (B01000000)
    #define         VAL_PLL_BW225KHZ    (B10000000)
    #define         VAL_PLL_BW300KHZ    (B11000000)

    */



};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif
