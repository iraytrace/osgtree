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
}


void OsgTreeView::resizeColumnsToFit()
{
    this->resizeColumnToContents(0);
    this->resizeColumnToContents(1);
}

void OsgTreeView::customMenuRequested(QPoint pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Action 1", this));
    menu->addAction(new QAction("Action 2", this));
    menu->addAction(new QAction("Action 3", this));
    menu->popup(this->viewport()->mapToGlobal(pos));
}

void OsgTreeView::announceObject(const QModelIndex &index)
{
    OsgItemModel *model = dynamic_cast<OsgItemModel *>(this->model());

    if (!model)
        return;

    emit osgObjectActivated(model->getObjectFromModelIndex(index));
}
