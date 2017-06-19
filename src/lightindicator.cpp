#include "lightindicator.h"

#include <QVector2D>
#include <QVector3D>

struct VertexDataIndicator
{
    QVector3D position;
    QVector3D color;
};


VertexDataIndicator LightIndicator::vertices[] = {
    {QVector3D(-0.5f, 0.0f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f)},
    {QVector3D(0.5f, 0.0f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f)},
    {QVector3D(0.0f, -0.5f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f)},
    {QVector3D(0.0f, 0.5f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f)},
    {QVector3D(0.0f, 0.0f, -0.5f), QVector3D(1.0f, 1.0f, 1.0f)},
    {QVector3D(0.0f, 0.0f, 0.5f), QVector3D(1.0f, 1.0f, 1.0f)},
};

const int nbrVertices = 6;

GLushort LightIndicator::indices[] = {
    0,1,
    2,3,
    4,5
};

const int nbrIndices = 6;


//! [0]
LightIndicator::LightIndicator()
    : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    initGeometry();
}

LightIndicator::~LightIndicator()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}
//! [0]

void LightIndicator::initGeometry()
{
////! [1]
    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, nbrVertices * sizeof(VertexDataIndicator));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, nbrIndices * sizeof(GLushort));
//! [1]
}

//! [2]
void LightIndicator::draw(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    program->bind();
    arrayBuf.bind();
    indexBuf.bind();


    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexDataIndicator));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int colorLocation = program->attributeLocation("color");
    program->enableAttributeArray(colorLocation);
    program->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 3, sizeof(VertexDataIndicator));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, 0);

    arrayBuf.release();
    indexBuf.release();
    program->release();
}
//! [2]
