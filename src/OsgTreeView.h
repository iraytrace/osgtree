#ifndef OSGTREEVIEW_H
#define OSGTREEVIEW_H

#include <QTreeView>
#include <QMenu>

#include <osg/ref_ptr>
#include <osg/Object>

class OsgTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit OsgTreeView(QWidget *parent = 0);

signals:
    void osgObjectActivated(osg::ref_ptr<osg::Object> object);

public slots:
    void resizeColumnsToFit();
    void customMenuRequested(QPoint pos);
private slots:
    void announceObject(const QModelIndex & index);

private:
    QMenu popupMenu;
};

#endif // OSGTREEVIEW_H
