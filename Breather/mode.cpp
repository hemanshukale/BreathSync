#include "mode.h"
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QString>

/*!
 * \brief The ModeData struct
 */
struct ModeData
{
    QColor color; ///< Color of the shape
    quint32 timeMS; ///< Time the shape will be changing
    quint8 thisMode; ///< Alotted num (enum) to this Mode
    quint8 shape = Shape::Ellipse; ///< Selected shape of the mode
    quint8 changable = Changable::Increasing; ///< Current size change status of the shape
    quint8 position = Position::Centred; ///< Set position of the shape
    quint8 direction = Direction::Vertical; ///< Direction of change of the shape
    QRect shapeCoord; ///< Will store the calculated coordinates of this shape
    quint8 transparency; ///< Store the trasnparancy for the shape
    float minScreenToUse = 0.1, maxScreenToUse = 0.9; ///< min max Extent of the screen to use
    float userScalingX = 1, userScalingY = 1; ///< User multiplier - to set the size
};

QPoint Mode::screenSize = QPoint(0,0);

/*!
 * \brief Mode::Mode Constructor for Mode Class
 * \param mode
 * \param time
 * \param color
 * \param transparency
 */
Mode::Mode(quint8 mode, quint32 time, const QColor &color, quint8 transparency)
{
    d=new ModeData;
    d->thisMode=mode;
    d->timeMS=time;
    d->color=color;
    d->transparency=transparency;
    d->color.setAlpha(d->transparency);
}

/*!
 * \brief Mode::Mode Constructor for Mode Class
 * \param mode
 * \param transparency
 */
Mode::Mode(quint8 mode, quint8 transparency)
{
    d=new ModeData;
    d->thisMode=mode;
    d->transparency=transparency;
}

/*!
 * \brief Mode::Mode Constructor for Mode Class
 * \param mode
 */
Mode::Mode(quint8 mode)
{
    d=new ModeData;
    d->thisMode=mode;
}

/*!
 * \brief Mode::setColor
 * \param color
 */
void Mode::setColor(const QColor &color)
{
    d->color = color;
    d->color.setAlpha(d->transparency);
}

/*!
 * \brief Mode::setTransparency
 * \param transparency
 */
void Mode::setTransparency(const quint8 &transparency)
{
    d->transparency=transparency;
    d->color.setAlpha(d->transparency);
}

/*!
 * \brief Mode::setTimeMS
 * \param time
 */
void Mode::setTimeMS(const quint32 &time)
{
    d->timeMS = time;
}

/*!
 * \brief Mode::setMode
 * \param mode
 */
void Mode::setMode(const quint8 &mode)
{
    d->thisMode=mode;
}

/*!
 * \brief Mode::setChangable
 * \param changable
 */
void Mode::setChangable(const quint8 &changable)
{
    d->changable = changable;
}

/*!
 * \brief Mode::setUserScaling
 * \param scalingX
 * \param scalingY
 */
void Mode::setUserScaling(float scalingX, float scalingY)
{
    d->userScalingX = scalingX;
    d->userScalingY = scalingY;
}

/*!
 * \brief Mode::setUserScaling
 * \param scaling
 */
void Mode::setUserScaling(const QPointF &scaling)
{
    d->userScalingX = scaling.x();
    d->userScalingY = scaling.y();
}

/*!
 * \brief Mode::getColor
 * \return
 */
QColor Mode::getColor()
{
    return d->color;
}

/*!
 * \brief Mode::getTransparency
 * \return
 */
quint8 Mode::getTransparency()
{
    return d->transparency;
}

/*!
 * \brief Mode::getTimeMS
 * \return
 */
quint32 Mode::getTimeMS()
{
    return d->timeMS;
}

/*!
 * \brief Mode::getMode
 * \return
 */
quint8 Mode::getMode()
{
    return d->thisMode;
}

/*!
 * \brief Mode::setNext
 * \param nextMode
 */
void Mode::setNext(Mode* nextMode)
{
    next = nextMode;
}

/*!
 * \brief Mode::getNext
 * \return
 */
Mode* Mode::getNext()
{
    return next;
}

/*!
 * \brief Mode::getShape
 * \return
 */
quint8 Mode::getShape()
{
    return d->shape;
}

/*!
 * \brief Mode::setShape
 * \param shape
 */
void Mode::setShape(const quint8 &shape)
{
    d->shape = shape;
}

/*!
 * \brief Mode::getPosition
 * \return
 */
quint8 Mode::getPosition()
{
    return d->position;
}

/*!
 * \brief Mode::getDirection
 * \return
 */
quint8 Mode::getDirection()
{
    return d->position;
}

/*!
 * \brief Mode::setPosition
 * \param position
 */
void Mode::setPosition(const quint8 &position)
{
    d->position = position;
}

/*!
 * \brief Mode::setDirection
 * \param direction
 */
void Mode::setDirection(const quint8 &direction)
{
    d->direction = direction;
}

/*!
 * \brief Mode::setScreenUsage
 * \param mintouse
 * \param maxtouse
 */
void Mode::setScreenUsage(const float &mintouse, const float &maxtouse)
{
    d->minScreenToUse = mintouse;
    d->maxScreenToUse = maxtouse;
}

/*!
 * \brief Mode::setScreenSize
 * \param screensize
 */
void Mode::setScreenSize(const QPoint &screensize)
{
    screenSize = QPoint(screensize);
}

/*!
 * \brief Mode::getRatioCompleted Get ratio of how much fraction the time has elapsed w.r.t. the set time
 * \param elapsedTimeMS
 * \return
 */
float Mode::getRatioCompleted(const quint32 &elapsedTimeMS)
{
    if     (d->changable == Changable::Increasing )
    return (float) elapsedTimeMS/d->timeMS;
    else if (d->changable == Changable::Decreasing )
            return (1 - (float) elapsedTimeMS/d->timeMS);
    else return 0;
}

/*!
 * \brief Mode::getShapeDimensions Get the dimensions (size) of this mode shape
 * \param elapsedTimeMS
 * \return
 */
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

/*!
 * \brief Mode::getScreenCentre
 * \return
 */
QPoint Mode::getScreenCentre()
{
    return QPoint(screenSize.x()/2, screenSize.y()/2);
}

/*!
 * \brief Mode::getInitShapeCoord
 * \return
 */
QRect Mode::getInitShapeCoord()
{
    return getShapeCoord(0);
}

/*!
 * \brief Mode::getEndShapeCoord
 * \return
 */
QRect Mode::getEndShapeCoord()
{
    return getShapeCoord(d->timeMS);
}

/*!
 * \brief Mode::changeUserScaling
 * \param scrollX
 * \param scrollY
 */
void Mode::changeUserScaling(qint8 scrollX, qint8 scrollY)
{
    d->userScalingX += scrollX*0.05;
    d->userScalingY += scrollY*0.05;
    if (d->userScalingX > 1) d->userScalingX = 1; if (d->userScalingX < 0) d->userScalingX = 0;
    if (d->userScalingY > 1) d->userScalingY = 1; if (d->userScalingY < 0) d->userScalingY = 0;
    qInfo() << Q_FUNC_INFO << scrollX << scrollY << d->userScalingX << d->userScalingY;
//    setAutoScaling();
}

/*!
 * \brief Mode::getUserScaling
 * \return
 */
QPointF Mode::getUserScaling()
{
    return QPointF(d->userScalingX, d->userScalingY);
}

/*!
 * \brief Mode::getShapeCoord
 * \param elapsedTimeMS
 * \return
 */
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
