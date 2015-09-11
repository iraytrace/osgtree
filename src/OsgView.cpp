#include "OsgView.h"

#include <QMenu>
#include "OsgItemModel.h"

#include <osg/LightModel>
#include <osgViewer/Renderer>

OsgView::OsgView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_viewingCore(new ViewingCore)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customMenuRequested(QPoint)));

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

void OsgView::setScene(OsgItemModel *model)
{
    qDebug("setScene");
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));

    osg::ref_ptr<osg::Group> root = model->getRoot();
    this->setSceneData(root);
    m_viewingCore->setSceneData(root);
}

void OsgView::paintGL()
{
    qDebug("paintGL");

    // Update the camera
    osg::Camera *cam = this->getCamera();
    const osg::Viewport* vp = cam->getViewport();

    m_viewingCore->setAspect(vp->width() / vp->height());

    cam->setViewMatrix(m_viewingCore->getInverseMatrix());
    cam->setProjectionMatrix(m_viewingCore->computeProjection());

    // Invoke the OSG traversal pipeline
    frame();
}

void OsgView::resizeGL(int w, int h)
{
    qDebug("resizeGL");

    m_osgGraphicsWindow->getEventQueue()->windowResize(9, 0, w, h);
    m_osgGraphicsWindow->resized(0,0,w,h);
}

void OsgView::hello()
{
    qDebug("hello");
}

void OsgView::mousePressEvent(QMouseEvent *event)
{
    qDebug("mousePressEvent");
    if (event->button() == Qt::RightButton)
        customMenuRequested(event->pos());
}

void OsgView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug("mouseReleaseEvent");
}

void OsgView::mouseMoveEvent(QMouseEvent *event)
{
    qDebug("mouseMoveEvent");
}

void OsgView::wheelEvent(QWheelEvent *event)
{

}

void OsgView::customMenuRequested(const QPoint &pos)
{
    qDebug("customMenu");
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Action 1", this));
    menu->addAction(new QAction("Action 2", this));
    menu->addAction(new QAction("Action 3", this));
    menu->popup(this->mapToGlobal(pos));
}

void OsgView::columnsInserted(const QModelIndex &parent, int first, int last)
{
    qDebug("columnsInserted");

}

void OsgView::rowsInserted(const QModelIndex &parent, int first, int last)
{
    qDebug("rowsInserted");
    m_viewingCore->viewTop();
    m_viewingCore->fitToScreen();
    update();
}

void OsgView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    qDebug("dataChanged");
}

void OsgView::setLightingTwoSided()
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

