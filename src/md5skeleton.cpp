#include "md5skeleton.h"

struct MD5VertexColor
{
    QVector3D position;
    QVector3D color;
};

MD5Skeleton::MD5Skeleton():
    m_arrayPointBuf(QOpenGLBuffer::VertexBuffer),
    m_indexPointBuf(QOpenGLBuffer::IndexBuffer),
    m_arrayLineBuf(QOpenGLBuffer::VertexBuffer),
    m_indexLineBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();
}

MD5Skeleton::~MD5Skeleton()
{
    if(m_arrayPointBuf.isCreated())
        m_arrayPointBuf.destroy();

    if(m_indexPointBuf.isCreated())
        m_indexPointBuf.destroy();

    if(m_arrayLineBuf.isCreated())
        m_arrayLineBuf.destroy();

    if(m_indexLineBuf.isCreated())
        m_indexLineBuf.destroy();
}

void MD5Skeleton::addJoint(int parentIndice, QVector3D position, QQuaternion orientation)
{
    MD5SkeletonJoint joint;
    joint.m_parentIndex=parentIndice;
    joint.m_position=position;
    joint.m_orientation=orientation;

    m_joints.append(joint);
}

QVector3D MD5Skeleton::getJointPosition(int i)
{
    return m_joints[i].m_position;
}

QQuaternion MD5Skeleton::getJointOrientation(int i)
{
    return m_joints[i].m_orientation;
}

void MD5Skeleton::draw(QOpenGLShaderProgram *lineShader, QOpenGLShaderProgram *pointShader)
{
    if(!m_arrayPointBuf.isCreated() || !m_arrayLineBuf.isCreated() ||
            !m_indexPointBuf.isCreated() || !m_indexLineBuf.isCreated())
    {
        qDebug() << "Skeleton VBOs not inited";
        return;
    }

    //Points
    pointShader->bind();
    m_arrayPointBuf.bind();
    m_indexPointBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = pointShader->attributeLocation("position");
    pointShader->enableAttributeArray(vertexLocation);
    pointShader->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = pointShader->attributeLocation("color");
    pointShader->enableAttributeArray(colorLocation);
    pointShader->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_POINTS, m_joints.size(), GL_UNSIGNED_INT, 0);

    m_arrayPointBuf.release();
    m_arrayPointBuf.release();
    pointShader->release();

    //Lines
    lineShader->bind();
    m_arrayLineBuf.bind();
    m_indexLineBuf.bind();

    // Offset for position
    offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    vertexLocation = lineShader->attributeLocation("position");
    lineShader->enableAttributeArray(vertexLocation);
    lineShader->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    colorLocation = lineShader->attributeLocation("color");
    lineShader->enableAttributeArray(colorLocation);
    lineShader->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_LINES, (m_joints.size()-1)*2 , GL_UNSIGNED_INT, 0);

    m_arrayLineBuf.release();
    m_indexLineBuf.release();
    lineShader->release();
}

void MD5Skeleton::prepareDrawing()
{
    m_arrayPointBuf.create();
    m_indexPointBuf.create();

    m_arrayLineBuf.create();
    m_indexLineBuf.create();

    QVector<MD5VertexColor> points;
    QVector<MD5VertexColor> lines;
    QVector<GLuint> index_points;
    QVector<GLuint> index_lines;

    for (int i = 0; i < m_joints.size(); ++i)
    {
        MD5VertexColor v;
        v.position = (m_joints[i].m_position);
        v.color = QVector3D(1.0f,0.0f,0.0f);

        points.append(v);
        index_points.append((GLuint)i);
    }

    int j =0;
    for (int i = 0; i < m_joints.size(); ++i)
    {
        if (m_joints[i].m_parentIndex != -1)
        {
            MD5VertexColor v1,v2;
            v1.position = (m_joints[m_joints[i].m_parentIndex].m_position);
            v1.color = QVector3D(0.0f,1.0f,0.0f);
            v2.position = (m_joints[i].m_position);
            v2.color = QVector3D(0.0f,1.0f,0.0f);

            lines.append(v1);
            index_lines.append((GLuint)(j*2));

            lines.append(v2);
            index_lines.append((GLuint)(j*2+1));

            j++;
        }
    }

    m_arrayPointBuf.bind();
    m_arrayPointBuf.allocate(&points[0], points.size() * sizeof(MD5VertexColor));
    m_arrayPointBuf.release();

    m_indexPointBuf.bind();
    m_indexPointBuf.allocate(&index_points[0], index_points.size() * sizeof(GLuint));
    m_indexPointBuf.release();

    m_arrayLineBuf.bind();
    m_arrayLineBuf.allocate(&lines[0], lines.size() * sizeof(MD5VertexColor));
    m_arrayLineBuf.release();

    m_indexLineBuf.bind();
    m_indexLineBuf.allocate(&index_lines[0], index_lines.size() * sizeof(GLuint));
    m_indexLineBuf.release();
}

void MD5Skeleton::clear()
{
    m_joints.clear();
}

void MD5Skeleton::copyFromSkeleton(MD5Skeleton *from)
{
    clear();
    for(int i=0;i<from->m_joints.size();++i)
    {
        addJoint(from->m_joints[i].m_parentIndex,
                     from->m_joints[i].m_position,
                     from->m_joints[i].m_orientation);
    }
}

bool MD5Skeleton::getInterpolationSkeleton(MD5Skeleton *skeleton_previous_frame, MD5Skeleton *skeleton_next_frame, MD5Skeleton *skeleton_interpolated, double t)
{
    if(skeleton_previous_frame->m_joints.size() != skeleton_next_frame->m_joints.size())
    {
        return false;
    }

    skeleton_interpolated->clear();
    for(int i=0;i<skeleton_previous_frame->m_joints.size();++i)
    {
        MD5SkeletonJoint &joint0 = skeleton_previous_frame->m_joints[i];
        MD5SkeletonJoint &joint1 = skeleton_next_frame->m_joints[i];

        skeleton_interpolated->addJoint(joint0.m_parentIndex,
                                        joint0.m_position + t * (joint1.m_position-joint0.m_position),
                                        QQuaternion::slerp(joint0.m_orientation,joint1.m_orientation,t));
    }

    return true;
}
