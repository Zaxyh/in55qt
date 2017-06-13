#ifndef CAMERA_H
#define CAMERA_H

#include "QApplication"
#include "QCursor"
#include "QQuaternion"
#include "QMatrix4x4"
#include "QVector2D"
#include "QVector3D"

class Camera
{
public:
    Camera();

    void initMouseRotation();
    void endMouseRotation();

    void update(QVector3D dir);
    QMatrix4x4 getView();
    QVector3D front();
    QVector3D right();
    QVector3D up();

protected:
    void updateMouseRotation();
    void updateMovement(QVector3D dir);

private:
    const qreal MOVEMENT_SPEED = 0.05f;
    const qreal ROTATION_SPEED = 0.05f;

    const QVector3D LOCAL_FRONT = QVector3D(0,0,-1);
    const QVector3D LOCAL_UP = QVector3D(0,1,0);
    const QVector3D LOCAL_RIGHT = QVector3D(1,0,0);

    QQuaternion m_orientation;
    QVector3D m_position;
    QPoint m_lastMousePos;
    bool m_mouseRotation;
};

#endif // CAMERA_H
