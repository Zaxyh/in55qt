#include "md5mesh.h"

struct MD5VertexTexture
{
    QVector3D position;
    QVector2D st;
    QVector3D normal;
};

struct MD5VertexColor
{
    QVector3D position;
    QVector3D color;
};

MD5Mesh::MD5Mesh():
    m_skeleton(NULL)
{
    initializeOpenGLFunctions();
}

MD5Mesh::~MD5Mesh()
{
    foreach(MD5MeshMesh m, m_meshes)
    {
        if(m.arrayBuf.isCreated())
            m.arrayBuf.destroy();

        if(m.indexBuf.isCreated())
            m.indexBuf.destroy();

        if(m.arrayNormalBuf.isCreated())
            m.arrayNormalBuf.destroy();

        if(m.indexNormalBuf.isCreated())
            m.indexNormalBuf.destroy();

        if(m.shader!=NULL)
            m.shader->destroy();
    }

    if(m_skeleton != NULL)
        delete m_skeleton;
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

bool MD5Mesh::checkAnimation(MD5Anim *anim)
{
    if ( m_numJoints != anim->getNumJoints() )
    {
        return false;
    }

    for ( int i = 0; i < m_joints.size(); ++i )
    {
        if ( m_joints[i].name != anim->getNameJoint(i) ||
             m_joints[i].parentIndex != anim->getParentJoint(i) )
        {
            return false;
        }
    }

    return true;
}

void MD5Mesh::addJoint(QString name, int parentIndex, QVector3D position, QQuaternion orientation)
{
    MD5MeshJoint joint;

    joint.name = name;
    joint.parentIndex = parentIndex;
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

void MD5Mesh::prepareDrawing()
{
    if(isCorrupted())
    {
        return;
    }

    for(int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];
        //Compute mesh vertices
        QVector<MD5VertexTexture> computed_vertices;

        QVector<MD5VertexColor> normal_vertice;
        QVector<GLuint> normal_index;

        for(int vert_ind=0;vert_ind<m->numVerts;++vert_ind)
        {
            MD5MeshVertex *v = &m->vertices[vert_ind];
            MD5VertexTexture computed_vertex;
            computed_vertex.position = QVector3D(0.0f, 0.0f, 0.0f);

            for(int wei_ind=0;wei_ind<v->countWeight;++wei_ind)
            {
                MD5MeshWeight *w = &m->weights[v->startWeight+wei_ind];

                QVector3D weight_rotated_position = m_skeleton->getJointOrientation(w->jointIndice).rotatedVector(w->position);

                computed_vertex.position += (m_skeleton->getJointPosition(w->jointIndice) + weight_rotated_position) * w->bias;

                computed_vertex.normal += (m_skeleton->getJointOrientation(w->jointIndice).rotatedVector(v->normal)) * w->bias;
            }

            computed_vertex.st = v->st;
            computed_vertices.append(computed_vertex);

            MD5VertexColor normal_vertex0,normal_vertex1;
            normal_vertex0.position = computed_vertex.position;
            normal_vertex0.color = QVector3D(1.0,1.0,0.0);
            normal_vertex1.position = normal_vertex0.position + computed_vertex.normal.normalized() * 0.1f;
            normal_vertex1.color = QVector3D(1.0,1.0,0.0);

            normal_vertice.append(normal_vertex0);
            normal_index.append(((GLuint) vert_ind*2));
            normal_vertice.append(normal_vertex1);
            normal_index.append(((GLuint) vert_ind*2+1));
        }

        //Init VBOs mesh
        m->arrayBuf = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m->arrayBuf.create();
        m->indexBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        m->indexBuf.create();

        m->arrayBuf.bind();
        m->arrayBuf.allocate(&computed_vertices[0], computed_vertices.size() * sizeof(MD5VertexTexture));
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

        //Init VBOs normal
        m->arrayNormalBuf = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m->arrayNormalBuf.create();
        m->indexNormalBuf = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        m->indexNormalBuf.create();

        m->arrayNormalBuf.bind();
        m->arrayNormalBuf.allocate(&normal_vertice[0], normal_vertice.size() * sizeof(MD5VertexColor));
        m->arrayNormalBuf.release();

        m->indexNormalBuf.bind();
        m->indexNormalBuf.allocate(&normal_index[0], normal_index.size() * sizeof(GLuint));
        m->indexNormalBuf.release();
    }

    m_skeleton->prepareDrawing();
}

void MD5Mesh::prepareNormals()
{
    for (int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];

        QVector<QVector3D> position_vertices;

        for(int vert_ind=0;vert_ind<m->numVerts;++vert_ind)
        {
            MD5MeshVertex *v = &m->vertices[vert_ind];
            QVector3D position_vertice = QVector3D(0.0f, 0.0f, 0.0f);

            for(int wei_ind=0;wei_ind<v->countWeight;++wei_ind)
            {
                MD5MeshWeight *w = &m->weights[v->startWeight+wei_ind];

                QVector3D weight_rotated_position = m_skeleton->getJointOrientation(w->jointIndice).rotatedVector(w->position);

                position_vertice += (m_skeleton->getJointPosition(w->jointIndice) + weight_rotated_position) * w->bias;
            }

            position_vertices.append(position_vertice);
        }

        for ( int i = 0; i < m->triangles.size(); ++i )
        {
            QVector3D v0 = position_vertices[ m->triangles[i].vertex0 ];
            QVector3D v1 = position_vertices[ m->triangles[i].vertex1 ];
            QVector3D v2 = position_vertices[ m->triangles[i].vertex2 ];

            QVector3D normal = QVector3D::crossProduct( v2 - v0, v1 - v0);

            m->vertices[ m->triangles[i].vertex0 ].normal += normal;
            m->vertices[ m->triangles[i].vertex1 ].normal += normal;
            m->vertices[ m->triangles[i].vertex2 ].normal += normal;
        }

        for ( int i = 0; i < m->vertices.size(); ++i )
        {
            MD5MeshVertex *v = &m->vertices[i];
            QVector3D normal = v->normal.normalized();

            v->normal = QVector3D(0.0f, 0.0f, 0.0f);

            //Pré calcule dans l'espace joint local
            for ( int j = 0; j < m->vertices[i].countWeight; ++j )
            {
                MD5MeshWeight *weight = &m->weights[v->startWeight+j];
                MD5MeshJoint *joint = &m_joints[weight->jointIndice];

                v->normal += ( joint->orientation.inverted().rotatedVector(normal) ) * weight->bias;
            }
        }
    }
}

void MD5Mesh::setDefaultSkeleton()
{
    if(m_skeleton != NULL)
    {
        delete m_skeleton;
    }

    m_skeleton = new MD5Skeleton();

    foreach(MD5MeshJoint j, m_joints)
    {
        m_skeleton->addJoint(j.parentIndex,j.position,j.orientation);
    }
}

void MD5Mesh::setSkeleton(MD5Skeleton *skeleton)
{
    if(m_skeleton != NULL)
    {
        delete m_skeleton;
    }

    m_skeleton = skeleton;
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

        offset += sizeof(QVector2D);

        int normalLocation = program->attributeLocation("normal");
        program->enableAttributeArray(normalLocation);
        program->setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexTexture));

        // Draw cube geometry using indices from VBO 1
        m->shader->bind();
        glDrawElements(GL_TRIANGLES, m->numTris*3, GL_UNSIGNED_INT, 0);

        m->arrayBuf.release();
        m->indexBuf.release();
    }
    program->release();
}

void MD5Mesh::drawNormal(QOpenGLShaderProgram *programLines)
{
    programLines->bind();
    for(int mesh_ind=0;mesh_ind<m_numMeshes;++mesh_ind)
    {
        MD5MeshMesh *m = &m_meshes[mesh_ind];
        m->arrayNormalBuf.bind();
        m->indexNormalBuf.bind();

        // Offset for position
        quintptr offset = 0;

        // Tell OpenGL programmable pipeline how to locate vertex position data
        int vertexLocation = programLines->attributeLocation("position");
        programLines->enableAttributeArray(vertexLocation);
        programLines->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

        // Offset for texture coordinate
        offset += sizeof(QVector3D);

        // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
        int colorLocation = programLines->attributeLocation("color");
        programLines->enableAttributeArray(colorLocation);
        programLines->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(MD5VertexColor));

        glDrawElements(GL_LINES, m->numVerts*2, GL_UNSIGNED_INT, 0);

        m->arrayBuf.release();
        m->indexBuf.release();
    }
    programLines->release();
}

void MD5Mesh::drawSkeleton(QOpenGLShaderProgram *programLines, QOpenGLShaderProgram *programPoints)
{
    if(m_skeleton != NULL)
    {
        m_skeleton->draw(programLines, programPoints);
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
            output +="\n"+j.name+" "+QString::number(j.parentIndex)+" "+str_position+" "+str_orientation;
        }
    }

    output += "\nisCorrupted : " + QString(isCorrupted()?"Yes":"No");

    return output;
}
