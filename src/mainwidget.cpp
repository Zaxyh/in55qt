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
    //angularSpeed(0),
    m_camera(),
    m_model()
{
    m_z=m_s=m_q=m_d=m_a=m_e=false;
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete geometries;
    doneCurrent();
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

//! [1]
void MainWidget::timerEvent(QTimerEvent *)
{
    /*
    // Decrease angular speed (friction)
    angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    } else {
        // Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

        // Request an update
        update();
    }
    */
}
//! [1]

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();
    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

    glClearColor(0, 0, 0, 1);

    initShaders();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    geometries = new GeometryEngine;

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);

    QTextStream out(stdout);
    QString file = QDir::currentPath()+"/Animation/bob_lamp_update.md5mesh";

    mesh = MD5Parser::ParseMeshFile(file);
    out << file << "\n";
    out << mesh.toString(true) << "\n";
    mesh.initDrawing();
    out << mesh.toString(true) << "\n";
    out <<"Inited for drawing !\n";
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]


//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 1.0, far plane to 1000.0, field of view 60 degrees
    const qreal zNear = 1.0, zFar = 1000.0, fov = 60.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}
//! [5]

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//! [6]
    // Calculate view transformation
    m_camera.update(m_movementDir);
    QMatrix4x4 view = m_camera.getView();

    //Calculate model transformation
    m_model.setToIdentity();
    m_model.translate(0.0, 0.0, -5.0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp", projection * view * m_model);
//! [6]

    // Draw cube geometry
    geometries->drawGeometry(&program);
    mesh.draw(&program);
}
