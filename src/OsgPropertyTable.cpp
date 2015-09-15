#include "OsgPropertyTable.h"

#include <QHeaderView>

#include <osg/Geometry>
#include <osg/Drawable>
#include <osg/Group>
#include <osg/Geode>

#include "VariantPtr.h"

OsgPropertyTable::OsgPropertyTable(QWidget *parent) :
    QTableWidget(parent)
{
    resizeColumnsToContents();
    horizontalHeader()->setStretchLastSection(true);

    connect(this, SIGNAL(itemClicked(QTableWidgetItem*)),
            this, SLOT(itemWasClicked(QTableWidgetItem*)));

    hideAllRows();
}

OsgPropertyTable::OsgPropertyTable(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent)
{

}

void OsgPropertyTable::displayObject(osg::ref_ptr<osg::Object> object)
{
    qDebug("displayObject(%s)", object->getName().c_str());

    hideAllRows();

    if (object.get()) {
        setTableValuesForObject(object);
        setTableValuesForNode(dynamic_cast<osg::Node *>(object.get()));
        setTableValuesForDrawable(dynamic_cast<osg::Drawable *>(object.get()));
    }

    resizeColumnsToContents();
    horizontalHeader()->setStretchLastSection(true);
}

void OsgPropertyTable::hideAllRows()
{
    for (int i=0 ; i < rowCount() ; i++)
        hideRow(i);
}

void OsgPropertyTable::itemWasClicked(QTableWidgetItem *item)
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

QTableWidgetItem * OsgPropertyTable::getOrCreateWidgetItem(QTableWidget *tw, int row, int col)
{
    QTableWidgetItem *twi = tw->item(row, col);
    if (!twi) {
        twi = new QTableWidgetItem;
        tw->setItem(row, col, twi);
    }
    return twi;
}
void OsgPropertyTable::setTextForKey(const QString key, const QString value)
{
    QTableWidgetItem *twi = itemForKey(key);

    twi->setText(value);

    twi->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

    showRow(twi->row());
}
QTableWidgetItem * OsgPropertyTable::setKeyChecked(const QString key, const bool value)
{
    QTableWidgetItem *twi = itemForKey(key);

    twi->setText("");

    if (value)
        twi->setCheckState(Qt::Checked);
    else
        twi->setCheckState(Qt::Unchecked);

    twi->setFlags(Qt::ItemIsEnabled);
    showRow(twi->row());

    return twi;
}
QTableWidgetItem * OsgPropertyTable::itemForKey(const QString key)
{
    QList<QTableWidgetItem *> twilist = findItems(key, Qt::MatchFixedString);
    QTableWidgetItem *twi;

    if (twilist.size() == 0) {
        int currentRowCount = rowCount();
        setRowCount(currentRowCount+1);
        twi = getOrCreateWidgetItem(this, currentRowCount, 0);
        twi->setText(key);
    } else
        twi = twilist.at(0);

     return getOrCreateWidgetItem(this, twi->row(), twi->column()+1);
}

void OsgPropertyTable::setTableValuesForObject(osg::ref_ptr<osg::Object> object)
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
void OsgPropertyTable::setTableValuesForNode(osg::Node *node)
{
    if (!node)
        return;

    setTextForKey("NodeMask", QString::asprintf("%08x", (unsigned)node->getNodeMask()));

    setKeyChecked("CullingActive", node->isCullingActive());

    setTextForKey("Descriptions", QString::asprintf("%d", node->getNumDescriptions()));

    setTableValuesForGroup(node->asGroup());
    setTableValuesForGeode(node->asGeode());
}
void OsgPropertyTable::setTableValuesForGroup(osg::Group *group)
{
    if(!group) return;
    setTextForKey("NumChildren", QString::asprintf("%d", group->getNumChildren()));
}
void OsgPropertyTable::setTableValuesForGeode(osg::Geode *geode)
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
void OsgPropertyTable::setTableValuesForDrawable(osg::Drawable *drawable)
{
    if (!drawable) return;

    setKeyChecked("UseVertexBuffer", drawable->getUseVertexBufferObjects());
    setKeyChecked("UseDisplayList", drawable->getUseDisplayList());

    setTableValuesForGeometry(drawable->asGeometry());
}
void OsgPropertyTable::setTableValuesForGeometry(osg::Geometry *geometry)
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
