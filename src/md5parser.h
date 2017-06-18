#ifndef MD5PARSER_H
#define MD5PARSER_H

#include "md5anim.h"
#include "md5mesh.h"

#include <QDir>
#include <QFile>
#include <QQuaternion>
#include <QString>
#include <QImageReader>

class MD5Parser
{
public:
    static MD5Mesh* ParseMeshFile(QString path);
    static MD5Anim* ParseAnimFile(QString path);

    static QQuaternion MakeUnitQuaternion(float x, float y, float z);
private:
    MD5Parser();
    static bool SplitNextLine(QTextStream& in, QStringList& list);
};

#endif // MD5PARSER_H
