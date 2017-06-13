#ifndef MD5MESH_H
#define MD5MESH_H

#include "QQuaternion"
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include "QString"
#include "QVector"
#include "QVector2D"
#include "QVector3D"

class MD5Mesh : protected QOpenGLFunctions
{
public:
    struct MD5MeshVertex
    {
        QVector2D st;
        int startWeight;
        int countWeight;

        QVector3D computedPosition;
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
        int parentIndice;
        QQuaternion orientation;
        QVector3D position;
    };

    struct MD5MeshMesh
    {
        QString shader;

        int numVerts;
        QVector<MD5MeshVertex> vertices;

        int numTris;
        QVector<MD5MeshTriangle> triangles;

        int numWeights;
        QVector<MD5MeshWeight> weights;

        bool vbosInited = false;
        QOpenGLBuffer arrayBuf;
        QOpenGLBuffer indexBuf;
    };

    MD5Mesh();
    ~MD5Mesh();

    void addJoint(QString name, int parentIndice, QVector3D position, QQuaternion orientation);
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

    void setShaderInMesh(int mesh_indice, QString name);
    QString getShaderInMesh();

    bool isCorrupted();

    void initDrawing();
    void draw(QOpenGLShaderProgram *program);

    QString toString(bool more = false);
private:
    int m_numJoints;
    int m_numMeshes;

    bool m_computed = false;

    QVector<MD5MeshMesh> m_meshes;
    QVector<MD5MeshJoint> m_joints;

    void computeVerticesPositions();
};

#endif // MD5MESH_H
