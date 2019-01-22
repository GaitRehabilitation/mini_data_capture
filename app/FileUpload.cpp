//
// Created by michaelpollind on 1/21/19.
//

#include "FileUpload.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QFileDialog>

FileUpload::FileUpload(QWidget *parent) : QToolButton(parent){
    setAcceptDrops(true);

    connect(this,&QToolButton::clicked,[=](bool checked){
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setNameFilter(trUtf8("data (*.BIN)"));
        if(dialog.exec()) {
            emit onFilesUploaded(dialog.selectedFiles());
        }
    });
}


void FileUpload::dragEnterEvent(QDragEnterEvent *event){
    QToolButton::dragEnterEvent(event);
    if(event->mimeData()->hasUrls()){
        event->acceptProposedAction();
    }
}

void FileUpload::dropEvent(QDropEvent *event) {
    QToolButton::dropEvent(event);
    QStringList result;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        result.append(fileName);
    }
    emit onFilesUploaded(result);
}

