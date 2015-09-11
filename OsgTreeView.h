#ifndef OSGTREEVIEW_H
#define OSGTREEVIEW_H

#include <QTreeView>
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
    void columnsWereInserted(const QModelIndex & parent, int first, int last);
    void resizeColumnsToFit(const QModelIndex & index);
    void customMenuRequested(QPoint pos);
private slots:
    void announceObject(const QModelIndex & index);
};

#endif // OSGTREEVIEW_H
