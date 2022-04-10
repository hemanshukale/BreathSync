#include "mode.h"
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QString>

struct ModeData
{
    QColor color;
    quint32 timeMS;
    quint8 thisMode, shape = Shape::Ellipse, changable = Changable::Increasing, position = Position::Centred, direction = Direction::Vertical;
    QRect shapeCoord;
    quint8 transparency;
    float minScreenToUse = 0.1, maxScreenToUse = 0.9;
    float userScalingX = 1, userScalingY = 1;
};

QPoint Mode::screenSize = QPoint(0,0);

Mode::Mode(quint8 mode, quint32 time, const QColor &color, quint8 transparency)
{
    d=new ModeData;
    d->thisMode=mode;
    d->timeMS=time;
    d->color=color;
    d->transparency=transparency;
    d->color.setAlpha(d->transparency);
}

Mode::Mode(quint8 mode, quint8 transparency)
{
    d=new ModeData;
    d->thisMode=mode;
    d->transparency=transparency;
}

Mode::Mode(quint8 mode)
{
    d=new ModeData;
    d->thisMode=mode;
}

void Mode::setColor(const QColor &color)
{
    d->color = color;
    d->color.setAlpha(d->transparency);
}

void Mode::setTransparency(const quint8 &transparency)
{
    d->transparency=transparency;
    d->color.setAlpha(d->transparency);
}

void Mode::setTimeMS(const quint32 &time)
{
    d->timeMS = time;
}

void Mode::setMode(const quint8 &mode)
{
    d->thisMode=mode;
}

void Mode::setChangable(const quint8 &changable)
{
    d->changable = changable;
}

void Mode::setUserScaling(float scalingX, float scalingY)
{
    d->userScalingX = scalingX;
    d->userScalingY = scalingY;
}

void Mode::setUserScaling(const QPointF &scaling)
{
    d->userScalingX = scaling.x();
    d->userScalingY = scaling.y();
}

QColor Mode::getColor()
{
    return d->color;
}

quint8 Mode::getTransparency()
{
    return d->transparency;
}

quint32 Mode::getTimeMS()
{
    return d->timeMS;
}

quint8 Mode::getMode()
{
    return d->thisMode;
}

void Mode::setNext(Mode* nextMode)
{
    next = nextMode;
}

Mode* Mode::getNext()
{
    return next;
}

quint8 Mode::getShape()
{
    return d->shape;
}

void Mode::setShape(const quint8 &shape)
{
    d->shape = shape;
}

quint8 Mode::getPosition()
{
    return d->position;
}

void Mode::setPosition(const quint8 &position)
{
    d->position = position;
}

void Mode::setDirection(const quint8 &direction)
{
    d->direction = direction;
}

void Mode::setScreenUsage(const float &mintouse, const float &maxtouse)
{
    d->minScreenToUse = mintouse;
    d->maxScreenToUse = maxtouse;
}

void Mode::setScreenSize(const QPoint &screensize)
{
    screenSize = QPoint(screensize);
}

float Mode::getRatioCompleted(const quint32 &elapsedTimeMS)
{
    if     (d->changable == Changable::Increasing )
    return (float) elapsedTimeMS/d->timeMS;
    else if (d->changable == Changable::Decreasing )
            return (1 - (float) elapsedTimeMS/d->timeMS);
    else return 0;
}

QPoint Mode::getShapeDimensions(const quint32 &elapsedTimeMS)
{
    QPoint dims;
    float completedRatio = getRatioCompleted(elapsedTimeMS);
    float completedRatioScreen = ((d->maxScreenToUse-d->minScreenToUse)*completedRatio + d->minScreenToUse);

    if (d->direction == Direction::Both || d->direction== Direction::Horizontal )
        dims.setX(screenSize.x() * d->userScalingX * completedRatioScreen);
    else
        dims.setX(screenSize.x() * d->userScalingX * d->maxScreenToUse);

    if (d->direction == Direction::Both || d->direction == Direction::Vertical )
        dims.setY(screenSize.y() * d->userScalingY * completedRatioScreen);
    else
        dims.setY(screenSize.y() * d->userScalingY * d->maxScreenToUse);

    if (dims.x() > screenSize.x() * d->maxScreenToUse) dims.setX(screenSize.x() * d->maxScreenToUse);
    if (dims.y() > screenSize.y() * d->maxScreenToUse) dims.setY(screenSize.y() * d->maxScreenToUse);

//    qDebug() << Q_FUNC_INFO << completedRatio << screenSize
//             << d->minScreenToUse << d->maxScreenToUse << dims;
    return  dims;
}

QPoint Mode::getScreenCentre()
{
    return QPoint(screenSize.x()/2, screenSize.y()/2);
}

QRect Mode::getInitShapeCoord()
{
    return getShapeCoord(0);
}

QRect Mode::getEndShapeCoord()
{
    return getShapeCoord(d->timeMS);
}

void Mode::changeUserScaling(qint8 scrollX, qint8 scrollY)
{
    d->userScalingX += scrollX*0.05;
    d->userScalingY += scrollY*0.05;
    if (d->userScalingX > 1) d->userScalingX = 1; if (d->userScalingX < 0) d->userScalingX = 0;
    if (d->userScalingY > 1) d->userScalingY = 1; if (d->userScalingY < 0) d->userScalingY = 0;
    qInfo() << Q_FUNC_INFO << scrollX << scrollY << d->userScalingX << d->userScalingY;
//    setAutoScaling();
}

QPointF Mode::getUserScaling()
{
    return QPointF(d->userScalingX, d->userScalingY);
}


QRect Mode::getShapeCoord(const quint32 &elapsedTimeMS)
{
    QPoint shapeDims = getShapeDimensions(elapsedTimeMS);
    QRect shapeCoords;
    QPoint centre = getScreenCentre();
    QPoint topLeft = QPoint(centre.x()-shapeDims.x()/2, centre.y()-shapeDims.y()/2 );
    QPoint minTopLeft = QPoint(screenSize.x()*d->minScreenToUse/2, screenSize.y()*d->minScreenToUse/2 );
    QPoint minBottomRight = QPoint(screenSize.x()*(1-d->minScreenToUse/2)-shapeDims.x(), screenSize.y()*(1-d->minScreenToUse/2)-shapeDims.y());
    switch (d->position)
    {
        case Position::Centred:
        {
            shapeCoords.setTopLeft(topLeft);
            break;
        }
        case Position::Top:
        {
            shapeCoords.setTopLeft(QPoint(topLeft.x(), minTopLeft.y()));

            break;
        }
        case Position::Bottom:
        {
            shapeCoords.setTopLeft(QPoint(topLeft.x(), minBottomRight.y()));
            break;

        }
        case Position::Left:
        {
            shapeCoords.setTopLeft(QPoint(minTopLeft.x(), topLeft.y()));
            break;
        }
        case Position::Right:
        {
            shapeCoords.setTopLeft(QPoint(minBottomRight.x(),topLeft.y()));
            break;
        }
        case Position::TopLeft:
        {
            shapeCoords.setTopLeft(QPoint(minTopLeft.x(), minTopLeft.y()));

            break;
        }
        case Position::TopRight:
        {
            shapeCoords.setTopLeft(QPoint(minBottomRight.x(), minTopLeft.y()));
            break;
        }
        case Position::BottomLeft:
        {
            shapeCoords.setTopLeft(QPoint(minTopLeft.x(), minBottomRight.y()));
            break;
        }
        case Position::BottomRight:
        {
            shapeCoords.setTopLeft(QPoint(minBottomRight.x(), minBottomRight.y()));
            break;
        }
    }
    shapeCoords.setX(shapeCoords.x());
    shapeCoords.setY(shapeCoords.y());
    shapeCoords.setWidth(shapeDims.x());
    shapeCoords.setHeight(shapeDims.y());
//    qInfo() << Q_FUNC_INFO << shapeCoords << shapeDims << centre << topLeft;
    return shapeCoords;
}
