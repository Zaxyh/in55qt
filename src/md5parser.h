#ifndef MD5PARSER_H
#define MD5PARSER_H

#include "md5anim.h"
#include "md5mesh.h"

#include "QFile"
#include "QQuaternion"
#include "QString"

class MD5Parser
{
public:
    static MD5Mesh ParseMeshFile(QString path);
private:
    MD5Parser();
    static bool SplitNextLine(QTextStream& in, QStringList& list);
    static QQuaternion MakeUnitQuaternion(float x, float y, float z);
};

#endif // MD5PARSER_H
