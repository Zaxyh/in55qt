#ifndef MD5MESH_H
#define MD5MESH_H

#include "md5anim.h"
#include "md5skeleton.h"

#include <QQuaternion>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QString>
#include <QVector>
#include <QVector2D>
#include <QVector3D>

class MD5Mesh : protected QOpenGLFunctions
{
public:
    struct MD5MeshVertex
    {
        QVector2D st;
        int startWeight;
        int countWeight;
    };

    struct MD5MeshTriangle
    {
        uint vertex0,vertex1,vertex2;
    };

    struct MD5MeshWeight
    {
        int jointIndice;
        float bias;
        QVector3D position;
    };

    struct MD5MeshJoint
    {
        QString name;
        int parentIndex;
        QQuaternion orientation;
        QVector3D position;
    };

    struct MD5MeshMesh
    {
        QOpenGLTexture *shader = NULL;

        int numVerts;
        QVector<MD5MeshVertex> vertices;

        int numTris;
        QVector<MD5MeshTriangle> triangles;

        int numWeights;
        QVector<MD5MeshWeight> weights;

        QOpenGLBuffer arrayBuf;
        QOpenGLBuffer indexBuf;
    };

    MD5Mesh();
    ~MD5Mesh();

    void addJoint(QString name, int parentIndex, QVector3D position, QQuaternion orientation);
    int addMesh();

    void addVertexInMesh(int mesh_indice, QVector2D st, int startWeight, int countWeight);
    void addTriangleInMesh(int mesh_indice, int vertex0, int vertex1, int vertex2);
    void addWeightInMesh(int mesh_indice, int jointIndice, float bias, QVector3D position);

    void setNumJoints(int n);
    int getNumJoints();
    void setNumMeshes(int n);
    int getNumMeshes();

    void setNumVerticesInMesh(int mesh_indice, int n);
    int getNumVerticesInMesh();
    void setNumTrianglesInMesh(int mesh_indice, int n);
    int getNumTrianglesInMesh();
    void setNumWeightsInMesh(int mesh_indice, int n);
    int getNumWeightsInMesh();

    void setShaderInMesh(int mesh_indice, QOpenGLTexture *texture);
    QString getShaderInMesh();

    bool isCorrupted();
    bool checkAnimation(MD5Anim *anim);

    void prepareDrawing();

    void setDefaultSkeleton();
    void setSkeleton(MD5Skeleton *skeleton);

    void draw(QOpenGLShaderProgram *program);
    void drawSkeleton(QOpenGLShaderProgram *programLines, QOpenGLShaderProgram *programPoints);

    QString toString(bool more = false);
private:
    int m_numJoints;
    int m_numMeshes;

    QVector<MD5MeshMesh> m_meshes;
    QVector<MD5MeshJoint> m_joints;

    MD5Skeleton *m_skeleton;
};

#endif // MD5MESH_H
