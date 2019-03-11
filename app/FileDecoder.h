//
// Created by michaelpollind on 1/21/19.
//

#ifndef MINI_DATA_CAPTURE_FILEDECODER_H
#define MINI_DATA_CAPTURE_FILEDECODER_H

#include <QThread>
#include <QObject>

class FileDecoder : public QThread {
    Q_OBJECT
private:
    QString _path;
public:
    explicit FileDecoder(const QString& path,QObject* object = nullptr);
    virtual void run();
signals:
    void complete();
};


#endif //MINI_DATA_CAPTURE_FILEDECODER_H
