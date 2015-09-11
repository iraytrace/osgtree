#include "OSGWidget.h"
#include "OsgDocument.h"
#include <math.h>
#include <QApplication>
#include <QDesktopWidget>

#include <QtGui/QKeyEvent>
#include <osg/LightModel>
#include <osgViewer/Renderer>
#include <osgGA/TrackballManipulator>

#include <QAction>

OSGWidget::OSGWidget(QWidget *parent)
    : QGLWidget(parent)
    , m_mouseButtons(Qt::NoButton)
    , m_mouseModifiers(Qt::NoModifier)
    , m_mouseMode(MM_ORBIT)
    , m_viewingCore(new ViewingCore)
    , m_lastMouseEventTime(QTime::currentTime())
    , m_mouseReleaseJustHappened(false)
    , m_throwTimeTolerance(100)
    , m_throwDistTolerance(0.1)
{
    setObjectName("osgv");
    // Strong focus policy needs to be set to capture keyboard events
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true); // get mouse move events even without button press

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

    // wake up and re-draw the window every so often
    connect(&m_redrawTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
    m_redrawTimer.setSingleShot(true);
    m_redrawTimer.start(20);
    //m_frameTimer.start();

    // Signal-slot to handle animating the camera for 'tossing' the geometry
    connect(&m_animateTimer, SIGNAL(timeout()),
            this, SLOT(animateCameraThrow()));

    // Set the minimum size for this viewer window
    setMinimumSize(200, 200);
}


//#define DEBUG_MOUSE 1

void OSGWidget::mousePressEvent( QMouseEvent* event )
{
    m_animateTimer.stop();

    m_mouseReleaseJustHappened = false;

    m_lastMouseEventNDCoords = getNormalized(event->x(), event->y());

    m_mouseButtons |= event->buttons();

    m_mouseModifiers = event->modifiers();

    // handle overrides
    if (m_mouseButtons == Qt::LeftButton) {
        if (m_mouseModifiers == Qt::ShiftModifier) {
            pushState();
            m_viewingCore->setViewingCoreMode(ViewingCore::THIRD_PERSON);
            setMouseMode(MM_PAN);
        } else if (m_mouseModifiers == Qt::ControlModifier) {
            pushState();
            m_viewingCore->setViewingCoreMode(ViewingCore::THIRD_PERSON);
            setMouseMode(MM_ORBIT);
        } else if ((unsigned)m_mouseModifiers == (unsigned)(Qt::ControlModifier | Qt::ShiftModifier) ) {
            pushState();
            m_viewingCore->setViewingCoreMode(ViewingCore::THIRD_PERSON);
            setMouseMode(MM_ZOOM);
        }
    } else if (m_mouseButtons == Qt::RightButton) {
        m_viewingCore->pickCenter(m_lastMouseEventNDCoords.x(),
                                  m_lastMouseEventNDCoords.y() );
    }

    // Do the job asked
    if (m_mouseMode & (MM_PAN|MM_ROTATE|MM_ORBIT|MM_ZOOM) )
        m_viewingCore->setPanStart( m_lastMouseEventNDCoords.x(), m_lastMouseEventNDCoords.y() );
    else if (m_mouseMode & MM_PICK_CENTER) {
        m_viewingCore->pickCenter(m_lastMouseEventNDCoords.x(),
                                  m_lastMouseEventNDCoords.y() );
    }

    m_lastMouseEventTime.start();
}


void OSGWidget::mouseReleaseEvent( QMouseEvent* event )
{
    m_mouseReleaseJustHappened = true;

    m_lastMouseEventNDCoords = getNormalized(event->x(), event->y());

    m_mouseButtons &= event->buttons();

    if (m_mouseModifiers != Qt::NoModifier) {
        popState();
    }

    m_mouseModifiers = Qt::NoModifier;
    m_lastMouseEventTime.start();
}

void OSGWidget::mouseMoveEvent( QMouseEvent* event )
{
    // either a button is pressed or a mouse release just happened

    osg::Vec2d currentNDC = getNormalized(event->x(), event->y());

    if ( m_mouseReleaseJustHappened) {
        // We are potentially starting a throw of the camera
        // Look to see if we meet the tolerance criteria for space and time
        // for starting camera animation

        int deltaT = m_lastMouseEventTime.elapsed();

        if (deltaT < m_throwTimeTolerance) {
            // compute the motion of the cursor

            float distance = inchesToLastMouseEvent(event);
            if (distance > m_throwDistTolerance) {
                // start animation

                m_animateStart = m_lastMouseEventNDCoords;

                osg::Vec2d kalmanNDC = (currentNDC + m_predictedPos) * 0.5;
                m_animateDelta = kalmanNDC - m_animateStart;

                m_animateTimer.start(deltaT);
            }
        }
        m_mouseReleaseJustHappened = false;
        return;
    }


    if (m_mouseButtons == Qt::NoButton)
        return;

    osg::Vec2d delta = currentNDC - m_lastMouseEventNDCoords;

    switch (m_mouseMode) {
    case MM_ORBIT:
        m_viewingCore->rotate(  m_lastMouseEventNDCoords, delta );
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
        m_viewingCore->rotate(  m_lastMouseEventNDCoords, delta );
        break;
    default:
        break;
    }

    m_predictedPos.set(currentNDC.x() + delta.x(), currentNDC.y() + delta.y());

    m_lastMouseEventNDCoords = currentNDC;
    m_lastMouseEventTime.start();
}

void OSGWidget::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
        m_viewingCore->dolly(0.5);
    else
        m_viewingCore->dolly(-0.5);
}

void OSGWidget::paintGL()
{
    // Update the camera
    osg::Camera *cam = this->getCamera();
    const osg::Viewport* vp = cam->getViewport();

    m_viewingCore->setAspect(vp->width() / vp->height());

    cam->setViewMatrix(m_viewingCore->getInverseMatrix());
    cam->setProjectionMatrix(m_viewingCore->computeProjection());

    // Invoke the OSG traversal pipeline
    frame();

    // Start the timer again
    m_redrawTimer.start();
}



void OSGWidget::animateCameraThrow()
{
    switch (m_mouseMode) {
    case MM_ORBIT:
        m_viewingCore->rotate( m_animateStart, m_animateDelta );
        break;
    case MM_PAN:
        m_viewingCore->pan(m_animateDelta.x(), m_animateDelta.y());
        break;
    case MM_ZOOM: {
        double tempScale = m_viewingCore->getFovyScale();
        m_viewingCore->setFovyScale(1.03);

        if(m_animateDelta.y() > 0)
            m_viewingCore->fovyScaleDown();
        else
            m_viewingCore->fovyScaleUp();

        m_viewingCore->setFovyScale(tempScale);
        break;
    }
    case MM_ROTATE:
        m_viewingCore->rotate( m_animateStart, m_animateDelta );
        break;
    default:
        break;
    }
}



void OSGWidget::setScene(osg::Node *root)
{
    this->setSceneData(root);
    m_viewingCore->setSceneData(root);
    m_viewingCore->fitToScreen();
}

void OSGWidget::setLightingTwoSided()
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

void OSGWidget::resizeGL(int width, int height)
{
    m_osgGraphicsWindow->getEventQueue()->windowResize(0, 0, width, height );
    m_osgGraphicsWindow->resized(0,0,width,height);
}

osg::Vec2d OSGWidget::getNormalized(const int ix, const int iy)
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

void OSGWidget::setMouseMode(MouseMode mode)
{
    m_mouseMode = mode;

    if(mode == MM_ROTATE)
        m_viewingCore->setViewingCoreMode( ViewingCore::FIRST_PERSON );
    else {
        m_viewingCore->setViewingCoreMode( ViewingCore::THIRD_PERSON );
    }

    emit mouseModeChanged(mode);
    m_animateTimer.stop();
}



float OSGWidget::inchesToLastMouseEvent(QMouseEvent* thisMouseEvent)
{
    // we don't really need the x, y values but the width/height are important
    int x, y, width, height;
    m_osgGraphicsWindow->getWindowRectangle(x, y, width, height);

    // un-normalize last event
    float center = width/2.0;
    int oldX = (m_lastMouseEventNDCoords.x() * center) + center;

    center = height/2.0;
    int oldY = (m_lastMouseEventNDCoords.y() * center) + center;

    float deltaX = thisMouseEvent->x() - oldX;
    float deltaY = (height - thisMouseEvent->y()) - oldY;

    float distXinches = deltaX / (float)qApp->desktop()->logicalDpiX();
    float distYinches = deltaY / (float)qApp->desktop()->logicalDpiY();

    float distSquared = distXinches * distXinches +
                        distYinches * distYinches;

    return sqrt(distSquared);
}


void OSGWidget::pushState()
{
    stashState state;
    state.mm = m_mouseMode;
    state.vcm = m_viewingCore->getViewingCoreMode();
    m_stashState.push_back(state);
}


void OSGWidget::popState()
{
    if (m_stashState.size() < 1)
        return;

    stashState state = m_stashState.last();
    m_stashState.pop_back();
    m_viewingCore->setViewingCoreMode(state.vcm);
    setMouseMode(state.mm);
}

