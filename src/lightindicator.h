#ifndef LIGHTINDICATOR_H
#define LIGHTINDICATOR_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

struct VertexDataIndicator;

class LightIndicator : protected QOpenGLFunctions
{
public:
    LightIndicator();
    virtual ~LightIndicator();

    void draw(QOpenGLShaderProgram *program);

private:
    void initGeometry();

    static VertexDataIndicator vertices[];
    static GLushort indices[];

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};

#endif // LIGHTINDICATOR_H
