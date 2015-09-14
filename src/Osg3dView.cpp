#include "Osg3dView.h"

#include <QMenu>
#include "OsgItemModel.h"

#include <osg/LightModel>
#include <osgViewer/Renderer>
#include <osg/ValueObject>

static bool debugView = false;
#define vDebug if (debugView) qDebug

Osg3dView::Osg3dView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_viewingCore(new ViewingCore)
    , m_mouseMode(MM_ORBIT)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customMenuRequested(QPoint)));
    buildPopupMenu();

    // Construct the embedded graphics window
    m_osgGraphicsWindow = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
    getCamera()->setGraphicsContext(m_osgGraphicsWindow);

    // Set up the camera
    getCamera()->setViewport(new osg::Viewport(0,0,width(),height()));
    getCamera()->setProjectionMatrixAsPerspective(30.0f,
            static_cast<double>(width())/static_cast<double>(height()),
            1.0f,
            10000.0f);
    // By default draw everthing that has even 1 bit set in the nodemask
    getCamera()->setCullMask( (unsigned)~0 );
    getCamera()->setDataVariance(osg::Object::DYNAMIC);

    // As of July 2010 there wasn't really a good way to multi-thread OSG
    // under Qt so just set the threading model to be SingleThreaded
    setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // draw both sides of polygons
    setLightingTwoSided();

    update();
}

void Osg3dView::setScene(OsgItemModel *model)
{
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(fitScreenTopView(QModelIndex,int,int)));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(update()));

    osg::ref_ptr<osg::Group> root = model->getRoot();
    this->setSceneData(root);
    m_viewingCore->setSceneData(root);
}

void Osg3dView::paintGL()
{
    vDebug("paintGL");

    // Update the camera
    osg::Camera *cam = this->getCamera();
    const osg::Viewport* vp = cam->getViewport();

    m_viewingCore->setAspect(vp->width() / vp->height());
    osg::Node *n = this->getSceneData();

    int i;
    if (n->getUserValue("fred", i)) {
        qDebug("fred is %d", i);
    } else {
        qDebug("fred not set");
    }
    cam->setViewMatrix(m_viewingCore->getInverseMatrix());
    cam->setProjectionMatrix(m_viewingCore->computeProjection());

    // Invoke the OSG traversal pipeline
    frame();

    emit updated();
}

void Osg3dView::resizeGL(int w, int h)
{
    vDebug("resizeGL");

    m_osgGraphicsWindow->getEventQueue()->windowResize(9, 0, w, h);
    m_osgGraphicsWindow->resized(0,0,w,h);
}

void Osg3dView::hello()
{
    qDebug("hello");
}

void Osg3dView::setMouseMode(Osg3dView::MouseMode mode)
{
    m_mouseMode = mode;

    if(mode == MM_ROTATE)
        m_viewingCore->setViewingCoreMode( ViewingCore::FIRST_PERSON );
    else {
        m_viewingCore->setViewingCoreMode( ViewingCore::THIRD_PERSON );
    }

    emit mouseModeChanged(mode);
}

osg::Vec2d Osg3dView::getNormalized(const int ix, const int iy)
{
    int x, y, width, height;
    osg::Vec2d ndc;

    // we don't really need the x, y values but the width/height are important
    m_osgGraphicsWindow->getWindowRectangle(x, y, width, height);
    int center = width/2;
    ndc[0] = ((double)ix - (double)center) / (double)center;
    if (ndc[0] > 1.0) ndc[0] = 1.0;

    center = height/2;
    int invertedY = height - iy;
    ndc[1] = ((double)invertedY - (double)center) / (double)center;
    if (ndc[1] > 1.0) ndc[1] = 1.0;

    return ndc;
}

void Osg3dView::mousePressEvent(QMouseEvent *event)
{
    vDebug("mousePressEvent");

    if (event->button() == Qt::LeftButton) {
        m_savedEventNDCoords = getNormalized(event->x(), event->y());

        // Do the job asked
        if (m_mouseMode & (MM_PAN|MM_ROTATE|MM_ORBIT|MM_ZOOM) )
            m_viewingCore->setPanStart( m_savedEventNDCoords.x(),
                                        m_savedEventNDCoords.y());
        else if (m_mouseMode & MM_PICK_CENTER) {
            m_viewingCore->pickCenter(m_savedEventNDCoords.x(),
                                      m_savedEventNDCoords.y() );
        }
    }
}

void Osg3dView::mouseMoveEvent(QMouseEvent *event)
{
    vDebug("mouseMoveEvent");
    osg::Vec2d currentNDC = getNormalized(event->x(), event->y());
    osg::Vec2d delta = currentNDC - m_savedEventNDCoords;

    switch (m_mouseMode) {
    case MM_ORBIT:
        m_viewingCore->rotate( m_savedEventNDCoords, delta);
        break;
    case MM_PAN:
        m_viewingCore->pan(delta.x(), delta.y());
        break;
    case MM_ZOOM: {
        double tempScale = m_viewingCore->getFovyScale();
        m_viewingCore->setFovyScale(1.03);

        if(delta.y() > 0)
            m_viewingCore->fovyScaleDown();

        if(delta.y() < 0)
            m_viewingCore->fovyScaleUp();

        m_viewingCore->setFovyScale(tempScale);
        break;
    }
    case MM_ROTATE:
        m_viewingCore->rotate(  m_savedEventNDCoords, delta );
        break;
    default:
        break;
    }

    m_savedEventNDCoords = currentNDC;
    update();
}

void Osg3dView::mouseReleaseEvent(QMouseEvent *event)
{
    vDebug("mouseReleaseEvent");
    m_savedEventNDCoords = getNormalized(event->x(), event->y());
}


void Osg3dView::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
        m_viewingCore->dolly(0.5);
    else
        m_viewingCore->dolly(-0.5);
    update();
}

void Osg3dView::buildPopupMenu()
{
    QAction *a;
    QMenu *sub = m_popupMenu.addMenu("MouseMode...");

    a = sub->addAction("Orbit", this, SLOT(setMouseMode()));
    a->setData(QVariant(MM_ORBIT));
    a = sub->addAction("Pan", this, SLOT(setMouseMode()));
    a->setData(QVariant(MM_PAN));
    a = sub->addAction("Rotate", this, SLOT(setMouseMode()));
    a->setData(QVariant(MM_ROTATE));
    a = sub->addAction("Zoom", this, SLOT(setMouseMode()));
    a->setData(QVariant(MM_ZOOM));
    a = sub->addAction("Pick Center", this, SLOT(setMouseMode()));
    a->setData(QVariant(MM_PICK_CENTER));

    sub = m_popupMenu.addMenu("Std View...");
    a = sub->addAction("Top", this, SLOT(setStandardView()));
    a->setData(V_TOP);
    a = sub->addAction("Bottom", this, SLOT(setStandardView()));
    a->setData(V_BOTTOM);
    a = sub->addAction("Front", this, SLOT(setStandardView()));
    a->setData(V_FRONT);
    a = sub->addAction("Back", this, SLOT(setStandardView()));
    a->setData(V_BACK);
    a = sub->addAction("Right", this, SLOT(setStandardView()));
    a->setData(V_RIGHT);
    a = sub->addAction("Left", this, SLOT(setStandardView()));
    a->setData(V_LEFT);

    sub = m_popupMenu.addMenu("Projection...");
    a = sub->addAction("Orthographic", this, SLOT(setProjection()));
    a->setData(P_ORTHO);
    a = sub->addAction("Perspective", this, SLOT(setProjection()));
    a->setData(P_PERSP);

    sub = m_popupMenu.addMenu("DrawMode...");
    a = sub->addAction("Facets", this, SLOT(setDrawMode()));
    a->setData(D_FACET);
    a = sub->addAction("Wireframe", this, SLOT(setDrawMode()));
    a->setData(D_WIRE);
    a = sub->addAction("Points", this, SLOT(setDrawMode()));
    a->setData(D_POINT);
}

void Osg3dView::customMenuRequested(const QPoint &pos)
{
    vDebug("customMenu %d %d", pos.x(), pos.y());

    m_popupMenu.popup(this->mapToGlobal(pos));
}

void Osg3dView::fitScreenTopView(const QModelIndex &parent, int first, int last)
{
    vDebug("rowsInserted");
    m_viewingCore->viewTop();
    m_viewingCore->fitToScreen();
    update();
}

void Osg3dView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    vDebug("dataChanged");
    update();
}

void Osg3dView::setLightingTwoSided()
{
    osg::ref_ptr<osg::LightModel> lm = new osg::LightModel;
    lm->setTwoSided(true);
    lm->setAmbientIntensity(osg::Vec4(0.1f,0.1f,0.1f,1.0f));

    osg::StateSet *ss;

    for (int i=0 ; i < 2 ; i++ ) {
        ss = ((osgViewer::Renderer *)getCamera()
              ->getRenderer())->getSceneView(i)->getGlobalStateSet();

        ss->setAttributeAndModes(lm, osg::StateAttribute::ON);
    }
}


void Osg3dView::setMouseMode()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;

    MouseMode mode = static_cast<MouseMode>(a->data().toUInt());
    setMouseMode(mode);
}

void Osg3dView::setStandardView()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;

    StandardView view = static_cast<StandardView>(a->data().toUInt());
    switch (view) {
    case V_TOP: m_viewingCore->viewTop(); break;
    case V_BOTTOM: m_viewingCore->viewBottom(); break;
    case V_FRONT: m_viewingCore->viewFront(); break;
    case V_BACK: m_viewingCore->viewBack(); break;
    case V_RIGHT: m_viewingCore->viewRight(); break;
    case V_LEFT: m_viewingCore->viewLeft(); break;
    }
    update();
}

void Osg3dView::setDrawMode()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;

    DrawMode drawMode = static_cast<DrawMode>(a->data().toUInt());
    switch (drawMode) {
    case D_FACET:
        break;
    case D_WIRE:
        break;
    case D_POINT:
        break;
    }
}

void Osg3dView::setProjection()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;

    Projection projType = static_cast<Projection>(a->data().toUInt());
    switch (projType) {
    case P_ORTHO:
        m_viewingCore->setOrtho(true);
        break;
    case P_PERSP:
        m_viewingCore->setOrtho(false);
        break;
    }
    update();
}
