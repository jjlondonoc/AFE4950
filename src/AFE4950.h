#ifndef __AFE4950_H__
#define __AFE4950_H__

#include <Arduino.h>
#include <Registers.h>

class AFE4950 {

    public:

        AFE4950();
        ~AFE4950();
        bool configure(
            const uint8_t pinSCLK,
            const uint8_t pinMISO,
            const uint8_t pinMOSI,
            const uint8_t pinCS,
            const uint8_t pin_AFE_RST,
            const uint8_t pin_FIFO_RDY
        );
        void enableCapture();
        void disableCapture();
        size_t getBytesToSend();
        bool ready();
        const uint8_t* getData();
        
    private:

        uint8_t _pinSCLK;
        uint8_t _pinMISO;
        uint8_t _pinMOSI;
        uint8_t _pinCS;
        uint8_t _pin_AFE_RST;
        uint8_t _pin_FIFO_RDY;
        
        Register _reg0 = {0x00, 0x000000};  // Read/Write mode control
        Register _reg1 = {0x01, 0x000000};  // Page select control

        static const size_t _dataBufferSize = 2048; // Including FIFO size (256) and header and tags
        struct TxDataStruct {
            uint8_t dataBuffer[_dataBufferSize];
            size_t dataBufferIndex;
        };
        TxDataStruct _txDataStruct = {{}, 0};

        volatile bool _dataAvailable = false;
        volatile bool _dataReadyFlag = false;

        enum Page: uint8_t {
            page0,
            page1
        };

        enum Mode: uint8_t {
            write,
            read
        };

        enum CaptureStatus {
            STOPPED,
            RUNNING
        };
        
        CaptureStatus _captureStatus = STOPPED;

        void switchPage(Page page);             // Register 0x01
        void switchReadWriteMode(Mode mode);    // Register 0x00

        void writeSPI(const Register& reg);
        uint32_t readSPI(uint8_t regAddress);
        void writeRegister(const Register& reg);
        uint32_t readRegister(uint8_t regAddress);

        bool validateRegisters();
        void fifoConfig();

        void softwareReset();
        void reset();

        static void IRAM_ATTR dataReadyISR(void* arg);
        void copyDataInTxBuff(uint32_t data, uint8_t identifier);
        void capture();  
};

#endif // __AFE4950_H__