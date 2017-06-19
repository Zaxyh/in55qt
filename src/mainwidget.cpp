/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"

#include <QMouseEvent>

#include <math.h>

#include "QDir"
MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    lightIndicator(0),
    m_camera(),
    m_model(),
    m_meshLoaded(false),
    m_animLoaded(false),
    m_showNormal(false),
    m_showSkeleton(false),
    m_isPlayingAnim(false),
    m_loopAnim(false),
    m_duration(0.0f),
    m_lastTime(QDateTime::currentMSecsSinceEpoch()),
    m_boolFaceCulling(false),
    m_angleLight(0.0f)
{
    m_z=m_s=m_q=m_d=m_a=m_e=false;

    this->resize(800,600);

    //Mesh loader
    m_loadMesh = new QPushButton("Charger un modèle MD5",this);
    m_loadMesh->setGeometry(10,10,200,20);
    connect(m_loadMesh, SIGNAL(clicked(bool)),this, SLOT(loadMesh(bool)));

    //Normal Check Box
    m_checkNormal = new QCheckBox("Afficher les normals",this);
    m_checkNormal->setStyleSheet("QCheckBox { color: white }"
                                   "QCheckBox:disabled { color: black }");
    m_checkNormal->setGeometry(10,40,200,20);
    m_checkNormal->setEnabled(false);
    m_checkNormal->setCheckState(m_showNormal?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    connect(m_checkNormal, SIGNAL (stateChanged(int)),this, SLOT (showNormalModified(int)));

    //Skeleton Check Box
    m_checkSkeleton = new QCheckBox("Afficher le squelette",this);
    m_checkSkeleton->setStyleSheet("QCheckBox { color: white }"
                                   "QCheckBox:disabled { color: black }");
    m_checkSkeleton->setGeometry(10,70,200,20);
    m_checkSkeleton->setEnabled(false);
    m_checkSkeleton->setCheckState(m_showSkeleton?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    connect(m_checkSkeleton, SIGNAL (stateChanged(int)),this, SLOT (showSkeletonModfied(int)));

    //Anim loader
    m_loadAnim = new QPushButton("Charger une animation MD5",this);
    m_loadAnim->setGeometry(10,100,200,20);
    m_loadAnim->setEnabled(false);
    connect(m_loadAnim, SIGNAL(clicked(bool)),this, SLOT(loadAnim(bool)));

    //Play button
    m_playAnim = new QPushButton(this);
    m_playAnim->setGeometry(10,130,48,48);
    QIcon ico_play;
    ico_play.addPixmap(QPixmap(":/play_button.png"),QIcon::Normal);
    ico_play.addPixmap(QPixmap(":/play_button_off.png"),QIcon::Disabled);
    m_playAnim->setIcon(ico_play);
    m_playAnim->setIconSize(QSize(48,48));
    m_playAnim->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    m_playAnim->setEnabled(false);
    connect(m_playAnim, SIGNAL(clicked(bool)),this, SLOT(playAnim(bool)));

    //Pause button
    m_pauseAnim = new QPushButton(this);
    m_pauseAnim->setGeometry(68,130,48,48);
    QIcon ico_pause;
    ico_pause.addPixmap(QPixmap(":/pause_button.png"),QIcon::Normal);
    ico_pause.addPixmap(QPixmap(":/pause_button_off.png"),QIcon::Disabled);
    m_pauseAnim->setIcon(ico_pause);
    m_pauseAnim->setIconSize(QSize(48,48));
    m_pauseAnim->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    m_pauseAnim->setEnabled(false);
    connect(m_pauseAnim, SIGNAL(clicked(bool)),this, SLOT(pauseAnim(bool)));

    //Loop anim check box
    m_checkLoopAnim = new QCheckBox("Loop animation",this);
    m_checkLoopAnim->setStyleSheet("QCheckBox { color: white }"
                                   "QCheckBox:disabled { color: black }");
    m_checkLoopAnim->setGeometry(10,188,200,20);
    m_checkLoopAnim->setEnabled(false);
    m_checkLoopAnim->setCheckState(m_loopAnim?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    connect(m_checkLoopAnim, SIGNAL (stateChanged(int)),this, SLOT (loopAnim(int)));
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete geometries;
    if(m_meshLoaded)
    {
        delete mesh;
    }

    if(m_animLoaded)
    {
        delete anim;
    }
    doneCurrent();

    delete m_loadMesh;
    delete m_checkNormal;
    delete m_checkSkeleton;
    delete m_loadAnim;

    delete m_playAnim;
    delete m_pauseAnim;
    delete m_checkLoopAnim;
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *)
{
    m_camera.initMouseRotation();
}

void MainWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_camera.endMouseRotation();
}

void MainWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key::Key_Z)
    {
        m_z = true;
        m_s = false;
    }
    else if (e->key() == Qt::Key::Key_S)
    {
        m_s = true;
        m_z = false;
    }
    else if (e->key() == Qt::Key::Key_Q)
    {
        m_q = true;
        m_d = false;
    }
    else if (e->key() == Qt::Key::Key_D)
    {
        m_d = true;
        m_q = false;
    }
    else if (e->key() == Qt::Key::Key_A)
    {
        m_a = true;
        m_e = false;
    }
    else if (e->key() == Qt::Key::Key_E)
    {
        m_e = true;
        m_a = false;
    }

    updateMovementDir();
}

void MainWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key::Key_Z)
    {
        m_z = false;
    }
    else if (e->key() == Qt::Key::Key_S)
    {
        m_s = false;
    }
    else if (e->key() == Qt::Key::Key_Q)
    {
        m_q = false;
    }
    else if (e->key() == Qt::Key::Key_D)
    {
        m_d = false;
    }
    else if (e->key() == Qt::Key::Key_A)
    {
        m_a = false;
    }
    else if (e->key() == Qt::Key::Key_E)
    {
        m_e = false;
    }

    updateMovementDir();
}

void MainWidget::updateMovementDir()
{
    m_movementDir = QVector3D(0,0,0);
    if (m_z)
    {
        m_movementDir.setZ(1);
    }
    else if (m_s)
    {
        m_movementDir.setZ(-1);
    }

    if (m_q)
    {
        m_movementDir.setX(-1);
    }
    else if (m_d)
    {
        m_movementDir.setX(1);
    }

    if (m_a)
    {
        m_movementDir.setY(1);
    }
    else if (m_e)
    {
        m_movementDir.setY(-1);
    }
}

//! [0]

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();
    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

    glClearColor(0.2, 0.2, 0.2, 1);

    initShaders();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);

    initLights();

    geometries = new GeometryEngine;
    lightIndicator = new LightIndicator;
}

void MainWidget::initLights()
{
    md5MeshProgram.bind();
    md5MeshProgram.setUniformValue("lightColor",QVector4D(1.0,1.0,1.0,1.0));
    md5MeshProgram.setUniformValue("materialDiffuse",QVector4D(0.55,0.55,0.55,1.0));
    md5MeshProgram.setUniformValue("materialSpecular",QVector4D(0.2,0.2,0.2,1.0));
    md5MeshProgram.setUniformValue("materialShininess",5.0f);

    md5MeshProgram.setUniformValue("ambient",QVector4D(0.3,0.3,0.3,1.0));
    md5MeshProgram.release();
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!defaultProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!defaultProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!defaultProgram.link())
        close();

    // Compile vertex shader
    if (!pointProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/pointvshader.glsl"))
        close();

    // Compile fragment shader
    if (!pointProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link pointProgram pipeline
    if (!pointProgram.link())
        close();

    pointProgram.bind();
    pointProgram.setUniformValue("point_size",5.0f);
    pointProgram.release();

    // Compile vertex shader
    if (!md5MeshProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/meshvshader.glsl"))
        close();

    // Compile fragment shader
    if (!md5MeshProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/meshfshader.glsl"))
        close();

    // Link shader pipeline
    if (!md5MeshProgram.link())
        close();

    md5MeshProgram.bind();
    md5MeshProgram.setUniformValue("texture",0);
}
//! [3]


//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 0.1, far plane to 1000.0, field of view 60 degrees
    const qreal zNear = 0.1, zFar = 1000.0, fov = 60.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);

    defaultProgram.bind();
    defaultProgram.setUniformValue("p",projection);
    defaultProgram.release();

    pointProgram.bind();
    pointProgram.setUniformValue("p",projection);
    pointProgram.release();

    md5MeshProgram.bind();
    md5MeshProgram.setUniformValue("p",projection);
    md5MeshProgram.release();
}
//! [5]

void MainWidget::paintGL()
{
    qint64 current_time = QDateTime::currentMSecsSinceEpoch();
    double delta_time = ((double)(current_time-m_lastTime)) / 1000.0;
    m_lastTime = current_time;

    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate view transformation
    m_camera.update(m_movementDir);
    QMatrix4x4 view = m_camera.getView();

    defaultProgram.bind();
    defaultProgram.setUniformValue("v",view);
    defaultProgram.release();

    pointProgram.bind();
    pointProgram.setUniformValue("v",view);
    pointProgram.release();

    md5MeshProgram.bind();
    md5MeshProgram.setUniformValue("v",view);
    md5MeshProgram.setUniformValue("eyePosWorld",QVector4D(m_camera.getPosition(),1.0));
    md5MeshProgram.release();

    //Calculate model transformation
    m_model.setToIdentity();
    m_model.translate(0.0, 0.0, -5.0);

    // Set modelview-projection matrix
    defaultProgram.bind();
    defaultProgram.setUniformValue("m", m_model);
    defaultProgram.release();

    // Draw cube geometry
    geometries->drawGeometry(&defaultProgram);

    m_angleLight += 30 * delta_time;
    m_angleLight = fmod(m_angleLight,360.0f);
    QMatrix4x4 light_model = m_model;

    if(m_meshLoaded)
    {
        m_model.rotate(-90.0f,QVector3D(1.0f,0.0f,0.0f));
        m_model.scale(0.3f);

        if(m_animLoaded)
        {
            if(m_isPlayingAnim)
            {
                m_duration += delta_time;

                MD5Skeleton *current_skeleton = new MD5Skeleton();
                if (anim->getSkeletonFromDuration(current_skeleton,m_duration,m_loopAnim))
                {
                    m_duration = 0.0;
                    pauseAnim();
                }

                mesh->setSkeleton(current_skeleton);
                mesh->prepareDrawing();
            }
        }


        //Compute Light position
        QVector4D light_pos(cos(qDegreesToRadians(m_angleLight))*2.0f,1.0,sin(qDegreesToRadians(m_angleLight))*2.0f,1.0);

        light_model.translate(light_pos.toVector3D());
        md5MeshProgram.bind();
        md5MeshProgram.setUniformValue("lightPosWorld", light_model * light_pos );
        md5MeshProgram.release();
        light_model.scale(0.1,0.1,0.1);

        //Draw Light
        defaultProgram.bind();
        defaultProgram.setUniformValue("m", light_model);
        defaultProgram.release();
        lightIndicator->draw(&defaultProgram);

        //Draw Mesh
        md5MeshProgram.bind();
        md5MeshProgram.setUniformValue("m", m_model);
        md5MeshProgram.release();
        mesh->draw(&md5MeshProgram);

        if(m_showNormal)
        {
            defaultProgram.bind();
            defaultProgram.setUniformValue("m", m_model);
            defaultProgram.release();
            mesh->drawNormal(&defaultProgram);
        }

        if(m_showSkeleton)
        {
            glDisable(GL_DEPTH_TEST);
            defaultProgram.bind();
            defaultProgram.setUniformValue("m", m_model);
            defaultProgram.release();
            pointProgram.bind();
            pointProgram.setUniformValue("m", m_model);
            pointProgram.release();
            mesh->drawSkeleton(&defaultProgram, &pointProgram);
            glEnable(GL_DEPTH_TEST);
        }
    }
}

void MainWidget::disableMesh()
{
    if(m_meshLoaded)
    {
        delete mesh;
    }

    m_meshLoaded = false;
    m_loadAnim->setEnabled(false);
    m_checkNormal->setEnabled(false);
    m_checkSkeleton->setEnabled(false);

    disableAnim();
}

void MainWidget::enableMesh()
{
    m_meshLoaded = true;
    m_checkNormal->setEnabled(true);
    m_checkSkeleton->setEnabled(true);
    m_loadAnim->setEnabled(true);
}

void MainWidget::disableAnim()
{
    if(m_animLoaded)
    {
        delete anim;
    }

    m_animLoaded = false;

    m_isPlayingAnim = false;
    m_duration = 0.0;
    m_checkLoopAnim->setEnabled(false);
    m_playAnim->setEnabled(false);
    m_pauseAnim->setEnabled(false);
}

void MainWidget::enableAnim()
{
    m_animLoaded = true;

    m_checkLoopAnim->setEnabled(true);
    playAnim();
}

void MainWidget::playAnim()
{
    m_isPlayingAnim = true;
    m_playAnim->setEnabled(false);
    m_pauseAnim->setEnabled(true);
}

void MainWidget::pauseAnim()
{
    m_isPlayingAnim = false;
    m_playAnim->setEnabled(true);
    m_pauseAnim->setEnabled(false);
}

//UI
void MainWidget::loadMesh(bool)
{
    disableMesh();

    QString meshPath = QFileDialog::getOpenFileName(this,"Charger un modèle MD5", QDir::currentPath(), "MD5 modèle (*.md5mesh)");

    if(!meshPath.isNull())
    {
        mesh = MD5Parser::ParseMeshFile(meshPath);
        mesh->prepareDrawing();

        enableMesh();
    }
}

void MainWidget::loadAnim(bool)
{
    disableAnim();
    mesh->setDefaultSkeleton();
    mesh->prepareDrawing();

    QString animPath = QFileDialog::getOpenFileName(this,"Charger une animation MD5", QDir::currentPath(), "MD5 anim (*.md5anim)");

    if(!animPath.isNull())
    {
        anim = MD5Parser::ParseAnimFile(animPath);

        enableAnim();
    }
}

void MainWidget::showNormalModified(int state)
{
    m_showNormal = (Qt::CheckState::Checked==state)?true:false;
}

void MainWidget::showSkeletonModfied(int state)
{
    m_showSkeleton = (Qt::CheckState::Checked==state)?true:false;
}

void MainWidget::playAnim(bool)
{
    playAnim();
}

void MainWidget::pauseAnim(bool)
{
    pauseAnim();
}

void MainWidget::loopAnim(int state)
{
    m_loopAnim = (Qt::CheckState::Checked==state)?true:false;
    playAnim();
}
