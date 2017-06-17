#include "md5mesh.h"

struct MD5VertexTexture
{
    QVector3D position;
    QVector2D st;
};

struct MD5VertexColor
{
    QVector3D position;
    QVector3D color;
};


MD5Mesh::MD5Mesh():
    m_arrayPointBuf(QOpenGLBuffer::VertexBuffer),
    m_indexPointBuf(QOpenGLBuffer::IndexBuffer),
    m_arrayLineBuf(QOpenGLBuffer::VertexBuffer),
    m_indexLineBuf(QOpenGLBuffer::IndexBuffer)
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

        if(m.shader!=NULL)
        {
            m.shader->destroy();
        }
    }

    if(m_arrayPointBuf.isCreated())
        m_arrayPointBuf.destroy();

    if(m_indexPointBuf.isCreated())
        m_indexPointBuf.destroy();

    if(m_arrayLineBuf.isCreated())
        m_arrayLineBuf.destroy();

    if(m_indexLineBuf.isCreated())
        m_indexLineBuf.destroy();
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

void MD5Mesh::setShaderInMesh(int mesh_indice, QOpenGLTexture *texture)
{
    m_meshes[mesh_indice].shader = texture;
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

    //Meshes drawing
    for(int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];
        m->arrayBuf = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m->arrayBuf.create();
        m->indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        m->indexBuf.create();
        m->vbosInited = true;

        QVector<MD5VertexTexture> draw_vertices;
        for(int vert_ind=0;vert_ind<m->numVerts;++vert_ind)
        {
            MD5VertexTexture vd;
            vd.position = m->vertices[vert_ind].computedPosition;
            vd.st = m->vertices[vert_ind].st;
            draw_vertices.append(vd);
        }

        m->arrayBuf.bind();
        m->arrayBuf.allocate(&draw_vertices[0], draw_vertices.size() * sizeof(MD5VertexTexture));
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

    //Skeleton drawing
    m_arrayPointBuf.create();
    m_indexPointBuf.create();

    m_arrayLineBuf.create();
    m_indexLineBuf.create();

    QVector<MD5VertexColor> points;
    QVector<MD5VertexColor> lines;
    QVector<GLuint> index_points;
    QVector<GLuint> index_lines;

    for (int i = 0; i < m_numJoints; ++i)
    {
        MD5VertexColor v;
        v.position = (m_joints[i].position);
        v.color = QVector3D(1.0f,0.0f,0.0f);

        points.append(v);
        index_points.append((GLuint)i);
    }

    int j =0;
    for (int i = 0; i < m_numJoints; ++i)
    {
        if (m_joints[i].parentIndice != -1)
        {
            MD5VertexColor v1,v2;
            v1.position = (m_joints[m_joints[i].parentIndice].position);
            v1.color = QVector3D(0.0f,1.0f,0.0f);
            v2.position = (m_joints[i].position);
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

void MD5Mesh::draw(QOpenGLShaderProgram *program)
{
    program->bind();
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
        program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexTexture));

        // Offset for texture coordinate
        offset += sizeof(QVector3D);

        // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
        int textureLocation = program->attributeLocation("vST");
        program->enableAttributeArray(textureLocation);
        program->setAttributeBuffer(textureLocation, GL_FLOAT, offset, 2, sizeof(MD5VertexTexture));

        // Draw cube geometry using indices from VBO 1
        m->shader->bind();
        glDrawElements(GL_TRIANGLES, m->numTris*3, GL_UNSIGNED_INT, 0);

        m->arrayBuf.release();
        m->indexBuf.release();
    }
    program->release();
}

void MD5Mesh::drawSkeleton(QOpenGLShaderProgram *programLines, QOpenGLShaderProgram *programPoints)
{
    //Points
    programPoints->bind();
    m_arrayPointBuf.bind();
    m_indexPointBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = programLines->attributeLocation("position");
    programPoints->enableAttributeArray(vertexLocation);
    programPoints->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = programLines->attributeLocation("color");
    programPoints->enableAttributeArray(colorLocation);
    programPoints->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_POINTS, m_numJoints, GL_UNSIGNED_INT, 0);

    m_arrayPointBuf.release();
    m_arrayPointBuf.release();
    programPoints->release();

    //Lines
    programLines->bind();
    m_arrayLineBuf.bind();
    m_indexLineBuf.bind();

    // Offset for position
    offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    vertexLocation = programPoints->attributeLocation("position");
    programLines->enableAttributeArray(vertexLocation);
    programLines->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    colorLocation = programPoints->attributeLocation("color");
    programLines->enableAttributeArray(colorLocation);
    programLines->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_LINES, (m_numJoints-1)*2 , GL_UNSIGNED_INT, 0);

    m_arrayLineBuf.release();
    m_indexLineBuf.release();
    programLines->release();
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
