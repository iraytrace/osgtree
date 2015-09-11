#ifndef OSGVIEW_H
#define OSGVIEW_H

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <osgViewer/Viewer>

#include "ViewingCore.h"
class OsgItemModel;

class OsgView : public QOpenGLWidget, public osgViewer::Viewer
{
    Q_OBJECT

public:
    OsgView(QWidget * parent = 0);

    /// Let others tell what scene graph we should be drawing
    void setScene(OsgItemModel *model);

public slots:
    void paintGL();
    void resizeGL(int w, int h);

    void hello();
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *event);
    void customMenuRequested(const QPoint &pos);
    void columnsInserted(const QModelIndex & parent, int first, int last);
    void rowsInserted(const QModelIndex & parent, int first, int last);

    void dataChanged(const QModelIndex & topLeft,
                     const QModelIndex & bottomRight,
                     const QVector<int> & roles = QVector<int> ());
private:
    /// OSG uses singleSided drawing/display by default.
    /// This is annoying when you are "inside" something and the back wall of
    /// it simply disappears. This gets called to set up to draw both front and
    /// back facing polygons so that the world behaves in a more normal fashion.
    /// Yes it's a performance hit.
    void setLightingTwoSided();

    /// OSG graphics window
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_osgGraphicsWindow;

    /// Camera manager
    osg::ref_ptr<ViewingCore> m_viewingCore;
};

#endif // OSGVIEW_H
