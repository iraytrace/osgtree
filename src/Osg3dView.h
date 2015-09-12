#ifndef OSGVIEW_H
#define OSGVIEW_H

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QMenu>

#include <osgViewer/Viewer>

#include "ViewingCore.h"

class OsgItemModel;

class Osg3dView : public QOpenGLWidget, public osgViewer::Viewer
{
    Q_OBJECT

public:
    Osg3dView(QWidget * parent = 0);

    enum MouseMode {
        MM_ORBIT = (1<<1),
        MM_PAN = (1<<2),
        MM_ZOOM = (1<<3),
        MM_ROTATE = (1<<4),
        MM_PICK_CENTER = (1<<5)
    };
    enum StandardView {
        V_TOP = (1<<0),
        V_BOTTOM = (1<<1),
        V_FRONT = (1<<2),
        V_BACK = (1<<3),
        V_RIGHT = (1<<4),
        V_LEFT = (1<<5)
    };
    enum Projection {
        P_ORTHO = (1<<0),
        P_PERSP = (1<<1)
    };
    enum DrawMode {
        D_FACET = (1<<1),
        D_WIRE = (1<<2),
        D_POINT = (1<<3)
    };

    /// Let others tell what scene graph we should be drawing
    void setScene(OsgItemModel *model);

    osg::ref_ptr<ViewingCore> getViewingCore() const { return m_viewingCore; }

public slots:
    void paintGL();
    void resizeGL(int w, int h);
    void hello();
    /// Set the m_mouseMode variable
    void setMouseMode(MouseMode mode);
    void setMouseMode();
    void setStandardView();
    void setDrawMode();
    void setProjection();

    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *event);
    void customMenuRequested(const QPoint &pos);

    void fitScreenTopView(const QModelIndex & parent, int first, int last);

    void dataChanged(const QModelIndex & topLeft,
                     const QModelIndex & bottomRight,
                     const QVector<int> & roles = QVector<int> ());

signals:
    /// Let the rest of the world (OsgView) know the current MouseMode
    void mouseModeChanged(Osg3dView::MouseMode);
    void updated();

private:
    osg::Vec2d getNormalized(const int ix, const int iy);

    /// OSG uses singleSided drawing/display by default.
    /// This is annoying when you are "inside" something and the back wall of
    /// it simply disappears. This gets called to set up to draw both front and
    /// back facing polygons so that the world behaves in a more normal fashion.
    /// Yes it's a performance hit.
    void setLightingTwoSided();

    void buildPopupMenu();

    QMenu m_popupMenu;

    /// OSG graphics window
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_osgGraphicsWindow;

    /// Camera manager
    osg::ref_ptr<ViewingCore> m_viewingCore;

    /// Current mouse mode
    MouseMode m_mouseMode;

    osg::Vec2d m_savedEventNDCoords;
};

#endif // OSGVIEW_H
