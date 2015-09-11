#include "OsgItemModel.h"
#include <QBrush>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>

static bool debugModel = false;
#define modelDebug if (debugModel) qDebug

static QString stringFromRole(const int role);

OsgItemModel::OsgItemModel(QObject * parent)
    : QAbstractItemModel(parent)
    , m_root(new osg::Group)
    , m_loadedModel(new osg::MatrixTransform)
    , m_clipBoard(new osg::Group)
{
    m_root->setName("__root");
    m_loadedModel->setName("__loadedModel");

    m_root->addChild(m_loadedModel);

    m_clipBoard->setName("__clipBoard");
}

int OsgItemModel::columnCount(const QModelIndex & parent) const
{
    int numberOfColumns = 1;
    if (!parent.isValid()) {
        // looking at root node
        if (m_loadedModel->getNumChildren() > 0)
            numberOfColumns = 3;
    } else {
        numberOfColumns = 3; // XXX how to tell the real number of columns?
    }

    modelDebug("columnCount(%d,%d) = %d",
           parent.row(), parent.column(), numberOfColumns);

    return numberOfColumns;
}

static void setObjectNameFromParent(osg::ref_ptr<osg::Object> object)
{
    if (object->getName().size() > 0)
        return;

    QString newName;

    if (osg::Drawable *drawable = dynamic_cast<osg::Drawable *>(object.get())) {
        if (drawable->getNumParents() == 0) {
            newName = QString::asprintf("Lonely_%s", object->className());
        } else {
            setObjectNameFromParent(drawable->getParent(0)); // make sure parent is named
            newName = QString::asprintf("%s_%s",
                              drawable->getParent(0)->getName().c_str(),
                              object->className() );
        }
    } else if (osg::Node *node = dynamic_cast<osg::Node *>(object.get())) {
        if (node->getNumParents() == 0) {
            newName = QString::asprintf("Lonely_%s", object->className());
        } else {
            setObjectNameFromParent(node->getParent(0)); // make sure parent is named
            newName = QString::asprintf("%s_%s",
                              node->getParent(0)->getName().c_str(),
                              object->className() );
        }
    }
    object->setName(qPrintable(newName));
}

QVariant OsgItemModel::data(const QModelIndex &index, int role) const
{
    QVariant variant;
    if (!index.isValid()) {
        modelDebug("data() on invalid\n");
        return QVariant();
    }

    osg::ref_ptr<osg::Object> object = getObjectFromModelIndex(index);

#if 0
    modelDebug("data(%d %d %s) ", index.row(), index.column(),
           qPrintable(stringFromRole(role)));
#else
    if (role == Qt::DisplayRole)
        modelDebug("data(%d %d %s) ", index.row(), index.column(), object->getName().c_str());
#endif

    switch (role) {
    case Qt::DisplayRole: {

        switch (index.column()) {
        case 0:
            if (object->getName().size() == 0)
                setObjectNameFromParent(object);

            variant = QVariant(QString::fromStdString(object->getName()));
            break;
        case 1:
            variant = QVariant(QString(object->className())); break;

        case 2: {
            osg::Node *node = dynamic_cast<osg::Node *>(object.get());
            if (node) {
                variant = QVariant(maskToString(node->getNodeMask()));
            }
            break;
        }
        default:
            break;
        }
        break;
    }
     default:
        variant = QVariant();
        break;
    }
    return variant;
}

Qt::ItemFlags OsgItemModel::flags(const QModelIndex &index) const
{
    osg::ref_ptr<osg::Object> object = getObjectFromModelIndex(index);

    modelDebug("flags %d %d \"%s\"",
           index.row(), index.column(),
           object->getName().c_str());

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == 0) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

osg::ref_ptr<osg::Object>  OsgItemModel::getObjectFromModelIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        osg::ref_ptr<osg::Object> obj = static_cast<osg::Node *>(index.internalPointer());
        if (obj)
            return obj;
    }
    return m_loadedModel;
}


bool OsgItemModel::hasChildren ( const QModelIndex & parent ) const
{
    osg::ref_ptr<osg::Object> object = getObjectFromModelIndex(parent);

    modelDebug("hasChildren(%s %d %d) ... ",
           object->getName().c_str(),
           parent.row(), parent.column());

    unsigned numberOfChildren = 0;

    if (osg::Geode *geode = dynamic_cast<osg::Geode *>(object.get())) {
        numberOfChildren = geode->getNumDrawables();
    } else if (osg::Group *group = dynamic_cast<osg::Group *>(object.get())) {
        numberOfChildren = group->getNumChildren();
    } else if (dynamic_cast<osg::Drawable *>(object.get())) {
        numberOfChildren = 0;
    } else {
        abort();
    }

    modelDebug("hasChildren(%s %d %d) = %u",
           object->getName().c_str(),
           parent.row(),
           parent.column(),
           numberOfChildren);

    return (numberOfChildren > 0);
}


QVariant OsgItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    modelDebug("headerData(%d, , )\n", section);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:  return QVariant(QString("Name"));
        case 1:  return QVariant(QString("Type"));
        case 2:  return QVariant(QString("mask"));
        default: return QVariant(QString("col %1").arg(section));
        }
    }
    return QVariant();
}

QModelIndex OsgItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!(parent.isValid() && parent.column() != 0))
    {
        osg::ref_ptr<osg::Object> parentItem = getObjectFromModelIndex(parent);

        if (osg::Group *group = dynamic_cast<osg::Group *>(parentItem.get())) {
            if (group->getNumChildren() > (unsigned)row)
                return createIndex(row, column, group->getChild(row));
        } else if (osg::Geode *geode = dynamic_cast<osg::Geode *>(parentItem.get())) {
            if (geode->getNumDrawables() > (unsigned)row) {
                return createIndex(row, column, geode->getDrawable(row));
            }
        }
    }
    return QModelIndex();
}

QModelIndex OsgItemModel::modelIndexFromNode(osg::ref_ptr<osg::Node> ptr,
                  int column) const
{
    if (!ptr.valid() || ptr == m_loadedModel || ptr->getNumParents() <= 0)
        return QModelIndex();

    osg::ref_ptr<osg::Group> parent = ptr->getParent(0);
    int idx = parent->getChildIndex(ptr);
    return createIndex(idx, column, ptr);
}

void OsgItemModel::insertNode(osg::ref_ptr<osg::Group> parent,
                              osg::ref_ptr<osg::Node> newChild,
                              int childPositionInParent,
                              int row)
{
    if (!parent.valid()) abort();

    QModelIndex pIndex = this->modelIndexFromNode(parent, 0);
    beginInsertRows(pIndex, row, row);
    parent->insertChild(childPositionInParent, newChild);
    endInsertRows();
}

QModelIndex OsgItemModel::parentOfNode(osg::Node *childNode) const
{
    if (childNode == m_loadedModel)
        return QModelIndex();

    // get the osg parent
    // XXX this assumes one parent
    osg::ref_ptr<osg::Group> parent = childNode->getParent(0);

    return modelIndexFromNode(parent, 0);
}

QModelIndex OsgItemModel::parentOfDrawable(osg::Drawable *childDrawable) const
{
    if (childDrawable->getNumParents() == 0)
        return QModelIndex();

    osg::ref_ptr<osg::Node> parentNode = childDrawable->getParent(0);

    return modelIndexFromNode(parentNode, 0);
}


QModelIndex OsgItemModel::parent(const QModelIndex &index) const
{
    if (! index.isValid())
        return QModelIndex();

    osg::ref_ptr<osg::Object> childObject = getObjectFromModelIndex(index);

    if (osg::Node *node = dynamic_cast<osg::Node *>(childObject.get()))
        return parentOfNode(node);

    osg::Drawable *drawable = dynamic_cast<osg::Drawable *>(childObject.get());
    if (!drawable) {
        abort();
    }
    return parentOfDrawable(drawable);
}

void OsgItemModel::printNode(osg::ref_ptr<osg::Node> n, const int level) const
{
    modelDebug("%*s%s  %s\n", level, "", n->getName().c_str(), qPrintable(maskToString(n->getNodeMask())));

    osg::ref_ptr<osg::Group> g = n->asGroup();
    if (g) {
        int kids = g->getNumChildren();
        for (int kid=0 ; kid < kids ; kid++) {
            osg::ref_ptr<osg::Node> child = g->getChild(kid);
            printNode(child, level+2);
        }
        return;
    }
}

QString OsgItemModel::maskToString(const osg::Node::NodeMask mask) const
{
    QString maskString = QString::asprintf("%08x", (unsigned)mask);
    modelDebug("maskToString %s", qPrintable(maskString));
    return maskString;
}
void OsgItemModel::printTree() const
{
    printNode(m_loadedModel, 2);
}


int OsgItemModel::rowCount(const QModelIndex &parent) const
{
    osg::ref_ptr<osg::Object> object = getObjectFromModelIndex(parent);
    int kids = 0;

    if (osg::Group *group = dynamic_cast<osg::Group *>(object.get())) {
        kids = group->getNumChildren();
    } else if (osg::Geode *geode = dynamic_cast<osg::Geode *>(object.get())) {
        kids = geode->getNumDrawables();
    }

    modelDebug("rowCount %d %d %s == %d\n",
           parent.row(),
           parent.column(),
           object->getName().c_str(), kids);

    return kids;
}

bool OsgItemModel::setData(const QModelIndex & index,
                           const QVariant & value,
                           int role )
{
    modelDebug("setData(%d %d %s) = \"%s\"",
           index.row(), index.column(),
           qPrintable(stringFromRole(role)),
           qPrintable(value.toString()));

    osg::ref_ptr<osg::Object> object = getObjectFromModelIndex(index);

    if (role != Qt::EditRole)
        return false;

    if (index.column() == 0 && value.toString().size() > 0) {
        object->setName(qPrintable(value.toString()));
        return true;
    }
    return false;
}

void OsgItemModel::importFileByName(const QString fileName)
{
    osg::Node *loaded = osgDB::readNodeFile(fileName.toStdString());

    if (!loaded)
        return;

    if (loaded->getName().size() == 0)
        loaded->setName(basename(qPrintable(fileName)));

    if (m_loadedModel->getNumChildren() == 0) {
        beginInsertColumns(createIndex(-1, -1), 1, 2);
        insertNode(m_loadedModel, loaded, m_loadedModel->getNumChildren(), 0);
        endInsertColumns();
    } else {
        insertNode(m_loadedModel, loaded, m_loadedModel->getNumChildren(), 0);
    }
}
static QString stringFromRole(const int role)
{
    switch (role) {
    case Qt::DisplayRole: return QString("DisplayRole"); break;
    case Qt::DecorationRole: return QString("DecorationRole"); break;
    case Qt::EditRole: return QString("EditRole"); break;
    case Qt::ToolTipRole: return QString("ToolTipRole"); break;
    case Qt::StatusTipRole: return QString("StatusTipRole"); break;
    case Qt::WhatsThisRole: return QString("WhatsThisRole"); break;
    case Qt::SizeHintRole: return QString("SizeHintRole"); break;
    case Qt::FontRole: return QString("FontRole"); break;
    case Qt::TextAlignmentRole: return QString("TextAlignmentRole"); break;
    case Qt::BackgroundRole: return QString("BackgroundRole"); break;
    case Qt::ForegroundRole: return QString("ForegroundRole"); break;
    case Qt::CheckStateRole: return QString("CheckStateRole"); break;
    case Qt::InitialSortOrderRole: return QString("InitialSortOrderRole"); break;
    case Qt::AccessibleTextRole: return QString("AccessibleTextRole"); break;
    case Qt::AccessibleDescriptionRole: return QString("AccessibleDescriptionRole"); break;
    }

    if (role >= Qt::UserRole)
        return QString("UserRole");

    return QString("invalidRole");
}
