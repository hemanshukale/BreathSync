#ifndef MODE_H
#define MODE_H

#include <QColor>
#include <QPoint>
#include <QRect>
#include <QObject>

enum Shape : quint8
{
    Ellipse=0,
    Rectangle,
    RoundedRectangle
};

enum Position : quint8
{
    TopLeft=0,
    Top,
    TopRight,
    Left,
    Centred,
    Right,
    BottomLeft,
    Bottom,
    BottomRight
};

enum Modes : quint8
{
    Inhale=0,
    HoldIn=1,
    Exhale=2,
    HoldOut=3
};

enum Changable: quint8
{
    None=0,
    Increasing,
    Decreasing,
};

enum Direction : quint8
{
    Both=0,
    Horizontal,
    Vertical
};

struct ModeData;

class Mode : public QObject
{
    Q_OBJECT
public:
    Mode(quint8 mode, quint32 time, const QColor &color, quint8 transparency);
    Mode(quint8 mode, quint8 transparency);
    Mode(quint8 mode);

    void setColor(const QColor &color);
    void setTransparency(const quint8 &transparency);
    void setTimeMS(const quint32 &time);
    void setMode(const quint8 &mode);
    static void setScreenSize(const QPoint &screensize);
    void setNext(Mode* setMode);
    void setShape(const quint8 &shape);
    void setPosition(const quint8 &position);
    void setScreenUsage(const float &mintouse, const float &maxtouse);
    void setChangable(const quint8 &changable);
    void setDirection(const quint8 &direction);
    void setUserScaling(float scalingX, float scalingY);
    void setUserScaling(const QPointF &scaling);
    void changeUserScaling(qint8 scrollX, qint8 scrollY);

    QColor  getColor();
    quint8  getTransparency();
    quint32 getTimeMS();
    quint8  getMode();
    Mode*   getNext();
    quint8  getShape();
    quint8  getPosition();
    quint8  getDirection();

    float   getRatioCompleted(const quint32 &elapsedTimeMS);
    QRect   getShapeCoord(const quint32 &elapsedTimeMS);
    QRect   getInitShapeCoord();
    QRect   getEndShapeCoord();
    QPoint  getShapeDimensions(const quint32 &elapsedTimeMS);
    QPoint  getScreenCentre();
    QPointF getUserScaling();


private:
    ModeData *d;
    Mode* next; ///< Pointer to the next mode - basically circular linked list
    static QPoint screenSize;
};

#endif // MODE_H
