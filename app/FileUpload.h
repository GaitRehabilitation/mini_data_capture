//
// Created by michaelpollind on 1/21/19.
//

#ifndef MINI_DATA_CAPTURE_FILEUPLOAD_H
#define MINI_DATA_CAPTURE_FILEUPLOAD_H

#include <QToolButton>
#include <QStringList>
class FileUpload : public QToolButton{
    Q_OBJECT
public:
    explicit FileUpload(QWidget *parent = nullptr);

     void dragEnterEvent(QDragEnterEvent *event) override;
     void dropEvent(QDropEvent *event) override;
 signals:
    void onFilesUploaded(const QStringList& files);
};


#endif //MINI_DATA_CAPTURE_QFILEUPLOAD_H
