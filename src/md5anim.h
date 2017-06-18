#ifndef MD5ANIM_H
#define MD5ANIM_H

#include "md5skeleton.h"

#include <QQuaternion>
#include <QVector>
#include <QVector3D>
#include <QtMath>

class MD5Anim
{
public:
    struct MD5AnimHierarchy
    {
        QString m_name;
        int m_parent;
        int m_flags;
        int m_startIndex;
    };

    struct MD5AnimBounds
    {
        QVector3D m_min;
        QVector3D m_max;
    };

    struct MD5AnimBaseFrame
    {
        QVector3D m_position;
        QQuaternion m_orientation;
    };

    struct MD5AnimFrame
    {
        int m_index;
        QVector<float> m_datas;
    };

    MD5Anim();
    ~MD5Anim();

    void addHierarchy(QString name, int parent, int flags, int startIndex);
    void addBounds(QVector3D min, QVector3D max);
    void addBaseFrame(QVector3D position, QQuaternion orientation);
    void addFrame(int index, QVector<float> datas);

    void setNumFrames(int n);
    void setNumJoints(int n);
    int getNumJoints();
    void setFrameRate(int n);
    void setNumAnimatedComponents(int n);

    QString getNameJoint(int n);
    int getParentJoint(int n);

    void buildSkeletonsFrames();
    bool getSkeletonFromDuration(MD5Skeleton *skeleton, double &duration, bool loop);
private:
    int m_numFrames;
    int m_numJoints;
    int m_frameRate;
    int m_numAnimatedComponents;

    QVector<MD5AnimHierarchy> m_hierarchy;
    QVector<MD5AnimBounds> m_bounds;
    QVector<MD5AnimBaseFrame> m_baseFrame;
    QVector<MD5AnimFrame> m_frames;

    QVector<MD5Skeleton*> m_skeletonsFrames;
};

#endif // MD5ANIM_H
