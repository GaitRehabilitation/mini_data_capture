//
// Created by michaelpollind on 1/21/19.
//

#ifndef MINI_DATA_CAPTURE_APPLICATION_H
#define MINI_DATA_CAPTURE_APPLICATION_H

#include <QMainWindow>

namespace Ui{
    class  Application;
}

class Application : public QMainWindow {
    Q_OBJECT
private:
    Ui::Application *ui;
public:
    Application(QWidget *parent = nullptr);

};


#endif //MINI_DATA_CAPTURE_APP_H
