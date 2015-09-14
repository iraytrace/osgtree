#ifndef OSGITEMMODEL_H
#define OSGITEMMODEL_H

#include <QAbstractItemModel>
#include <osg/Node>
#include <osg/MatrixTransform>

class OsgItemModel : public QAbstractItemModel
{
public:
    OsgItemModel(QObject * parent = 0);

    //////////////////// Start QAbstractItemModel methods //////////////////////
    int             columnCount(const QModelIndex &parent) const;
    QVariant        data(const QModelIndex &index, int role) const;
    Qt::ItemFlags   flags(const QModelIndex &index) const;
    bool            hasChildren(const QModelIndex &parent=QModelIndex()) const;
    QVariant        headerData(int section,
                               Qt::Orientation orientation,
                               int role) const;
    QModelIndex     index(int row,
                          int column,
                          const QModelIndex & parent = QModelIndex()) const;

    QModelIndex     parent(const QModelIndex &index) const;
    int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool            setData(const QModelIndex &index,
                            const QVariant &value,
                            int role = Qt::EditRole);
    //////////////////// End QAbstractItemModel methods ////////////////////////

    void importFileByName(const QString fileName); ///< load a file into the "root"
    bool saveToFileByName(const QString fileName);

    /// print the entire heirarchy on stderr.  Mostly for debugging.
    void            printTree() const;

    /// print a node of the tree and all its children
    void printNode(osg::ref_ptr<osg::Node> n,
                   const int level) const;

    void insertNode(osg::ref_ptr<osg::Group> parent,
                    osg::ref_ptr<osg::Node> newChild,
                    int childPositionInParent,
                    int row);

    osg::ref_ptr<osg::Object> getObjectFromModelIndex(const QModelIndex &index) const;

    // The only thing that should call this is OsgView::setScene()
    osg::ref_ptr<osg::Group> getRoot() const { return m_root; }

private:

    QString maskToString(const osg::Node::NodeMask mask) const;
    QModelIndex modelIndexFromNode(osg::ref_ptr<osg::Node> ptr,
                                   int column) const;
    QModelIndex parentOfNode(osg::Node *childNode) const;
    QModelIndex parentOfDrawable(osg::Drawable *childDrawable) const;
    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<osg::MatrixTransform> m_loadedModel;
    osg::ref_ptr<osg::Group> m_clipBoard;
    bool setObjectMask(const QModelIndex &index, const QVariant &value);
    bool setObjectName(const QModelIndex &index, const QVariant &value);
};

#endif // OSGITEMMODEL_H
