#ifndef OSGTREEWIDGETMODEL_H
#define OSGTREEWIDGETMODEL_H

#include <QObject>
#include <osg/Group>
class OsgTreeWidgetModel : public QObject
{
    Q_OBJECT
public:
    explicit OsgTreeWidgetModel(QObject *parent = 0);

signals:

public slots:
    void importFileByName(const QString fileName); ///< load a file into the "root"
    bool saveToFileByName(const QString fileName);
private:
    osg::ref_ptr<osg::Group> m_root;
};

#endif // OSGTREEWIDGETMODEL_H
