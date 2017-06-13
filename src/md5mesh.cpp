#include "md5mesh.h"

struct MD5VertexDrawing
{
    QVector3D position;
    QVector3D color;
};

MD5Mesh::MD5Mesh()
{

}

MD5Mesh::~MD5Mesh()
{
    foreach(MD5MeshMesh m, m_meshes)
    {
        if(m.vbosInited)
        {
            m.arrayBuf.destroy();
            m.indexBuf.destroy();
        }
    }
}

bool MD5Mesh::isCorrupted()
{
    if (m_numJoints != m_joints.size() || m_numMeshes != m_meshes.size())
    {
        return true;
    }

    for(QVectorIterator<MD5MeshMesh> it_mesh(m_meshes);it_mesh.hasNext();)
    {
        MD5MeshMesh m = it_mesh.next();
        if(m.numVerts!=m.vertices.size() ||
                m.numTris!=m.triangles.size() ||
                m.numWeights!=m.weights.size())
            return true;
    }

    return false;
}

void MD5Mesh::addJoint(QString name, int parentIndice, QVector3D position, QQuaternion orientation)
{
    MD5MeshJoint joint;

    joint.name = name;
    joint.parentIndice = parentIndice;
    joint.position = position;
    joint.orientation = orientation;

    m_joints.append(joint);
}

int MD5Mesh::addMesh()
{
    m_meshes.append(MD5MeshMesh());
    return m_meshes.size()-1;
}

void MD5Mesh::addVertexInMesh(int mesh_indice, QVector2D st, int startWeight, int countWeight)
{
    MD5MeshVertex vertex;

    vertex.st = st;
    vertex.startWeight = startWeight;
    vertex.countWeight = countWeight;

    m_meshes[mesh_indice].vertices.append(vertex);
}

void MD5Mesh::addTriangleInMesh(int mesh_indice, int vertex0, int vertex1, int vertex2)
{
    MD5MeshTriangle triangle;

    triangle.vertex0 = vertex0;
    triangle.vertex1 = vertex1;
    triangle.vertex2 = vertex2;

    m_meshes[mesh_indice].triangles.append(triangle);
}

void MD5Mesh::addWeightInMesh(int mesh_indice, int jointIndice, float bias, QVector3D position)
{
    MD5MeshWeight weight;

    weight.jointIndice = jointIndice;
    weight.bias = bias;
    weight.position = position;

    m_meshes[mesh_indice].weights.append(weight);
}

void MD5Mesh::setNumJoints(int n)
{
    m_numJoints = n;
}

int MD5Mesh::getNumJoints()
{
    return m_numJoints;
}

void MD5Mesh::setNumMeshes(int n)
{
    m_numMeshes = n;
}

void MD5Mesh::setNumVerticesInMesh(int mesh_indice, int n)
{
    m_meshes[mesh_indice].numVerts = n;
}

void MD5Mesh::setNumTrianglesInMesh(int mesh_indice, int n)
{
    m_meshes[mesh_indice].numTris = n;
}

void MD5Mesh::setNumWeightsInMesh(int mesh_indice, int n)
{
    m_meshes[mesh_indice].numWeights = n;
}

void MD5Mesh::setShaderInMesh(int mesh_indice, QString name)
{
    m_meshes[mesh_indice].shader = name;
}

void MD5Mesh::computeVerticesPositions()
{
    for(int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];
        for(int vert_ind=0;vert_ind<m->numVerts;++vert_ind)
        {
            MD5MeshVertex *v = &m->vertices[vert_ind];
            v->computedPosition = QVector3D(0.0f, 0.0f, 0.0f);

            float total_bias = 0.0f;
            for(int wei_ind=0;wei_ind<v->countWeight;++wei_ind)
            {
                MD5MeshWeight *w = &m->weights[v->startWeight+wei_ind];
                MD5MeshJoint *j = &m_joints[w->jointIndice];
                QVector3D weight_rotated_position = j->orientation.rotatedVector(w->position);

                v->computedPosition += (j->position + weight_rotated_position) * w->bias;
                total_bias+=w->bias;
            }
            qDebug() << "Computed["<<mesh_ind<<"|"<<vert_ind<<+"] : "<<v->computedPosition << " bias : " << total_bias;
        }
    }

    m_computed = true;
}

void MD5Mesh::initDrawing()
{
    if(isCorrupted())
    {
        return;
    }

    initializeOpenGLFunctions();
    computeVerticesPositions();

    for(int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];
        m->arrayBuf = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m->arrayBuf.create();
        m->indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        m->indexBuf.create();
        m->vbosInited = true;

        QVector<MD5VertexDrawing> draw_vertices;
        for(int vert_ind=0;vert_ind<m->numVerts;++vert_ind)
        {
            MD5VertexDrawing vd;
            vd.position = m->vertices[vert_ind].computedPosition;
            vd.color = QVector3D(0.65f,0.65f,0.65f);
            draw_vertices.append(vd);
        }

        m->arrayBuf.bind();
        m->arrayBuf.allocate(&draw_vertices[0], draw_vertices.size() * sizeof(MD5VertexDrawing));
        m->arrayBuf.release();

        QVector<GLuint> draw_indices;
        for(int tris_ind=0;tris_ind<m->numTris;++tris_ind)
        {
            draw_indices.append(m->triangles[tris_ind].vertex0);
            draw_indices.append(m->triangles[tris_ind].vertex1);
            draw_indices.append(m->triangles[tris_ind].vertex2);
        }

        m->indexBuf.bind();
        m->indexBuf.allocate(&draw_indices[0], draw_indices.size() * sizeof(GLuint));
        m->indexBuf.release();
    }
}

void MD5Mesh::draw(QOpenGLShaderProgram *program)
{
    for(int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];
        m->arrayBuf.bind();
        m->indexBuf.bind();

        // Offset for position
        quintptr offset = 0;

        // Tell OpenGL programmable pipeline how to locate vertex position data
        int vertexLocation = program->attributeLocation("position");
        program->enableAttributeArray(vertexLocation);
        program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexDrawing));

        // Offset for texture coordinate
        offset += sizeof(QVector3D);

        // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
        int colorLocation = program->attributeLocation("color");
        program->enableAttributeArray(colorLocation);
        program->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexDrawing));

        // Draw cube geometry using indices from VBO 1
        glDrawElements(GL_TRIANGLES, m->numTris*3, GL_UNSIGNED_SHORT, 0);

        m->arrayBuf.release();
        m->indexBuf.release();
    }
}

QString MD5Mesh::toString(bool more)
{
    QString output = "";

    output += "numJoints : " + QString::number(m_numJoints) + "/" + QString::number(m_joints.size());
    output += "\nnumMeshes: " + QString::number(m_numMeshes) + "/" + QString::number(m_meshes.size());

    int i=0;
    foreach(const MD5MeshMesh &m, m_meshes)
    {
        output += "\nmesh["+QString::number(i++)+"]";
        output += "\n\tnumVerts : " + QString::number(m.numVerts) + "/" + QString::number(m.vertices.size());
        output += "\n\tnumTris : " + QString::number(m.numTris) + "/" + QString::number(m.triangles.size());
        output += "\n\tnumWeights : " + QString::number(m.numWeights) + "/" + QString::number(m.weights.size());
    }

    if(more)
    {
        foreach(const MD5MeshJoint &j, m_joints)
        {
            QString str_position,str_orientation;
            &str_position<<j.position;
            &str_orientation<<j.orientation;
            output +="\n"+j.name+" "+QString::number(j.parentIndice)+" "+str_position+" "+str_orientation;
        }

        i=0;
        foreach(const MD5MeshMesh &m, m_meshes)
        {
            int j=0;
            foreach(const MD5MeshVertex &v, m.vertices)
            {
                QString str_position;
                &str_position<<v.computedPosition;
                output += "\n"+QString::number(i)+"|"+QString::number(j++)+" : "+str_position;
            }
            ++i;
        }
    }

    output += "\nisCorrupted : " + QString(isCorrupted()?"Yes":"No");

    return output;
}
