#include "OsgCameraForm.h"
#include "ui_OsgCameraForm.h"
#include "Osg3dView.h"
#include "ViewingCore.h"

OsgCameraForm::OsgCameraForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OsgCameraForm)
{
    ui->setupUi(this);
}

OsgCameraForm::~OsgCameraForm()
{
    delete ui;
}

void OsgCameraForm::updateFromCamera()
{
    Osg3dView *osgWin = dynamic_cast<Osg3dView *>(sender());
    if (!osgWin)
        return;

    osg::ref_ptr<ViewingCore> viewingCore = osgWin->getViewingCore();

    osg::Vec3d eyePos = viewingCore->getEyePosition();
    ui->eyePositionEdit->setText(QString::asprintf("%g %g %g", eyePos.x(), eyePos.y(), eyePos.z()));

    osg::Vec3d viewCenter = viewingCore->getViewCenter();
    ui->viewCenterEdit->setText(QString::asprintf("%g %g %g", viewCenter.x(), viewCenter.y(), viewCenter.z()));

    ui->fovyEdit->setText(QString::asprintf("%g", viewingCore->getFovy()));
}
