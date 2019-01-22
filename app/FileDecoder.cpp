//
// Created by michaelpollind on 1/21/19.
//

#include "FileDecoder.h"

#include <QFile>
#include <QDir>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTextStream>
#include <QDebug>

// conversion factors
// https://www.invensense.com/products/motion-tracking/9-axis/icm-20948/
//#define ACCEL_SENSIVITY 16384 // +-2g
//#define ACCEL_SENSIVITY 8192 // +-4g
#define ACCEL_SENSIVITY 4096 // +-8g
//#define ACCEL_SENSIVITY 2048 // +-16g

//#define GYRO_SENSIVITY 131  // +-250  deg/s
//#define GYRO_SENSIVITY 65.5 // +-500  deg/s
#define GYRO_SENSIVITY 32.8 // +-1000 deg/s
//#define GYRO_SENSIVITY 16.4 // +-2000 deg/s


FileDecoder::FileDecoder(const QString& path,QObject* obj ): QObject::QObject(obj),_path(path){

}

void FileDecoder::run() {

    QFile file(_path);
    qint64 samples = 0;

    if(!file.open(QIODevice::ReadOnly)){
        return;
    }
    char payload[512];
    QFileInfo info(_path);

    QFile newFile(QString("%1/%2.csv").arg(info.dir().absolutePath()).arg(info.baseName()));
    if(!newFile.open(QIODevice::NewOnly)){
        QMessageBox::critical(0,"Error","File Already Exists!");
        return;
    }
    QTextStream out(&newFile);
    out << "epoch(ms),gyro_x(fdps),gyro_y(fdps),gyro_z(fdps)" << '\n';
    while (file.read(payload,512) > 0){
        int idx = 0;
        short count =
                ((unsigned char)payload[idx + 1] << 8) |
                ((unsigned char)payload[idx + 0]);
        if(count == 0)
            break;
        qDebug() << count;
        idx += 2;
        for(int i = 1; i <= count; ++i){
            auto time = (unsigned int)(((unsigned char)payload[idx + 3] << 24) |
                                ((unsigned char)payload[idx + 2] << 16) |
                                ((unsigned char)payload[idx + 1] << 8) |
                                ((unsigned char)payload[idx]));
            idx += 4;
            auto accx = short(((unsigned char)payload[idx + 3] << 8) |
                         ((unsigned char)payload[idx + 2]));
            idx += 2;
            auto accy = short(((unsigned char)payload[idx + 3] << 8) |
                               ((unsigned char)payload[idx + 2]));
            idx += 2;
            auto accz = short(((unsigned char)payload[idx + 3] << 8) |
                               ((unsigned char)payload[idx + 2]));
            idx += 2;
            auto gyrox = short(((unsigned char)payload[idx + 3] << 8) |
                               ((unsigned char)payload[idx + 2]));
            idx += 2;
            auto gyroy = short(((unsigned char)payload[idx + 3] << 8) |
                               ((unsigned char)payload[idx + 2]));
            idx += 2;
            auto gyroz = short(((unsigned char)payload[idx + 3] << 8) |
                               ((unsigned char)payload[idx + 2]));
            idx += 2;
            auto temp = short(((unsigned char)payload[idx + 3] << 8) |
                                ((unsigned char)payload[idx + 2]));
            idx += 2;
            if(idx >= 512) {
                QMessageBox::critical(0, "Error", "Count exceeds the number of entries in this block!");
                return;
            }
            out << time << ',' <<
            (((double)accx)/ACCEL_SENSIVITY) << ',' <<
            (((double)accy)/ACCEL_SENSIVITY) << ',' <<
            (((double)accz)/ACCEL_SENSIVITY) << ',' <<
            (((double)gyrox)/GYRO_SENSIVITY) << ',' <<
            (((double)gyroy)/GYRO_SENSIVITY) << ',' <<
            (((double)gyroz)/GYRO_SENSIVITY) << ',' <<
             temp << '\n';
            samples++;
        }
        emit totalSamples(samples);

    }

}