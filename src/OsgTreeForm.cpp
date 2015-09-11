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
    ui->osgTableWidget->resizeColumnsToContents();
    ui->osgTableWidget->horizontalHeader()->setStretchLastSection(true);


    connect(ui->osgTreeView, SIGNAL(osgObjectActivated(osg::ref_ptr<osg::Object>)),
            this, SLOT(osgObjectActivated(osg::ref_ptr<osg::Object>)));

    connect(ui->osgTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
            this, SLOT(itemClicked(QTableWidgetItem *)));

    // hide all rows
    for (int i=0 ; i < ui->osgTableWidget->rowCount() ; i++) {
        ui->osgTableWidget->hideRow(i);
    }


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
}

QTableWidgetItem * OsgTreeForm::getOrCreateWidgetItem(QTableWidget *tw, int row, int col)
{
    QTableWidgetItem *twi = tw->item(row, col);
    if (!twi) {
        twi = new QTableWidgetItem;
        tw->setItem(row, col, twi);
    }
    return twi;
}
void OsgTreeForm::setTextForKey(const QString key, const QString value)
{
    QTableWidgetItem *twi = itemForKey(key);

    twi->setText(value);

    twi->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    ui->osgTableWidget->showRow(twi->row());
}

QTableWidgetItem * OsgTreeForm::setKeyChecked(const QString key, const bool value)
{
    QTableWidgetItem *twi = itemForKey(key);

    twi->setText("");

    if (value)
        twi->setCheckState(Qt::Checked);
    else
        twi->setCheckState(Qt::Unchecked);

    twi->setFlags(Qt::ItemIsEnabled);
    ui->osgTableWidget->showRow(twi->row());

    return twi;
}

QTableWidgetItem * OsgTreeForm::itemForKey(const QString key)
{
    QList<QTableWidgetItem *> twilist = ui->osgTableWidget->findItems(key, Qt::MatchFixedString);
    QTableWidgetItem *twi;

    if (twilist.size() == 0) {
        int currentRowCount = ui->osgTableWidget->rowCount();
        ui->osgTableWidget->setRowCount(currentRowCount+1);
        twi = getOrCreateWidgetItem(ui->osgTableWidget, currentRowCount, 0);
        twi->setText(key);
    } else
        twi = twilist.at(0);

     return getOrCreateWidgetItem(ui->osgTableWidget, twi->row(), twi->column()+1);
}

void OsgTreeForm::osgObjectActivated(osg::ref_ptr<osg::Object> object)
{
    qDebug("activated(%s)", object->getName().c_str());


    for (int i=0 ; i < ui->osgTableWidget->rowCount() ; i++) {
        ui->osgTableWidget->hideRow(i);
    }

    setTableValuesObject(object);
    setTableValuesNode(dynamic_cast<osg::Node *>(object.get()));
    setTableValuesDrawable(dynamic_cast<osg::Drawable *>(object.get()));

    ui->osgTableWidget->resizeColumnsToContents();
    ui->osgTableWidget->horizontalHeader()->setStretchLastSection(true);
}

void OsgTreeForm::itemClicked(QTableWidgetItem *item)
{
    if ( ! (item->flags() & Qt::ItemIsUserCheckable) )
        return;

    QVariant v = item->data(Qt::UserRole);

    qDebug("clicked %d %d %s", item->row(), item->column(), qPrintable(item->text()));
    if (v.isValid() && !v.isNull()) {
        osg::Object *obj = VariantPtr<osg::Object>::asPtr(v);

        if (osg::Node *n = dynamic_cast<osg::Node *>(obj)) {
            qDebug("Node");
        }
        if (osg::Drawable *d = dynamic_cast<osg::Drawable *>(obj)) {
            qDebug("Drawable");
        }

    }
}

void OsgTreeForm::setTableValuesObject(osg::ref_ptr<osg::Object> object)
{
    setTextForKey("Name", QString::fromStdString( object->getName() ));

    osg::Object::DataVariance dataVariance = object->getDataVariance();
    QString key = "DataVariance";
    switch (dataVariance) {
    case osg::Object::DYNAMIC:
        setTextForKey(key, "DYNAMIC");
        break;
    case osg::Object::STATIC:
        setTextForKey(key, "STATIC");
        break;
    case osg::Object::UNSPECIFIED:
        setTextForKey(key, "UNSPECIFIED");
        break;
    }
}

void OsgTreeForm::setTableValuesNode(osg::Node *node)
{
    if (!node)
        return;

    setTextForKey("NodeMask", QString::asprintf("%08x", (unsigned)node->getNodeMask()));

    setKeyChecked("CullingActive", node->isCullingActive());

    setTextForKey("Descriptions", QString::asprintf("%d", node->getNumDescriptions()));

    setTableValuesGroup(node->asGroup());
    setTableValuesGeode(node->asGeode());
}
void OsgTreeForm::setTableValuesGroup(osg::Group *group)
{
    if(!group) return;
    setTextForKey("NumChildren", QString::asprintf("%d", group->getNumChildren()));
}

void OsgTreeForm::setTableValuesGeode(osg::Geode *geode)
{
    if(!geode) return;

    setTextForKey("NumDrawables", QString::asprintf("%d", geode->getNumDrawables()));

    osg::BoundingSpheref bs = geode->computeBound();
    setTextForKey("GeodeBoundingSphere", QString::asprintf("radius:%g @(%g %g %g)",
                                                           bs.radius(),
                                                           bs.center().x(),
                                                           bs.center().y(),
                                                           bs.center().z()
                                                           ));
}

void OsgTreeForm::setTableValuesDrawable(osg::Drawable *drawable)
{
    if (!drawable) return;

    setKeyChecked("UseVertexBuffer", drawable->getUseVertexBufferObjects());
    setKeyChecked("UseDisplayList", drawable->getUseDisplayList());

    setTableValuesGeometry(drawable->asGeometry());
}

void OsgTreeForm::setTableValuesGeometry(osg::Geometry *geometry)
{
    if (!geometry) return;

    osg::Array *array = geometry->getVertexArray();
    setTextForKey("PrimitiveSets",
                  QString::asprintf("%d", geometry->getNumPrimitiveSets() ));

    if (array)
        setTextForKey("VertexCount", QString::asprintf("%d", array->getNumElements()));


    array = geometry->getNormalArray();
    if (array)
        setTextForKey("NormalCount", QString::asprintf("%d", array->getNumElements()));


    array = geometry->getColorArray();
    if (array)
        setTextForKey("ColorCount", QString::asprintf("%d", array->getNumElements()));

    setTextForKey("TextCoordArrayCount",
                  QString::asprintf("%d", geometry->getNumTexCoordArrays()));

}
