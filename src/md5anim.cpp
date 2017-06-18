#include "md5anim.h"
#include "md5parser.h"

MD5Anim::MD5Anim()
{

}

MD5Anim::~MD5Anim()
{
    while(!m_skeletonsFrames.empty())
    {
        delete m_skeletonsFrames.back();
        m_skeletonsFrames.pop_back();
    }
}

void MD5Anim::setNumFrames(int n)
{
    m_numFrames = n;
}

void MD5Anim::setNumJoints(int n)
{
    m_numJoints = n;
}

int MD5Anim::getNumJoints()
{
    return m_numJoints;
}

void MD5Anim::setFrameRate(int n)
{
    m_frameRate = n;
}

void MD5Anim::setNumAnimatedComponents(int n)
{
    m_numAnimatedComponents = n;
}

QString MD5Anim::getNameJoint(int n)
{
    return m_hierarchy[n].m_name;
}

int MD5Anim::getParentJoint(int n)
{
    return m_hierarchy[n].m_parent;
}

void MD5Anim::addHierarchy(QString name, int parent, int flags, int startIndex)
{
    MD5AnimHierarchy hierarchy;
    hierarchy.m_name = name;
    hierarchy.m_parent = parent;
    hierarchy.m_flags = flags;
    hierarchy.m_startIndex = startIndex;

    m_hierarchy.append(hierarchy);
}

void MD5Anim::addBounds(QVector3D min, QVector3D max)
{
    MD5AnimBounds bounds;
    bounds.m_min = min;
    bounds.m_max = max;

    m_bounds.append(bounds);
}

void MD5Anim::addBaseFrame(QVector3D position, QQuaternion orientation)
{
    MD5AnimBaseFrame base_frame;
    base_frame.m_position = position;
    base_frame.m_orientation = orientation;

    m_baseFrame.append(base_frame);
}

void MD5Anim::addFrame(int index, QVector<float> datas)
{
    MD5AnimFrame frame;
    frame.m_index = index;
    frame.m_datas = datas;

    m_frames.append(frame);
}

void MD5Anim::buildSkeletonsFrames()
{
    for(int frame=0;frame<m_numFrames;++frame)
    {
        MD5Skeleton *skeleton = new MD5Skeleton();
        for(int i=0;i<m_numJoints;++i)
        {
            int parent_indice = m_hierarchy[i].m_parent;
            QVector3D position = m_baseFrame[i].m_position;
            QQuaternion orientation = m_baseFrame[i].m_orientation;

            unsigned int j = 0;
            if ( m_hierarchy[i].m_flags & 1 ) // Pos.x
            {
                position.setX(m_frames[frame].m_datas[ m_hierarchy[i].m_startIndex + j++ ]);
            }
            if ( m_hierarchy[i].m_flags & 2 ) // Pos.y
            {
                position.setY(m_frames[frame].m_datas[ m_hierarchy[i].m_startIndex + j++ ]);
            }
            if ( m_hierarchy[i].m_flags & 4 ) // Pos.x
            {
                position.setZ(m_frames[frame].m_datas[ m_hierarchy[i].m_startIndex + j++ ]);
            }
            if ( m_hierarchy[i].m_flags & 8 ) // Orient.x
            {
                orientation.setX(m_frames[frame].m_datas[ m_hierarchy[i].m_startIndex + j++ ]);
            }
            if ( m_hierarchy[i].m_flags & 16 ) // Orient.y
            {
                orientation.setY(m_frames[frame].m_datas[ m_hierarchy[i].m_startIndex + j++ ]);
            }
            if ( m_hierarchy[i].m_flags & 32 ) // Orient.z
            {
                orientation.setZ(m_frames[frame].m_datas[ m_hierarchy[i].m_startIndex + j++ ]);
            }

            orientation = MD5Parser::MakeUnitQuaternion(orientation.x(),orientation.y(),orientation.z());
            if(parent_indice != -1)
            {
                //Position
                QVector3D position_rotated = skeleton->getJointOrientation(parent_indice).rotatedVector(position);
                position = skeleton->getJointPosition(parent_indice) + position_rotated;

                //Orientation
                orientation = skeleton->getJointOrientation(parent_indice) * orientation;
                orientation.normalize();
            }
            skeleton->addJoint(parent_indice,position,orientation);
        }
        m_skeletonsFrames.append(skeleton);
    }
}

bool MD5Anim::getSkeletonFromDuration(MD5Skeleton *skeleton, double &duration, bool loop)
{
    if(m_frameRate<1){return false;}

    double frame_time = 1/(double)m_frameRate;
    double anim_duration = frame_time*(m_numFrames-1);

    if(duration > anim_duration)
    {
        if(!loop)
        {
            skeleton->copyFromSkeleton(m_skeletonsFrames[m_numFrames-1]);
            return true;
        }
        else
        {
            while(duration > anim_duration)
            {
                duration -= anim_duration;
            }
        }
    }

    int current_frame = qFloor(duration * m_frameRate);

    MD5Skeleton *skeleton_previous_frame = m_skeletonsFrames[current_frame];
    MD5Skeleton *skeleton_next_frame = m_skeletonsFrames[current_frame+1];

    double interpolation = fmod(duration,frame_time)/frame_time;

    MD5Skeleton::getInterpolationSkeleton(skeleton_previous_frame,skeleton_next_frame,skeleton,interpolation);
    return false;
}
