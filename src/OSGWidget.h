#ifndef OSGWIDGET_H
#define OSGWIDGET_H

#include <QTimer>
#include <QTime>
#include <QGLWidget>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include "OsgView/ViewingCore.h"

class OSGWidget : public QGLWidget,
    public osgViewer::Viewer
{
    Q_OBJECT

public:

    OSGWidget(QWidget *parent);

    // Helper types ////////////////////////////////////////////////////////////
    enum MouseMode {
        MM_ORBIT = (1<<1),
        MM_PAN = (1<<2),
        MM_ZOOM = (1<<3),
        MM_ROTATE = (1<<4),
        MM_PICK_CENTER = (1<<5)
    };

    // Overridden from QGLWidget //////////////////////////////////////////////

    // Qt Event handlers
    //void keyPressEvent(QKeyEvent* event);
    //void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *event);

    // Overridden from osgViewer::Viewer //////////////////////////////////////

    /// Let others tell what scene graph we should be drawing
    void setScene(osg::Node *);

    /// Render one frame
    void paintGL();
public slots:
    /// Set the view of the camera
    void viewTop()    {
        m_viewingCore->viewTop();
    }
    void viewbottom() {
        m_viewingCore->viewBottom();
    }
    void viewRight()  {
        m_viewingCore->viewRight();
    }
    void viewLeft()   {
        m_viewingCore->viewLeft();
    }
    void viewFront()  {
        m_viewingCore->viewFront();
    }
    void viewBack()   {
        m_viewingCore->viewBack();
    }
    void resetView()  {
        m_viewingCore->computeInitialView();
    }
    void fitToScreen() {
        m_viewingCore->fitToScreen();
    }

    /// Set the m_mouseMode variable
    void setMouseMode(MouseMode mode);
private slots:

    /// Handle camera animation for 'tossing' the geometry
    void animateCameraThrow();

signals:

    /// Let the rest of the world (OsgView) know the current MouseMode
    void mouseModeChanged(OSGWidget::MouseMode);

private:

    // Helper types ///////////////////////////////////////////////////////////

    struct stashState {
        MouseMode mm;
        ViewingCore::ViewingCoreMode vcm;
    };

    // Helper functions ///////////////////////////////////////////////////////

    /// OSG uses singleSided drawing/display by default.
    /// This is annoying when you are "inside" something and the back wall of
    /// it simply disappears. This gets called to set up to draw both front and
    /// back facing polygons so that the world behaves in a more normal fashion.
    /// Yes it's a performance hit.
    void setLightingTwoSided();

    /// Invoked whenever the window size changes so that the OpenGL viewport
    /// gets updated appropriately
    void resizeGL( int width, int height );

    /// Get the Normalized Device Coordinates for a given X,Y pixel location
    osg::Vec2d getNormalized(const int ix, const int iy);


    /// compute the distance (in inches) between m_lastMouseEventNDCCoords and
    /// the parmeter mouse event.
    float inchesToLastMouseEvent(QMouseEvent* thisMouseEvent);

    /// stash the current mouse behaviour for later
    void pushState();

    /// restore a previously saved mouse behavior
    void popState();

    // Private data ///////////////////////////////////////////////////////////



    /// The currently pressed mouse buttons
    int m_mouseButtons;

    /// The currently active modifiers (from the mousePress event)
    int m_mouseModifiers;

    /// Current mouse mode
    MouseMode m_mouseMode;

    /// OSG graphics window
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_osgGraphicsWindow;

    /// Viewing Core --> controls the camera of the osgViewer
    osg::ref_ptr< ViewingCore > m_viewingCore;

    /// Redraw the OSG window every time this timer signals
    QTimer m_redrawTimer;

    /// When this ticks update the camera position to support animation
    /// If the time to draw a frame is large then multiple of these will be
    /// accumulated at once and arguably should only be done once.  However if
    /// the redraw time is very small compared to this timer then the angular
    /// difference frame to frame might become very small and induce floating
    /// point error.
    QTimer m_animateTimer;

    /// The position (in Normalized Device Coordinates) of the last mouse event
    osg::Vec2d m_lastMouseEventNDCoords;

    /// The time at which the last mouse event occured.
    QTime m_lastMouseEventTime;

    QList<stashState> m_stashState;

    // Throwing support //////////////////////////////////////////////////

    /// flag: the last mouse event was a release
    bool m_mouseReleaseJustHappened;

    /// if a mouse move event happens in less time than this after a mouse
    /// release event then we are possibly throwing the display
    int m_throwTimeTolerance;

    /// If the mouse moves more than this distance (inches) from the time
    /// of the mouse is released to the next mouse move event, we are throwing
    float m_throwDistTolerance; ///< if moues moves more than this din

    /// These three Vec2d are NDC coordinates used for doing animation.

    /// The location of the event prior to the start of animation
    /// Initialized from m_lastMouseEventNDCoords
    osg::Vec2d m_animateStart;

    /// delta from m_aimateStart to event that causes animation to start
    osg::Vec2d m_animateDelta;

    /// prediction of location of next mouse event
    osg::Vec2d m_predictedPos;
};

#endif // OSGVIEW_H
