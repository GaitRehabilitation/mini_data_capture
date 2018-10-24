#include "Arduino.h"

#include<Wire.h>
#include <SPI.h>
#include <SD.h>

const int MPU_addr = 0x68; // I2C address of the MPU-6050
const int LOGGING_BUTTON_PIN = 2; // logger button
const int CHIP_SELECT_PIN = 10;

void sleepNow();

char fileName[13];
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int loggingButton = 0;
volatile bool wakeUpFlag = false;

void setup ()
{
    //set sensitivity @ address 1C
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x1C);
    Wire.write(B00010000);   //here is the byte for sensitivity (4g here)
    Wire.endTransmission(true);


    Wire.beginTransmission(MPU_addr);
    Wire.write(0x1B);
    Wire.write(B00010000);   //here is the byte for sensitivity (1000 degree sec)
    Wire.endTransmission(true);

    pinMode(LOGGING_BUTTON_PIN, INPUT);
    sleepNow();
}

void wakeUpNow()        // here the interrupt is handled after wakeup
{
    wakeUpFlag = true;
}


void  sleepNow(){

    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(B010000);     // set to mpu to sleep to avoid consuming power
    Wire.endTransmission(true);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

    sleep_enable(); // enable sleep

    attachInterrupt(digitalPinToInterrupt(LOGGING_BUTTON_PIN), wakeUpNow, RISING); // set wakeup flag when button is pressed

    sleep_mode(); // put processor to sleep

    sleep_disable(); //disable sleep

    detachInterrupt(digitalPinToInterrupt(LOGGING_BUTTON_PIN)); // disables interrupt 0 on pin 2 to avoid setting flag

    delay(1000);
}

void loop() {
    if(wakeUpFlag){
        int count = 0;
        while(true){
            sprintf_P(fileName, PSTR("LOG%05u.TXT"), count);
            if (!SD.exists(fileName)){
                break;
            }

            if (count > 65533) //There is a max of 65534 logs
            {
                while(1);
            }
            count++;
        }

        Wire.begin();
        Wire.beginTransmission(MPU_addr);
        Wire.write(0x6B);  // PWR_MGMT_1 register
        Wire.write(0);     // set to zero (wakes up the MPU-6050)
        Wire.endTransmission(true);
        wakeUpFlag = false;
    } else {

        if (SD.begin(CHIP_SELECT_PIN)) {
            Wire.beginTransmission(MPU_addr);
            Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
            Wire.endTransmission(false);
            Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
            AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
            AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) a& 0x3E (ACCEL_YOUT_L)
            AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
            Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
            GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
            GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
            GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

            File dataFile = SD.open(fileName, FILE_WRITE);
            if (dataFile) {
                dataFile.print(millis());
                dataFile.print(",");
                dataFile.print(((double)AcX)/8192.0);
                dataFile.print(",");
                dataFile.print(((double)AcY)/8192.0);
                dataFile.print(",");
                dataFile.print(((double)AcZ)/8192.0);
                dataFile.print(",");
                dataFile.print(Tmp);
                dataFile.print(",");
                dataFile.print(((double)GyX)/32.8);
                dataFile.print(",");
                dataFile.print(((double)GyY)/32.8);
                dataFile.print(",");
                dataFile.print(((double)GyZ)/32.8);
                dataFile.print("\n");
                dataFile.close();
            }
            delay(10);
        } else {
            // put logger to sleep if no SD card is present
            sleepNow();
        }
    }

    loggingButton = digitalRead(LOGGING_BUTTON_PIN);
    if(loggingButton == HIGH){
        sleepNow();
    }
}