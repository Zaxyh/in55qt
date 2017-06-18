#ifndef MD5SKELETON_H
#define MD5SKELETON_H

#include <QQuaternion>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector>
#include <QVector3D>

class MD5Skeleton : protected QOpenGLFunctions
{
public:
    struct MD5SkeletonJoint
    {
        int m_parentIndex;
        QVector3D m_position;
        QQuaternion m_orientation;
    };

    MD5Skeleton();
    ~MD5Skeleton();

    void addJoint(int parentIndice, QVector3D position, QQuaternion orientation);
    QVector3D getJointPosition(int i);
    QQuaternion getJointOrientation(int j);

    void draw(QOpenGLShaderProgram *lineShader, QOpenGLShaderProgram *pointShader);
    void prepareDrawing();

    void clear();
    void copyFromSkeleton(MD5Skeleton *from);

    static bool getInterpolationSkeleton(MD5Skeleton *skeleton_previous_frame, MD5Skeleton *skeleton_next_frame, MD5Skeleton *skeleton_interpolated, double t);
private:
    QVector<MD5SkeletonJoint> m_joints;

    QOpenGLBuffer m_arrayPointBuf;
    QOpenGLBuffer m_indexPointBuf;

    QOpenGLBuffer m_arrayLineBuf;
    QOpenGLBuffer m_indexLineBuf;
};

#endif // MD5SKELETON_H
