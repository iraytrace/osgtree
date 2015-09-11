// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __VIEWING_CORE_H__
#define __VIEWING_CORE_H__


#include <osg/Object>
#include <osg/Node>
#include <osg/Matrixd>
#include <cmath>

//#include <QTextStream> // this should get replaced by a C++11
#include <iostream>
#include <sstream>

/** \brief A GUI-independent class for maintaining view and projection matrix parameters.
 *
 * This camera model primarily replaces OpenSceneGraph's camera model with a
 * better one.  In particular, it allows switching between Orthographic and
 * Perspective modes with reasonable correspondence of the view.  It allows
 * better interaction and inspection of camera parameters, and provides for
 * rotation about both the view center and the eye position.
 */
class ViewingCore : public osg::Object
{
public:
    ViewingCore();
    ViewingCore( const ViewingCore& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );

    META_Object(osgwTools,ViewingCore)


    /** Specify the scene. Used to create appropriate view volume boundaries
    when switching from perspective to ortho in the setOrtho() function. Used
    by computeProjection() to compute appropriate zNear and zFar planes. */
    void setSceneData( osg::Node* scene );

    void computeInitialView();


    /** Return a matrix using \c _viewDistance and \c _viewCenter as a translation,
    and \c _viewDir and \c _viewUp as an orientation basis. */
    osg::Matrixd getMatrix() const;
    /** Just orientation, without the translation. */
    osg::Matrixd getOrientationMatrix() const;
    /** Return the inverse of the getMatrixui->osgv->saveView()() function. */
    osg::Matrixd getInverseMatrix() const;

    /** Causes the view to rotate. How the code changes the view depends on
    the viewing mode (setViewingCoreMode()).

    In FIRST_PERSON mode, the view direction changes while the view position
    remains constant. The \c start parameter is ignored, \c dir[0] is taken as
    a delta left/right angle in radians, and \c dir[1] is taken as a delta
    up/down angle in radians.

    In THIRD_PERSON mode, the code rotates the view position around a
    constant center point (selected with pickCenter()). Rotation emulates a
    trackball. The \c start parameter indicates a normalized location on the
    trackball surface, with (0.,0.) corresponding to the lower left window
    corner and (1.,1.) to the upper right. \c dir indicates the direction of
    rotation in normalized window space. Rotation alters the view up vector.
    */
    void rotate( osg::Vec2d start, osg::Vec2d dir );

    /** Pass in a delta motion vector. pan() creates a viewer coordinate
    system in which +x to the right and +y up, then changes
    the view center by \c deltaMovement in this coordinate system. */
    void pan( const double ndcX, const double ndcY );

    /** Dolly the camera forwards and backwards. Changes the view distance.
    This function is a no-op for orthographic projections. */
    void dolly( const double deltaMovement );

    /** Set the initial location for panning. The UI typically calls this function for
    a click event, then calls \c pan() for subsequent drag events.

    This function creates a pan plane that is used by \c pan() to calculate the world
    coordinate motion. To define the pan plane, this function performs an intersection
    test with the scene. If an intersection occurs, the picked point is used to define
    the plane. Otherwise, \c _viewCenter is used. The plane created passes through the
    point (either the intersection point or \c _viewCenter and uses \c _viewDir as the
    plane normal. */
    void setPanStart( const double ndcX, const double ndcY );

    typedef enum {
        FIRST_PERSON,
        THIRD_PERSON
    } ViewingCoreMode;
    /** Specify FIRST_PERSON or the THIRD_PERSON viewing modes. The viewing mode
    is THIRD_PERSON by default. The viewing mode affects interpretation of
    the rotate() parameters. See rotate() for more information. */
    void setViewingCoreMode( ViewingCoreMode mode );
    ViewingCoreMode getViewingCoreMode() const {
        return( _mode );
    }

    /** Select a new view center. This function back-transforms the input NDC xy values
    into world space and uses them to create a LineSegmentIntersector to pick a new
    \c _center for the view matrix. */
    void pickCenter( const double ndcX, const double ndcY );

    /** Get the current eye position. */
    inline osg::Vec3d getEyePosition() const {
        return( _viewCenter - ( _viewDir * _viewDistance ) );
    }

    /** When the viewing mode is THIRD_PERSON, the code emulates a trackball
    in which mouse motion along the window boundary rolls the current view.
    The application can use this function to adjust the sensitivity of the
    emulated trackball. Smaller values produui->osgv->saveView()ce less of a roll effect, while
    larger values produce more pronounced roll effects.

    \c rollSensitivity is literally the maximum angle of the trackball's
    rotation axis relative to the view plane. This value is scaled by
    the mouse's normalized distance from the window center.
    \param rollSensitivity Clamped to range 0.0 to pi/2. Default is 1.3. */
    void setTrackballRollSensitivity( double rollSensitivity );

    /** Set world space vectors used to compute yaw/pitch/roll (see
    getYawPitchRoll() ). The \c up vector corresponds to pitch (pi/2)
    and defines a horison plane corresponding to pitch 0.0. The \c north
    vector corresponds to yaw 0.0. */
    void setBaseUpNorth( const osg::Vec3d& up, const osg::Vec3d& north );

    /** Get current yaw/pitch/roll angles for the current view.
    Values are computed relative to the base up and north vectors
    (see setBaseUpNorth() ). All return values are in degrees.
    \param yaw Heading value. 0.0 <= yaw < 360.0.
    \param pitch Elevation value. -(pi/2.0) <= pitch <= (pi/2.0).
    \param roll Twist value. 0.0 <= roll < 360.0.
    \param rightHanded Use a right-handed coordinate system to compute
    yaw and roll angles. Turning left increases the yaw angle when rightHanded=true,
    and decreases the yaw angle when rightHanded=false. Likewise, counter-clockwise
    roll increases the roll angle when rightHanded=true, but decreases the roll angle
    when rightHanded=false. */
    void getYawPitchRoll( double& yaw, double& pitch, double& roll, bool rightHanded=false ) const;


    /** Specify whether or not to use an orthographic projection.
    Specify \c true to enable orthographic mode, and false to disable
    orthographic mode (and use perspective instead). The default is
    false (perspective). */
    void setOrtho( bool ortho );
    bool getOrtho() const {
        return( _ortho );
    }

    /** Set the aspect ratio. This value is used in the computation of the projection
    matrix. */
    void setAspect( double aspect ) {
        _aspect = aspect;
    }

    /** Modify \c proj so that it used \c _fovy for its field of view in y,
    maintaining the same aspect ratio, and near and far plane values. This
    function works for both symmetrical and assymetrical view volumes. */
    void updateFovy( osg::Matrixd& proj ) const;
    /** Conpute a projection matrix from specified aspect and fovy. Creates
    a symmetrical projection matrix. zNear and zFar planes are computed from
    the proximity of view position to scene data. */
    osg::Matrixd computeProjection() const;

    /** Set the field of view in y (fovy) in degrees. Default is 30 degrees. */
    void setFovy( double fovy );
    double getFovy() const {
        return( _fovy );
    }
    double getFovyRadians() const;

    /** Scale fovy up or down, using the scale value set with setFovyScale(). */
    void fovyScaleUp();
    void fovyScaleDown();

    /** Percentage to increase the fovy in a fovyScaleUp() call.
    For example, to increase fovy by 120% in that call, pass 1.2. Default is
    1.1 (110%). The inverse (1.0 / fovyScale) is used in the fovyScaleDown()
    call. */
    void setFovyScale( double fovyScale ) {
        _fovyScale = fovyScale;
    }
    double getFovyScale() const {
        return( _fovyScale );
    }

    /** Default is to clamp fovy scaling to the range (1.0,160.0). Pass \c false
    as first paramter to disable clamping. */
    void setClampFovyScale( bool clamp, osg::Vec2d clampFovyRange=osg::Vec2d(1.0,140.0) );
    osg::Vec2d getClampFovyScale() {
        return _clampFovyRange;
    }

    void viewTop();
    void viewBottom();
    void viewRight();
    void viewLeft();
    void viewFront();
    void viewBack();

    //Used for moving the wand
    osg::Vec3d findDeltaOnPanPlane(const double ndcX1, const double ndcY1,
                                   const double ndcX2, const double ndcY2);

    osg::Vec3d getViewDir() {
        return _viewDir;
    }

    void getZNearZFarProj(double &zNear, double &zFar, osg::Matrixd &projMat);

    void fitToScreen();

    void saveView(std::stringstream &stream);
    void loadView(std::stringstream &stream);

    osg::Vec3d getViewCenter() const {
        return _viewCenter;
    }
protected:
    ~ViewingCore();


    bool intersect( osg::Vec3d& result, const osg::Vec3d& farPoint );

    /** OSG doesn't appear to have a utility function to intersect a plane and a ray.
    TBD This should probably go in osgWorks. */
    bool intersectPlaneRay( osg::Vec3d& result, const osg::Vec4d& plane, const osg::Vec3d& p0, const osg::Vec3d& p1 );

    double _trackballRollSensitivity;
    osg::Vec3d _baseUp, _baseNorth;
    osg::Vec3d _lastCross, _viewUp, _viewDir, _viewCenter;
    double _viewDistance;
    double _initialViewDistance;

    osg::Vec4d _panPlane;

    ViewingCoreMode _mode;


    // Projection matrix and field of view support.
    bool _ortho;
    osg::ref_ptr< osg::Node > _scene;
    double _aspect;

    double _fovy;
    double _fovyScale;
    bool _clampFovyScale;
    osg::Vec2d _clampFovyRange;
    double _orthoBottom, _orthoTop;
};





// __VIEWING_CORE_H__
#endif
