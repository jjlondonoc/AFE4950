#include <AFE4950.h>
#include <SPI.h>


AFE4950::AFE4950() {}
AFE4950::~AFE4950() {}

// Hardware reset (LOW state in RESETZ pin)
void AFE4950::reset() {
    digitalWrite(_pin_AFE_RST, LOW);
    delayMicroseconds(50);
    digitalWrite(_pin_AFE_RST, HIGH);
    delay(1000);
    _reg0.data = 0x000000;
    _reg1.data = 0x000000;
}

// Clear all registers, except page 1 registers
void AFE4950::softwareReset() {
    _reg0.data = 0x000008;
    writeRegister(_reg0);
    delay(1000);

    switchPage(page0);
    
    _reg0.data = 0x000002;
    writeRegister(_reg0);
}

void AFE4950::switchPage(Page page) {
    switch(page) {
        case page0:
        _reg1.data &= ~(1 << 0);  // Set 0 in bit 0 for switch page 0 (Address 0x01)
        break;
        case page1:
        _reg1.data |= (1 << 0);   // set 1 in bit 1 for switch page 1 (Address 0x01)
        break;
    }

    writeRegister(_reg1);
}

void AFE4950::switchReadWriteMode(Mode mode) {
    switch(mode) {
        case write:
        _reg0.data &= ~(1 << 0);  // Set 0 in bit 0 for write (Address 0x00)
        break;
        case read:
        _reg0.data |= (1 << 0);   // Set 1 in bit 0 for read (Address 0x00)
        break;
    }

    writeSPI(_reg0);
}

void AFE4950::writeSPI(const Register& reg) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    digitalWrite(_pinCS, LOW);

    // Send register address
    SPI.transfer(reg.address);

    // Send 3 bytes (MSB first)
    SPI.transfer((reg.data >> 16) & 0xFF); // MSB
    SPI.transfer((reg.data >> 8) & 0xFF); 
    SPI.transfer(reg.data & 0xFF);         // LSB

    digitalWrite(_pinCS, HIGH);
    
    SPI.endTransaction();
}

uint32_t AFE4950::readSPI(uint8_t regAddress) {
    uint32_t data = 0;
    if(regAddress != 0x00) {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

        digitalWrite(_pinCS, LOW);

        // Send register address
        SPI.transfer(regAddress);

        // Send dummy bytes and receive data
        uint8_t byte1 = SPI.transfer(0x00); // MSB
        uint8_t byte2 = SPI.transfer(0x00); 
        uint8_t byte3 = SPI.transfer(0x00); // LSB

        data = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;

        digitalWrite(_pinCS, HIGH);

        SPI.endTransaction();
    }
    else {
        data = _reg0.data;
    }
    return data;
}

void AFE4950::writeRegister(const Register& reg) {
    uint32_t controlBit = _reg0.data & (1 << 0);
    if(controlBit != 0) {
        switchReadWriteMode(write);
    }
    if(reg.address == 0x00) {
        _reg0.data = reg.data;
    }
    else if(reg.address == 0x01) {
        _reg1.data = reg.data;
    }

    writeSPI(reg);
}

uint32_t AFE4950::readRegister(uint8_t regAddress) {
    uint32_t controlBit = _reg0.data & (1 << 0);
    if(controlBit != (1 << 0)) {
        switchReadWriteMode(read);
    }
    return readSPI(regAddress);
}

/*
* Only read registers (don't validate):
* (0x28), (0x30), (0x6D), (0xAE), (0xBC), (0x9A)
*/
bool AFE4950::validateRegisters() {
    bool success = true;

    switchPage(page1);
    for(size_t i = 0; i < page_1_registers_size; ++i) {
        uint32_t regValue = readRegister(page_1_registers[i].address);
        if(regValue != page_1_registers[i].data) {
            success = false;
        }
    }

    switchPage(page0);
    for(size_t i = 0; i < page_0_registers_size; ++i) {
        if(page_0_registers[i].address != 0x28 && page_0_registers[i].address != 0x30 && page_0_registers[i].address != 0x6D
            && page_0_registers[i].address != 0xAE && page_0_registers[i].address != 0xBC && page_0_registers[i].address != 0x9A) {
                
            uint32_t regValue = readRegister(page_0_registers[i].address);
            if(regValue != page_0_registers[i].data) {
                success = false;
            }
        }
    }

    Register fifoReg = {0x1D, 0xE00140}; // FIFO config
    if(readRegister(fifoReg.address) != fifoReg.data) {
        success = false;
    }

    return success;
}

// Sequence for reconfigure FIFO
void AFE4950::fifoConfig() {
    switchPage(page0);
    _reg0.data = 0x000002;
    writeRegister(_reg0);
    _reg0.data = 0x000040;
    writeRegister(_reg0);

    Register fifoConf = {0x1D, 0xE00140};
    writeRegister(fifoConf); // Write at the end always
}

bool AFE4950::configure(
    const uint8_t pinSCLK,
    const uint8_t pinMISO,
    const uint8_t pinMOSI,
    const uint8_t pinCS,
    const uint8_t pin_AFE_RST,
    const uint8_t pin_FIFO_RDY
) {
    _pinSCLK = pinSCLK;
    _pinMISO = pinMISO;
    _pinMOSI = pinMOSI;
    _pinCS = pinCS;
    _pin_AFE_RST = pin_AFE_RST;
    _pin_FIFO_RDY = pin_FIFO_RDY;

    pinMode(_pinCS, OUTPUT);
    pinMode(_pin_AFE_RST, OUTPUT);
    digitalWrite(_pinCS, HIGH);
    digitalWrite(_pin_AFE_RST, HIGH);

    pinMode(_pin_FIFO_RDY, INPUT);

    SPI.begin(_pinSCLK, _pinMISO, _pinMOSI, _pinCS);

    delay(1);

    reset();
    softwareReset();

    switchPage(page0);
    for (size_t i = 0; i < page_0_registers_size; ++i) {
        writeRegister(page_0_registers[i]);
    }

    switchPage(page1);
    for (size_t i = 0; i < page_1_registers_size; ++i) {
        writeRegister(page_1_registers[i]);
    }

    fifoConfig();

    return validateRegisters();
}

/*
* Reset and reconfigure FIFO
* Enable microcontroller interrupts
*/
void AFE4950::enableCapture() {
    if(_captureStatus == STOPPED) {
        switchPage(page0);
        _reg0.data = 0x000002;
        writeRegister(_reg0);
        _reg0.data = 0x000040;
        writeRegister(_reg0);

        attachInterruptArg(
            digitalPinToInterrupt(_pin_FIFO_RDY),
            dataReadyISR,
            this,
            RISING
        );

        _captureStatus = RUNNING;
    }
}

void AFE4950::disableCapture() {
    if(_captureStatus == RUNNING) {
        detachInterrupt(digitalPinToInterrupt(_pin_FIFO_RDY));
        _captureStatus = STOPPED;
    }
}

void IRAM_ATTR AFE4950::dataReadyISR(void* arg) {
    AFE4950* afe = static_cast<AFE4950*>(arg);
    if(!afe -> _dataReadyFlag) {
        afe -> _dataReadyFlag = true;
    }
}

// Prepare buffer with tags
void AFE4950::copyDataInTxBuff(uint32_t data, uint8_t identifier) {
    if (_txDataStruct.dataBufferIndex + 4 <= _dataBufferSize) {
        _txDataStruct.dataBuffer[_txDataStruct.dataBufferIndex++] = data & 0xFF;         // LSB
        _txDataStruct.dataBuffer[_txDataStruct.dataBufferIndex++] = (data >> 8) & 0xFF; 
        _txDataStruct.dataBuffer[_txDataStruct.dataBufferIndex++] = (data >> 16) & 0xFF; // MSB
        _txDataStruct.dataBuffer[_txDataStruct.dataBufferIndex++] = identifier;          // Tag
    }
    else {
        _txDataStruct.dataBufferIndex = 0;
    }
}

// Read out FIFO
void AFE4950::capture() {
    if(_captureStatus == RUNNING && _dataReadyFlag) {
        _dataReadyFlag = false;

        uint32_t pointerDiff = readRegister(0x6D) & 0xFF;
        uint32_t numWordsToRead = (pointerDiff + 1) & 0xFF;

        _txDataStruct.dataBufferIndex = 0;
        copyDataInTxBuff(0x000000, 0x02);    // Header to indicate new sampling window
        copyDataInTxBuff(pointerDiff, 0x03); // POINTER_DIFF value

        while(numWordsToRead != 0) {
            uint32_t data = readRegister(0xFF);
            copyDataInTxBuff(data, 0x05);
            numWordsToRead--;
        }
        _dataAvailable = true;
    }
}

// Get number of bytes to be sent in buffer
size_t AFE4950::getBytesToSend() {
    return _txDataStruct.dataBufferIndex;
}

// Get status of data reading
bool AFE4950::ready() {
    capture();
    return _dataAvailable;
}

// When FIFO is readed out return data available
const uint8_t* AFE4950::getData() {
    _dataAvailable = false;
    return _txDataStruct.dataBuffer;
}