#include "camera.h"

Camera::Camera():m_mouseRotation(false)
{
}

void Camera::initMouseRotation()
{
    m_mouseRotation = true;
    m_lastMousePos = QCursor::pos();
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void Camera::updateMouseRotation()
{
    if(m_mouseRotation)
    {
        QPoint diff = QCursor::pos() - m_lastMousePos;
        QCursor::setPos(m_lastMousePos);

        m_orientation = QQuaternion::fromAxisAndAngle(LOCAL_UP,-MOVEMENT_SPEED*diff.x()) * m_orientation;
        m_orientation = QQuaternion::fromAxisAndAngle(right(),-MOVEMENT_SPEED*diff.y()) * m_orientation;
    }
}

void Camera::endMouseRotation()
{
    m_mouseRotation = false;
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void Camera::updateMovement(QVector3D dir)
{
    m_position+= front()*MOVEMENT_SPEED*dir.z();
    m_position+= up()*MOVEMENT_SPEED*dir.y();
    m_position+= right()*MOVEMENT_SPEED*dir.x();
}

void Camera::update(QVector3D dir)
{
    updateMouseRotation();
    updateMovement(dir);
}

QMatrix4x4 Camera::getView()
{
    QMatrix4x4 returned;
    returned.setToIdentity();
    returned.rotate(m_orientation.conjugate());
    returned.translate(-m_position);
    return returned;
}

QVector3D Camera::front()
{
    return (m_orientation.rotatedVector(LOCAL_FRONT));
}

QVector3D Camera::right()
{
    return (m_orientation.rotatedVector(LOCAL_RIGHT));
}

QVector3D Camera::up()
{
    return (m_orientation.rotatedVector(LOCAL_UP));
}
