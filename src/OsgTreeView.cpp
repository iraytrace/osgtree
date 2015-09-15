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

//    connect(this, SIGNAL(activated(QModelIndex)),
//            this, SLOT(announceObject(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)),
            this, SLOT(announceObject(QModelIndex)));

    popupMenu.addAction("Cut", this, SLOT(cutObject()) );
    popupMenu.addAction("Copy", this, SLOT(copyObject()), QKeySequence::Copy );
    popupMenu.addAction("Paste", this, SLOT(pasteObject()), QKeySequence::Paste );

    this->setSelectionBehavior( QAbstractItemView::SelectRows );
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
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

void OsgTreeView::cutObject()
{
    QModelIndexList idxList = this->selectionModel()->selectedRows();

    this->selectionModel()->clearSelection();
    this->selectionModel()->clearCurrentIndex();
    if (OsgItemModel *oim = dynamic_cast<OsgItemModel *>(this->model())) {
        oim->cutItem(idxList);
    }
}

void OsgTreeView::pasteObject()
{
    qDebug("pasteObject()");

}

void OsgTreeView::copyObject()
{
    qDebug("copyObject()");

}

void OsgTreeView::announceObject(const QModelIndex &index)
{
    OsgItemModel *model = dynamic_cast<OsgItemModel *>(this->model());
    qDebug("announceObject");
    if (!model)
        return;

    emit osgObjectClicked(model->getObjectFromModelIndex(index));
}

void OsgTreeView::selectionWasChanged(QItemSelection selected,
                                      QItemSelection deselected)
{
    qDebug("selectionWasChanged");
}

void OsgTreeView::currentWasChanged(QModelIndex current, QModelIndex previous)
{
    qDebug("currentWasChanged (%d,%d) -> (%d,%d)",
           previous.row(), previous.column(),
           current.row(), current.column()
           );
}
