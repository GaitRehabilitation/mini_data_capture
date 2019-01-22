//
// Created by michaelpollind on 1/21/19.
//

#include <QtCore/QThreadPool>
#include "Application.h"

#include "ui_application.h"
#include "FileUpload.h"
#include "FileDecoder.h"
#include <QLabel>

Application::Application(QWidget *parent) : QMainWindow(parent),ui(new Ui::Application){
    ui->setupUi(this);

    connect(ui->upload_button,&FileUpload::onFilesUploaded,[=](const QStringList& files){
        foreach(QString file,files){
            QLabel* label = new QLabel(this);
            FileDecoder* decoder = new FileDecoder(file,label);
            ui->uploadEntries->addWidget(label);
            connect(decoder,&FileDecoder::totalSamples,[=](qint64 total){
                label->setText(QString("%1 Samples").arg(total));
            });
            QThreadPool::globalInstance()->start(decoder);
        }
    });

}