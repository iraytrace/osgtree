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
    void osgObjectClicked(osg::ref_ptr<osg::Object> object);



private:

    Ui::OsgTreeForm *ui;
};

#endif // OSGTREEFORM_H
