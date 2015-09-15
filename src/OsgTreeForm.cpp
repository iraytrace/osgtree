#include "OsgTreeForm.h"
#include "ui_OsgTreeForm.h"
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>

#include "VariantPtr.h"

OsgTreeForm::OsgTreeForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OsgTreeForm)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 1);

    connect(ui->osgTreeView, SIGNAL(osgObjectClicked(osg::ref_ptr<osg::Object>)),
            this, SLOT(osgObjectClicked(osg::ref_ptr<osg::Object>)));

//    connect(ui->osgTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
//            this, SLOT(itemClicked(QTableWidgetItem *)));
}
OsgTreeForm::~OsgTreeForm()
{
    delete ui;
}
void OsgTreeForm::setModel(OsgItemModel *model)
{
    ui->osgTreeView->setModel(model);

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->osgTreeView, SLOT(resizeColumnsToFit()));

    QItemSelectionModel *ism =ui->osgTreeView->selectionModel();

    if (ism) {
        qDebug("connecting...");
        connect(ism, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                ui->osgTreeView, SLOT(selectionWasChanged(QItemSelection,QItemSelection)));

        connect(ism, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                ui->osgTreeView, SLOT(currentWasChanged(QModelIndex,QModelIndex)));
    }
}


void OsgTreeForm::osgObjectClicked(osg::ref_ptr<osg::Object> object)
{
    qDebug("osgObjectClicked(%s)", object->getName().c_str());

    ui->osgPropertyTable->displayObject(object);
}


