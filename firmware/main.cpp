
#include "Arduino.h"

#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>

const int MPU_addr = 0x68;        // I2C address of the MPU-6050
const int LOGGING_BUTTON_PIN = 2; // logger button
const int CHIP_SELECT_PIN = 10;

const uint32_t LOG_INTERVAL_USEC = 2000;

const uint8_t BUFFER_BLOCK_COUNT = 10;

const uint32_t FILE_BLOCK_COUNT = 256000;

char fileName[13];
int loggingButton = 0;
volatile bool wakeUpFlag = false;

SdFat sd;

SdBaseFile binFile;

const uint8_t ACCEL_DIM = 3;
const uint8_t GYRO_DIM = 3;
struct data_t
{
    uint32_t time;
    int16_t accel[ACCEL_DIM];
    int16_t gyro[GYRO_DIM];
    int16_t temp;
};

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4) / sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM * sizeof(data_t);

struct block_t
{
    uint16_t count;
    uint16_t overrun;
    data_t data[DATA_DIM];
    uint8_t fill[FILL_DIM];
};

void setup();
void wakupInterrupt();
void processorSleep();
void acquireData(data_t *data);
void createBin();
void configureMPU();
bool recordBinData();

void setup()
{
    Wire.begin();
    pinMode(LOGGING_BUTTON_PIN, INPUT);
    processorSleep();
}

void wakupInterrupt() // here the interrupt is handled after wakeup
{
    wakeUpFlag = true;
}

void processorSleep()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x06);
    Wire.write(B01000000); // set to mpu to sleep to avoid consuming power
    Wire.endTransmission(true);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);                                                // sleep mode is set here
    sleep_enable();                                                                     // enable sleep
    attachInterrupt(digitalPinToInterrupt(LOGGING_BUTTON_PIN), wakupInterrupt, RISING); // set wakeup flag when button is pressed
    sleep_mode();                                                                       // put processor to sleep
    sleep_disable();                                                                    //disable sleep
    detachInterrupt(digitalPinToInterrupt(LOGGING_BUTTON_PIN));                         // disables interrupt 0 on pin 2 to avoid setting flag
    delay(1000);
}

void createBin()
{
    // max number of blocks to erase per erase call
    const uint32_t ERASE_SIZE = 262144L;
    uint32_t bgnBlock, endBlock;
  

    if (!sd.begin(CHIP_SELECT_PIN, SD_SCK_MHZ(50)))
    {
        processorSleep();
    }
    else
    {
        int count = 0;
        while (true)
        {
            sprintf_P(fileName, PSTR("LOG%05u.BIN"), count);
            if (count > 65533) //There is a max of 65534 logs
            {
                break;
            }
            if (!sd.exists(fileName))
            {
                // create a new bin file if exsists
                binFile.close();
                if (!binFile.createContiguous(fileName, 512 * FILE_BLOCK_COUNT))
                {
                    break;
                }

                // Get the address of the file on the SD.
                if (!binFile.contiguousRange(&bgnBlock, &endBlock))
                {
                    break;
                }

                // Flash erase all data in the file.
                uint32_t bgnErase = bgnBlock;
                uint32_t endErase;
                while (bgnErase < endBlock)
                {
                    endErase = bgnErase + ERASE_SIZE;
                    if (endErase > endBlock)
                    {
                        endErase = endBlock;
                    }
                    if (!sd.card()->erase(bgnErase, endErase))
                    {
                        break;
                    }
                    bgnErase = endErase + 1;
                }

                break;
            }
            count++;
        }
    }
}

void configureMPU()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x06); // PWR_MGMT_1 register
    Wire.write(0);    // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_addr);
    Wire.write(0x7F);
    Wire.write(B00100000); //SELECT BANK 2
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_addr);
    Wire.write(0x14);
    Wire.write(B0000011); //here is the byte for sensitivity (4g here)
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_addr);
    Wire.write(0x01);
    Wire.write(B00000101); //here is the byte for sensitivity (1000 degree sec)
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_addr);
    Wire.write(0x7F);
    Wire.write(0);
    Wire.endTransmission(true);
}

void loop()
{
    if (wakeUpFlag)
    {

        createBin();
        configureMPU();
        wakeUpFlag = false;
    }
    else
    {
        while(recordBinData()){};
        processorSleep();
    }
}

bool recordBinData(){

    const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 1;
    // Index of last queue location.
    const uint8_t QUEUE_LAST = QUEUE_DIM - 1;

    // Allocate extra buffer space.
    block_t block[BUFFER_BLOCK_COUNT - 1];

    block_t *curBlock = 0;

    block_t *emptyStack[BUFFER_BLOCK_COUNT];
    uint8_t emptyTop;
    uint8_t minTop;

    block_t *fullQueue[QUEUE_DIM];
    uint8_t fullHead = 0;
    uint8_t fullTail = 0;

    emptyStack[0] = (block_t *)sd.vol()->cacheClear();
    if (emptyStack[0] == 0)
    {
        return false;
    }
    // Put rest of buffers on the empty stack.
    for (int i = 1; i < BUFFER_BLOCK_COUNT; i++)
    {
        emptyStack[i] = &block[i - 1];
    }
    emptyTop = BUFFER_BLOCK_COUNT;
    minTop = BUFFER_BLOCK_COUNT;

    // Start a multiple block write.
    if (!sd.card()->writeStart(binFile.firstBlock()))
    {
        return false;
    }

    bool closeFile = false;
    uint32_t bn = 0;
    uint32_t maxLatency = 0;
    uint32_t overrun = 0;
    //   uint32_t overrunTotal = 0;
    uint32_t logTime = micros();
    while (1)
    {
        // Time for next data record.
        logTime += LOG_INTERVAL_USEC;
        
        //check if logging button is pressed again
        loggingButton = digitalRead(LOGGING_BUTTON_PIN);
        if (loggingButton == HIGH)
        {
            closeFile = true;
        }

        if (closeFile)
        {
            if (curBlock != 0)
            {
                // Put buffer in full queue.
                fullQueue[fullHead] = curBlock;
                fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
                curBlock = 0;
            }
        }
        else
        {
            if (curBlock == 0 && emptyTop != 0)
            {
                curBlock = emptyStack[--emptyTop];
                if (emptyTop < minTop)
                {
                    minTop = emptyTop;
                }
                curBlock->count = 0;
                curBlock->overrun = overrun;
                overrun = 0;
            }
            // while ((int32_t)(logTime - micros()) < 0)
            // {
            //     delayMicroseconds(100);
            // }
            int32_t delta;
            do
            {
                delta = micros() - logTime;
            } while (delta < 0);
            if (curBlock != 0)
            {
                acquireData(&curBlock->data[curBlock->count++]);
                if (curBlock->count == DATA_DIM)
                {
                    fullQueue[fullHead] = curBlock;
                    fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
                    curBlock = 0;
                }
            }
        }
        if (fullHead == fullTail)
        {
            // Exit loop if done.
            if (closeFile)
            {
                break;
            }
        }
        else if (!sd.card()->isBusy())
        {
            // Get address of block to write.
            block_t *pBlock = fullQueue[fullTail];
            fullTail = fullTail < QUEUE_LAST ? fullTail + 1 : 0;
            // Write block to SD.
            uint32_t usec = micros();
            if (!sd.card()->writeData((uint8_t *)pBlock))
            {
                break;
            }
            usec = micros() - usec;
            if (usec > maxLatency)
            {
                maxLatency = usec;
            }
            // Move block to empty queue.
            emptyStack[emptyTop++] = pBlock;
            bn++;
            if (bn == FILE_BLOCK_COUNT)
            {
                createBin();
                return true;
            }
        }
    }
    return false;
}

void acquireData(data_t *data)
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x2D); // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);            // request a total of 14 registers
    data->accel[0] = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    data->accel[1] = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) a& 0x3E (ACCEL_YOUT_L)
    data->accel[2] = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    data->gyro[0] = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    data->gyro[1] = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    data->gyro[2] = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    data->temp = Wire.read() << 8 | Wire.read();     // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    data->time = micros(); // set time
}