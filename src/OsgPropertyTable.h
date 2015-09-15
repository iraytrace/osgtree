#ifndef OSGPROPERTYTABLE_H
#define OSGPROPERTYTABLE_H

#include <QTableWidget>

#include <osg/ref_ptr>

namespace osg {
class Object;
class Node;
class Group;
class Geode;
class Drawable;
class Geometry;
}

class OsgPropertyTable : public QTableWidget
{
public:
    OsgPropertyTable(QWidget * parent = 0);
    OsgPropertyTable(int rows, int columns, QWidget * parent = 0);

signals:

public slots:
    void displayObject(osg::ref_ptr<osg::Object> object);
    void hideAllRows();
private slots:
    void itemWasClicked(QTableWidgetItem *item);

private:
    void setTableValuesForObject(osg::ref_ptr<osg::Object> object);
    void setTableValuesForNode(osg::Node * node);
    void setTableValuesForGroup(osg::Group *group);
    void setTableValuesForGeode(osg::Geode *geode);
    void setTableValuesForDrawable(osg::Drawable *drawable);
    void setTableValuesForGeometry(osg::Geometry *geometry);

    QTableWidgetItem *getOrCreateWidgetItem(QTableWidget *tw, int row, int col);
    QTableWidgetItem *itemForKey(const QString key);
    QTableWidgetItem *setKeyChecked(const QString key, const bool value);
    void setTextForKey(const QString key, const QString value = QString(""));

};

#endif // OSGPROPERTYTABLE_H
