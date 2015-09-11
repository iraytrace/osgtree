#include "OsgCameraForm.h"
#include "ui_OsgCameraForm.h"

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
