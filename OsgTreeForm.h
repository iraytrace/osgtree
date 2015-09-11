#ifndef OSGTREEFORM_H
#define OSGTREEFORM_H

#include <QWidget>


#include "OsgItemModel.h"

class QTableWidgetItem;
class QTableWidget;
#include <osg/Drawable>
#include <osg/Geometry>

namespace Ui {
class OsgTreeForm;
}

class OsgTreeForm : public QWidget
{
    Q_OBJECT

public:
    explicit OsgTreeForm(QWidget *parent = 0);
    ~OsgTreeForm();

    void setModel(OsgItemModel *model);
private slots:
    void osgObjectActivated(osg::ref_ptr<osg::Object> object);
    void itemClicked(QTableWidgetItem * item);

private:
    void setTableValuesObject(osg::ref_ptr<osg::Object> object);
    void setTableValuesNode(osg::Node * node);
    void setTableValuesGroup(osg::Group *group);
    void setTableValuesGeode(osg::Geode *geode);
    void setTableValuesDrawable(osg::Drawable *drawable);

    void setTableValuesGeometry(osg::Geometry *geometry);

    QTableWidgetItem *getOrCreateWidgetItem(QTableWidget *tw, int row, int col);
    QTableWidgetItem *itemForKey(const QString key);
    void setTextForKey(const QString key, const QString value = QString(""));

    Ui::OsgTreeForm *ui;
    QTableWidgetItem *setKeyChecked(const QString key, const bool value);
};

#endif // OSGTREEFORM_H
