#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "OsgItemModel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_actionFileOpen_triggered();

private:
    Ui::MainWindow *ui;

    OsgItemModel m_itemModel;
};

#endif // MAINWINDOW_H
