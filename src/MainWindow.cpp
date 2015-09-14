#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSettings>
#include <QFileDialog>

#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->osgTreeForm->setModel(&m_itemModel);
    ui->osg3dView->setScene(&m_itemModel);

    connect(ui->osg3dView, SIGNAL(updated()),
            ui->osgCameraView, SLOT(updateFromCamera()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFileOpen_triggered()
{
    QSettings settings;
    settings.value("currentDirectory");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Select File",
                                                    settings.value("currentDirectory").toString(),
                                                        "OpenSceneGraph (*.osg *.ive *.osgt *.osgb *.obj)");
    if (fileName.isEmpty())
        return;

    settings.setValue("currentDirectory", QVariant(QFileInfo(fileName).path()));

    m_itemModel.importFileByName(fileName);
    settings.setValue("recentFile", fileName);
}

void MainWindow::on_actionFileSave_triggered()
{
    QSettings settings;

    QString fileName = settings.value("recentFile").toString();

    if (m_itemModel.saveToFileByName(fileName))
        qDebug("yes");
    else
        qDebug("no");
}

void MainWindow::on_actionFileSaveAs_triggered()
{
    QSettings settings;
    settings.value("currentDirectory");
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Select File",
                                                    settings.value("currentDirectory").toString(),
                                                        "OpenSceneGraph (*.osgt *.osgb *.osgx)");
    qDebug("save file<%s>", qPrintable(fileName));
    if (m_itemModel.saveToFileByName(fileName))
        qDebug("yes");
    else
        qDebug("no");

}
