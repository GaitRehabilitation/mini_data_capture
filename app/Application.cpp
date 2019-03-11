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

    connect(ui->uploadButton,&FileUpload::onFilesUploaded,[=](const QStringList& files){
        this->numberOfEntries = files.length();
        this->entriesCompleted = 0;
        ui->uploadProgress->setValue(0);

        foreach(QString file,files){
            FileDecoder* decoder = new FileDecoder(file,this);
            connect(decoder,&FileDecoder::complete,this,[=](){
                    this->entriesCompleted += 1;
                    ui->processEntries->addItem(new QListWidgetItem(file + " ... done"));
                    ui->uploadProgress->setValue((int)(((float)this->entriesCompleted/(float)this->numberOfEntries)* 100));
            },Qt::QueuedConnection);
			connect(decoder, &FileDecoder::finished, this, [=]() {
				decoder->deleteLater();
			});
			decoder->start();
           //QThreadPool::globalInstance()->start(decoder);
        }
    });



}