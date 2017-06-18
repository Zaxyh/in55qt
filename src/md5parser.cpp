#include "md5parser.h"

MD5Parser::MD5Parser(){}

bool MD5Parser::SplitNextLine(QTextStream& in, QStringList& list)
{
    if (in.atEnd())
    {
       return false;
    }

    list = in.readLine().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (list.empty())
    {
        return SplitNextLine(in,list);
    }

    return true;
}

QQuaternion MD5Parser::MakeUnitQuaternion(float x, float y, float z)
{
    float w = 1-x*x-y*y-z*z;
    if (w<0)
    {
        w = 0;
    }
    else
    {
        w = -sqrt(w);
    }
    return (QQuaternion(w,x,y,z));
}

//MD5Mesh parser
MD5Mesh* MD5Parser::ParseMeshFile(QString path)
{
    MD5Mesh *mesh = new MD5Mesh();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return mesh;
    }

    QDir dir = QFileInfo(path).dir();

    QTextStream in(&file);

    QStringList list;
    while (SplitNextLine(in,list)) {
        if (list[0] == "numJoints")
        {
            bool ok;
            mesh->setNumJoints(list[1].toInt(&ok));
            if (!ok)
            {
                qDebug()<<"Line error";
            }
        }
        else if (list[0] == "numMeshes")
        {
            bool ok;
            mesh->setNumMeshes(list[1].toInt(&ok));
            if (!ok)
            {
                qDebug()<<"Line error";
            }
        }
        //Joints
        else if (list[0] == "joints")
        {
            //Pass current line
            if (!SplitNextLine(in,list)){return mesh;}

            //Parse every joint
            bool ok_parent,ok_x,ok_y,ok_z,ok_qx,ok_qy,ok_qz;
            QString name;
            int parent_indice;
            QVector3D position;
            QQuaternion orientation;

            while (list[0] != "}")
            {
                name = list[0].remove('"');
                parent_indice = list[1].toInt(&ok_parent);
                position = QVector3D(list[3].toFloat(&ok_x),
                        list[4].toFloat(&ok_y),
                        list[5].toFloat(&ok_z));
                orientation = MakeUnitQuaternion(list[8].toFloat(&ok_qx),
                        list[9].toFloat(&ok_qy),
                        list[10].toFloat(&ok_qz));
                if (ok_parent && ok_x && ok_y && ok_z && ok_qx && ok_qy && ok_qz)
                {
                    mesh->addJoint(name,parent_indice,position,orientation);
                }
                else
                {
                    qDebug()<<"Line error";
                }
                if (!SplitNextLine(in,list)){return mesh;}
            }
        }
        //Meshes
        else if (list[0] == "mesh")
        {
            //Pass current line
            if (!SplitNextLine(in,list)){return mesh;}

            int current_mesh_indice = mesh->addMesh();
            //Parse every joint
            while (list[0] != "}")
            {
                if (list[0] == "shader")
                {
                    list[1].remove('"');
                    QString textPath = dir.filePath(list[1]+".png");

                    QImageReader imageReader(textPath);
                    QImage image = imageReader.read();

                    QOpenGLTexture *texture = new QOpenGLTexture(image);
                    mesh->setShaderInMesh(current_mesh_indice,texture);
                }
                else if (list[0] == "numverts")
                {
                    bool ok_num;
                    int num_verts = list[1].toInt(&ok_num);
                    if(ok_num)
                    {
                        mesh->setNumVerticesInMesh(current_mesh_indice,num_verts);
                    }
                    else
                    {
                        qDebug()<<"Line error";
                    }
                }
                else if (list[0] == "vert")
                {
                    bool ok_stx, ok_sty, ok_start, ok_count;
                    QVector2D st = QVector2D(list[3].toFloat(&ok_stx),list[4].toFloat(&ok_sty));
                    int start_weight = list[6].toInt(&ok_start);
                    int count_weight = list[7].toInt(&ok_count);

                    if (ok_stx && ok_sty && ok_start && ok_count)
                    {
                        mesh->addVertexInMesh(current_mesh_indice,st,start_weight,count_weight);
                    }
                    else
                    {
                        qDebug()<<"Line error";
                    }
                }
                else if (list[0] == "numtris")
                {
                    bool ok_num;
                    int num_tris = list[1].toInt(&ok_num);
                    if(ok_num)
                    {
                        mesh->setNumTrianglesInMesh(current_mesh_indice,num_tris);
                    }
                    else
                    {
                        qDebug()<<"Line error";
                    }
                }
                else if (list[0] == "tri")
                {
                    bool ok_vert0, ok_vert1, ok_vert2;
                    int vert0 = list[2].toUInt(&ok_vert0);
                    int vert1 = list[3].toUInt(&ok_vert1);
                    int vert2 = list[4].toUInt(&ok_vert2);

                    if (ok_vert0 && ok_vert1 && ok_vert2)
                    {
                        mesh->addTriangleInMesh(current_mesh_indice,vert0,vert1,vert2);
                    }
                    else
                    {
                        qDebug()<<"Line error";
                    }
                }
                else if (list[0] == "numweights")
                {
                    bool ok_num;
                    int num_weights = list[1].toInt(&ok_num);
                    if(ok_num)
                    {
                        mesh->setNumWeightsInMesh(current_mesh_indice,num_weights);
                    }
                    else
                    {
                        qDebug()<<"Line error";
                    }
                }
                else if (list[0] == "weight")
                {
                    bool ok_ind_joint, ok_bias, ok_pos_x, ok_pos_y, ok_pos_z;
                    int ind_joint = list[2].toInt(&ok_ind_joint);
                    float bias = list[3].toFloat(&ok_bias);
                    QVector3D position = QVector3D(list[5].toFloat(&ok_pos_x),
                            list[6].toFloat(&ok_pos_y),
                            list[7].toFloat(&ok_pos_z));

                    if (ok_ind_joint && ok_bias && ok_pos_x && ok_pos_y && ok_pos_z)
                    {
                        mesh->addWeightInMesh(current_mesh_indice,ind_joint,bias,position);
                    }
                    else
                    {
                        qDebug()<<"Line error";
                    }
                }

                SplitNextLine(in,list);
            }
        }
    }

    //Prepare default position
    if(!mesh->isCorrupted())
    {
        mesh->setDefaultSkeleton();
    }

    return mesh;
}

//MD5Anim parser
MD5Anim* MD5Parser::ParseAnimFile(QString path)
{
    MD5Anim *anim = new MD5Anim();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return anim;
    }

    QDir dir = QFileInfo(path).dir();

    QTextStream in(&file);

    QStringList list;
    while (SplitNextLine(in,list)) {
        if (list[0] == "numFrames")
        {
            bool ok;
            anim->setNumFrames(list[1].toInt(&ok));
            if (!ok)
            {
                qDebug()<<"Line error";
            }
        }
        else if (list[0] == "numJoints")
        {
            bool ok;
            anim->setNumJoints(list[1].toInt(&ok));
            if (!ok)
            {
                qDebug()<<"Line error";
            }
        }
        else if (list[0] == "frameRate")
        {
            bool ok;
            anim->setFrameRate(list[1].toInt(&ok));
            if (!ok)
            {
                qDebug()<<"Line error";
            }
        }
        else if (list[0] == "numAnimatedComponents")
        {
            bool ok;
            anim->setNumAnimatedComponents(list[1].toInt(&ok));
            if (!ok)
            {
                qDebug()<<"Line error";
            }
        }
        else if (list[0] == "hierarchy")
        {
            //Pass current line
            if (!SplitNextLine(in,list)){return anim;}

            //Parse every joint
            bool ok_parent,ok_flags,ok_startIndex;
            QString name;
            int parent_indice;
            int flags;
            int startIndex;

            while (list[0] != "}")
            {
                name = list[0].remove('"');
                parent_indice = list[1].toInt(&ok_parent);
                flags = list[2].toInt(&ok_flags);
                startIndex = list[3].toInt(&ok_startIndex);

                if (ok_parent && ok_flags && ok_startIndex)
                {
                    anim->addHierarchy(name,parent_indice,flags,startIndex);
                }
                else
                {
                    qDebug()<<"Line error";
                }
                if (!SplitNextLine(in,list)){return anim;}
            }
        }
        else if (list[0] == "bounds")
        {
            //Pass current line
            if (!SplitNextLine(in,list)){return anim;}

            //Parse every joint
            bool ok_x1,ok_y1,ok_z1,ok_x2,ok_y2,ok_z2;
            QVector3D min,max;

            while (list[0] != "}")
            {
                min = QVector3D(list[1].toFloat(&ok_x1),
                        list[2].toFloat(&ok_y1),
                        list[3].toFloat(&ok_z1));

                max = QVector3D(list[6].toFloat(&ok_x2),
                        list[7].toFloat(&ok_y2),
                        list[8].toFloat(&ok_z2));

                if (ok_x1 && ok_y1 && ok_z1 && ok_x2 && ok_y2 && ok_z2)
                {
                    anim->addBounds(min,max);
                }
                else
                {
                    qDebug()<<"Line error";
                }
                if (!SplitNextLine(in,list)){return anim;}

            }
        }
        else if (list[0] == "baseframe")
        {
            //Pass current line
            if (!SplitNextLine(in,list)){return anim;}

            //Parse every joint
            bool ok_x,ok_y,ok_z,ok_qx,ok_qy,ok_qz;
            QVector3D position;
            QQuaternion orientation;

            while (list[0] != "}")
            {
                position = QVector3D(list[1].toFloat(&ok_x),
                        list[2].toFloat(&ok_y),
                        list[3].toFloat(&ok_z));

                orientation = MakeUnitQuaternion(list[6].toFloat(&ok_qx),
                        list[7].toFloat(&ok_qy),
                        list[8].toFloat(&ok_qz));

                if (ok_x && ok_y && ok_z && ok_qx && ok_qy && ok_qz)
                {
                    anim->addBaseFrame(position, orientation);
                }
                else
                {
                    qDebug()<<"Line error";
                }
                if (!SplitNextLine(in,list)){return anim;}

            }
        }
        else if (list[0] == "frame")
        {
            bool ok_index;
            int index = list[1].toInt(&ok_index);

            //Pass current line
            if (!SplitNextLine(in,list)){return anim;}

            //Parse every joint
            bool ok_data,ok_datas=true;
            QVector<float> datas;

            while (list[0] != "}")
            {
                foreach(QString s, list)
                {
                    datas.append(s.toFloat(&ok_data));
                    if(!ok_data){ok_datas=false;}
                }

                if (!SplitNextLine(in,list)){return anim;}
            }

            if (ok_index && ok_datas)
            {
                anim->addFrame(index, datas);
            }
            else
            {
                qDebug()<<"Frame error";
            }
        }
    }

    anim->buildSkeletonsFrames();

    return anim;
}
