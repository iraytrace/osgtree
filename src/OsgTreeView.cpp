#include "OsgTreeView.h"
#include <QMenu>
#include "OsgItemModel.h"

OsgTreeView::OsgTreeView(QWidget *parent) : QTreeView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customMenuRequested(QPoint)));

    connect(this, SIGNAL(expanded(QModelIndex)),
            this, SLOT(resizeColumnsToFit()));
    connect(this, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(resizeColumnsToFit()));

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(announceObject(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(announceObject(QModelIndex)));

    popupMenu.addAction(new QAction("Copy", this));
    popupMenu.addAction(new QAction("Cut", this));
    popupMenu.addAction(new QAction("Paste", this));

}


void OsgTreeView::resizeColumnsToFit()
{
    this->resizeColumnToContents(0);
    this->resizeColumnToContents(1);
}

void OsgTreeView::customMenuRequested(QPoint pos)
{
    popupMenu.popup(this->viewport()->mapToGlobal(pos));
}

void OsgTreeView::announceObject(const QModelIndex &index)
{
    OsgItemModel *model = dynamic_cast<OsgItemModel *>(this->model());

    if (!model)
        return;

    emit osgObjectActivated(model->getObjectFromModelIndex(index));
}
