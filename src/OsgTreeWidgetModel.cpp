#include "OsgTreeWidgetModel.h"

OsgTreeWidgetModel::OsgTreeWidgetModel(QObject *parent)
    : QObject(parent)
    , m_root(new osg::Group)
{

}

void OsgTreeWidgetModel::importFileByName(const QString fileName)
{
    osg::Node *loaded = osgDB::readNodeFile(fileName.toStdString());

    if (!loaded)
        return;

}

bool OsgTreeWidgetModel::saveToFileByName(const QString fileName)
{

}

