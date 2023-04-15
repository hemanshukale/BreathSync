#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPoint>
#include <QtGui>
#include <QColor>
#include "mode.h"
#include <QTime>
#include <QTimer>
#include <QMap>
#include <QString>
#include "dialog.h"
#include "defaults.h"

/*!
 * \brief The MainData struct
 */
struct MainData
{
    Mode *currMode ; ///< Pointer to the active Mode
    QTime *modeTimer; ///< Maintains the time elapsed in the current mode
    QTimer *timeKeeper; ///< Timer firing interrupt when the time set for the mode is elapsed
    QHash<quint8,Mode*> modeList; ///< Maintain the list of pointers to modes
    quint8 currModeEnum; ///< Keeps the mode number (enum) of the active mode
    quint8 shapeOpacity = 127; ///< Default shape opacity to start with
    quint8 freq = 30; ///< Sets the shape update fps
    bool showTitleBar = false; ///< To show the title bar : toggled by double click
    QPoint oldPos = QPoint(0,0); ///< Keeps the position for calculation
    QPoint ellipse_size = QPoint(300,300); ///< Stores the size of ellipse
    QPoint windowSize= QPoint(300,300); ///< Stores the size of window
    Mode *lastMode = NULL; ///< Stores the mode which was active before the current one
    quint8 currFocus = 0; ///< Stores which mode is in focus currently
    Dialog *dialog; ///< Pointer to the dialog class
};

/*!
 * \brief MainWindow::MainWindow Constructor
 * \param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Setting up UI
    dptr=new MainData;
    qDebug() << Q_FUNC_INFO << "1";
    ui->setupUi(this);
    startTimer((int)(SEC_TO_MSEC/dptr->freq));
    dptr->dialog = new Dialog(this);
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowTitle("Breathe");
    this->setWindowOpacity(0.7);
    this->setAttribute(Qt::WA_NoSystemBackground);
    qDebug() << Q_FUNC_INFO << "2";

    // Initiating and adding modes to the Mode list
    dptr->modeList[Modes::Inhale]  = new Mode(Modes::Inhale, dptr->shapeOpacity);
    dptr->modeList[Modes::HoldIn]  = new Mode(Modes::HoldIn, dptr->shapeOpacity);
    dptr->modeList[Modes::Exhale]  = new Mode(Modes::Exhale, dptr->shapeOpacity);
    dptr->modeList[Modes::HoldOut] = new Mode(Modes::HoldOut,dptr->shapeOpacity);

    dptr->modeList[Modes::Inhale]->setNext(dptr->modeList[Modes::HoldIn]);
    dptr->modeList[Modes::HoldIn]->setNext(dptr->modeList[Modes::Exhale]);
    dptr->modeList[Modes::Exhale]->setNext(dptr->modeList[Modes::HoldOut]);
    dptr->modeList[Modes::HoldOut]->setNext(dptr->modeList[Modes::Inhale]);

    dptr->modeList[Modes::Inhale]->setChangable(Changable::Increasing);
    dptr->modeList[Modes::HoldIn]->setChangable(Changable::Increasing);
    dptr->modeList[Modes::Exhale]->setChangable(Changable::Decreasing);
    dptr->modeList[Modes::HoldOut]->setChangable(Changable::Decreasing);

    // update settings based on stored settings
    updateSettings();
    qDebug() << Q_FUNC_INFO << "3";

    // Start with the first mode -> Inhale in this case
    dptr->currModeEnum = Modes::Inhale;
    dptr->currMode = dptr->modeList[dptr->currModeEnum];
    dptr->timeKeeper = new QTimer;
    dptr->modeTimer = new QTime;
    dptr->timeKeeper->setSingleShot(true);
//    QTimer::singleShot(currMode->getTimeMS(),this,SLOT(onModeTimeout()));
    dptr->timeKeeper->singleShot(dptr->currMode->getTimeMS(),this,SLOT(onModeTimeout()));
    dptr->modeTimer->start();

    // Update Window settings
    this->resize(300,300);
    QSize sz = this->window()->size();
    dptr->windowSize = QPoint(sz.width(),sz.height());
    Mode::setScreenSize(dptr->windowSize);

    // Set SIGNAL-SLOT mapping
    connect(dptr->dialog,SIGNAL(settingsClosed()),this,SLOT(showWindow()) );
    connect(dptr->dialog,SIGNAL(settingsChanged()),this,SLOT(updateSettings()) );

    qInfo() << Q_FUNC_INFO << dptr->windowSize;
}


/*!
 * \brief MainWindow::onModeTimeout
 * Called when QTimer timeKeeper times out
 */
void MainWindow::onModeTimeout()
{
//    qDebug() << Q_FUNC_INFO << "Curr Mode=" << dptr->currMode->getMode() << dptr->currMode->getTimeMS();
    dptr->lastMode = dptr->currMode;
    dptr->currMode = dptr->currMode->getNext();
//    QTimer::singleShot(currMode->getTimeMS(),this,SLOT(onModeTimeout()));
    dptr->timeKeeper->singleShot(dptr->currMode->getTimeMS(),this,SLOT(onModeTimeout()));
    dptr->modeTimer->start();
//    qDebug() << Q_FUNC_INFO << "New  Mode=" << dptr->currMode->getMode() << dptr->currMode->getTimeMS();
}

/*!
 * \brief MainWindow::sizeHint
 * \return
 */
QSize MainWindow::sizeHint() const
{
    return QSize(dptr->windowSize.x(), dptr->windowSize.y()) ;// Set this to the exact image resolution
}


/*!
 * \brief MainWindow::drawShape Draw shapes based on given input and shape set in current mode
 * \param qp
 * \param xywh
 * \param shape
 */
void MainWindow::drawShape(QPainter &qp, QRect xywh, quint8 shape)
{
//    qInfo() << Q_FUNC_INFO << xywh << shape;
    switch (shape)
    {
        case Shape::Ellipse:
        {
            qp.drawEllipse(xywh);
            break;
        }
        case Shape::Rectangle:
        {
            qp.drawRect(xywh);
            break;
        }
        case Shape::RoundedRectangle:
        {
            qp.drawRoundRect(xywh);
            break;
        }
    }
}


/*!
 * \brief MainWindow::isModeInFocus Get the combo mode in focus since we are sharing shapes for two modes - ex. Inhale or Exhale mode will return Focus::InhaleExhale
 * \param mode
 * \param focus
 * \return
 */
bool MainWindow::isModeInFocus(quint8 mode, quint8 focus)
{
    if ( ((mode == Modes::Inhale || mode == Modes::Exhale ) && focus == Focus::InhaleExhale)
      || ((mode == Modes::HoldOut || mode == Modes::HoldIn) && focus == Focus::HoldInOut) )
        return true;
    return false;
}

/*!
 * \brief MainWindow::paintEvent Called when repaint or update is called
 */
void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter qp ;
    qp.begin(this);
    qp.setRenderHint(QPainter::Antialiasing);
    qp.setPen(Qt::NoPen);
    int elapsedTime = dptr->modeTimer->elapsed();
    //= timeKeeper->interval()-timeKeeper->remainingTime();
//    qDebug() << Q_FUNC_INFO << "Time" << elapsedTime << dptr->currMode->getColor() << dptr->currMode->getShapeCoord(elapsedTime);
    QPen pen(Qt::NoPen);

    if (!(!dptr->lastMode))
    {
        if (isModeInFocus(dptr->lastMode->getMode(), dptr->currFocus))
                pen = QPen(Qt::gray, 3, Qt::DashDotLine);
        qp.setPen(pen);
        qp.setBrush(dptr->lastMode->getColor());
        drawShape(qp, dptr->lastMode->getEndShapeCoord(), dptr->lastMode->getShape());
    }

    // If we are editing any mode using numpad or Ctrl+Scroll, this will draw an outline around it to show that this shape is being edited
    if (isModeInFocus(dptr->currMode->getMode(), dptr->currFocus))
        pen = QPen(Qt::gray, 3, Qt::DashDotLine);
    else pen = QPen(Qt::NoPen);
    qp.setPen(pen);
    qp.setBrush(dptr->currMode->getColor());
    drawShape(qp, dptr->currMode->getShapeCoord(elapsedTime), dptr->currMode->getShape());
    qp.end();
}

/*!
 * \brief MainWindow::mouseMoveEvent Called when mouse move event is detected
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = QPoint(event->globalPos() - dptr->oldPos);
    this->move(this->x() + delta.x(), this->y() + delta.y());
    dptr->oldPos = event->globalPos();
}


/*!
 * \brief MainWindow::mousePressEvent Called when mouse press event is detected
 * \param event
 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    dptr->oldPos = event->globalPos();
    try
    {
        qInfo() << event->button() << Qt::RightButton << Qt::LeftButton << event->type() << QEvent::MouseButtonRelease << QEvent::MouseButtonPress;
        if (event->button() == Qt::RightButton && dptr->showTitleBar && event->type() ==  QEvent::MouseButtonPress)
        {
            dptr->dialog->show();
//            this->hide();
            return;
        }
        // If left double click detected, toggle the showing of title bar
        // Also toggles showing the settings dialog box by middle click
        if (event->button() == Qt::LeftButton && event->type() ==  QEvent::MouseButtonDblClick)
        {
            dptr->showTitleBar = ! dptr->showTitleBar;
            if (dptr->showTitleBar)
            {
                this->setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
                this->activateWindow();
            }
            else
            {
                this->setWindowFlags( Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
            }
            this->setAttribute(Qt::WA_TranslucentBackground);
            this->show();
        }
    }
    catch (QException exc)
    {
        qInfo() << "Exc:" ;
    }
}

/*!
 * \brief MainWindow::wheelEvent Called when mouse wheel event is detected
 * \param event
 */
void MainWindow::wheelEvent(QWheelEvent *event)
{
    qint8 scroll=0, scrollX=0, scrollY=0;
    if (event->angleDelta().y() == 0 ) return ;
    else if(event->angleDelta().y() > 0) // up Wheel
        scroll = 1;
    else if(event->angleDelta().y() < 0) //down Wheel
        scroll = -1;

    if ( event->modifiers() & Qt::ControlModifier ) scrollY = scroll;
    if ( event->modifiers() & Qt::ShiftModifier )   scrollX = scroll;

    setFocusedModesScaling(scrollX, scrollY);
}

/*!
 * \brief MainWindow::setFocusedModesScaling Set the scaling (sizes) of the shapes with Ctrl + Scroll
 * \param scrollX
 * \param scrollY
 */
void MainWindow::setFocusedModesScaling(qint8 scrollX, qint8 scrollY)
{
    if (dptr->currFocus == Focus::NoFocus) return ;
    else if (dptr->currFocus == Focus::InhaleExhale)
    {
        dptr->modeList[Modes::Inhale]->changeUserScaling(scrollX,scrollY);
        dptr->modeList[Modes::Exhale]->changeUserScaling(scrollX,scrollY);
    }
    else if (dptr->currFocus == Focus::HoldInOut)
    {
        dptr->modeList[Modes::HoldIn]->changeUserScaling(scrollX,scrollY);
        dptr->modeList[Modes::HoldOut]->changeUserScaling(scrollX,scrollY);
    }
}

/*!
 * \brief MainWindow::showWindow
 */
void MainWindow::showWindow()
{
    qInfo() << Q_FUNC_INFO;
    this->show();
}

/*!
 * \brief MainWindow::resizeEvent This function is called when Window is resized
 * \param event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{

    inherited::resizeEvent(event);
    QSize sz = this->window()->size();
    dptr->windowSize = QPoint(sz.width(),sz.height());
    Mode::setScreenSize(dptr->windowSize);
}

/*!
 * \brief MainWindow::getNextFocus After Tab is pressed change focus to next mode
 */
void MainWindow::getNextFocus()
{
    dptr->currFocus++;

//    dptr->currFocus %= Focus::CountKeeper;
    dptr->currFocus %= enumFocus.keyCount();

    qInfo() << Q_FUNC_INFO << dptr->currFocus;
    if (dptr->currFocus == Focus::NoFocus)
    {
        quint8 changed = 1;
        if (dptr->modeList[Modes::Inhale]->getUserScaling() != dptr->dialog->getUserScaling(Modes::Inhale) && changed++)
            dptr->dialog->setUserScaling(Modes::Inhale,dptr->modeList[Modes::Inhale]->getUserScaling());

        if (dptr->modeList[Modes::Exhale]->getUserScaling() != dptr->dialog->getUserScaling(Modes::Exhale))
            dptr->dialog->setUserScaling(Modes::Exhale,dptr->modeList[Modes::Exhale]->getUserScaling());

        if (dptr->modeList[Modes::HoldIn]->getUserScaling() != dptr->dialog->getUserScaling(Modes::HoldIn))
            dptr->dialog->setUserScaling(Modes::HoldIn,dptr->modeList[Modes::HoldIn]->getUserScaling());

        if (dptr->modeList[Modes::HoldOut]->getUserScaling() != dptr->dialog->getUserScaling(Modes::HoldOut))
            dptr->dialog->setUserScaling(Modes::HoldOut,dptr->modeList[Modes::HoldOut]->getUserScaling());

        if (dptr->modeList[Modes::Inhale]->getPosition() != dptr->dialog->getPosition(Modes::Inhale))
            dptr->dialog->setPosition(Modes::Inhale,dptr->modeList[Modes::Inhale]->getPosition());

        if (dptr->modeList[Modes::HoldIn]->getPosition() != dptr->dialog->getPosition(Modes::HoldIn))
            dptr->dialog->setPosition(Modes::HoldIn,dptr->modeList[Modes::HoldIn]->getPosition());

        dptr->dialog->saveUserScaling();
        dptr->dialog->savePosition();
    }
}

/*!
 * \brief MainWindow::setFocusedModesPosition Set the postition of the shape in focus based on input
 * \param position
 */
void MainWindow::setFocusedModesPosition(quint8 position)
{
    if (dptr->currFocus == Focus::NoFocus) return ;
    else if (dptr->currFocus == Focus::InhaleExhale)
    {
        dptr->modeList[Modes::Inhale]->setPosition(position);
        dptr->modeList[Modes::Exhale]->setPosition(position);
        qInfo() << Q_FUNC_INFO << "InhExh" << position;

    }
    else if (dptr->currFocus == Focus::HoldInOut)
    {
        dptr->modeList[Modes::HoldIn]->setPosition(position);
        dptr->modeList[Modes::HoldOut]->setPosition(position);
        qInfo() << Q_FUNC_INFO << "HoldInOut" << position;
    }
}

/*!
 * \brief MainWindow::keyPressEvent Called when KeyPress event is detected
 * \param event
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Tab: getNextFocus(); break;
        case Qt::Key_1:   setFocusedModesPosition(Position::TopLeft); break;
        case Qt::Key_2:   setFocusedModesPosition(Position::Top); break;
        case Qt::Key_3:   setFocusedModesPosition(Position::TopRight); break;
        case Qt::Key_4:   setFocusedModesPosition(Position::Left); break;
        case Qt::Key_5:   setFocusedModesPosition(Position::Centred); break;
        case Qt::Key_6:   setFocusedModesPosition(Position::Right); break;
        case Qt::Key_7:   setFocusedModesPosition(Position::BottomLeft); break;
        case Qt::Key_8:   setFocusedModesPosition(Position::Bottom); break;
        case Qt::Key_9:   setFocusedModesPosition(Position::BottomRight); break;
    }
}

/*!
 * \brief MainWindow::timerEvent
 * \param event
 */
void MainWindow::timerEvent(QTimerEvent *event)
{
    // refresh window
    this->repaint();
}


/*!
 * \brief MainWindow::updateSettings Set all the mode parameters by getting the settings from Dialog class
 */
void MainWindow::updateSettings()
{

    dptr->modeList[Modes::Inhale]->setShape(dptr->dialog->getShape(Modes::Inhale));
    dptr->modeList[Modes::Exhale]->setShape(dptr->dialog->getShape(Modes::Exhale));
    dptr->modeList[Modes::HoldIn]->setShape(dptr->dialog->getShape(Modes::HoldIn));
    dptr->modeList[Modes::HoldOut]->setShape(dptr->dialog->getShape(Modes::HoldOut));

    dptr->modeList[Modes::Inhale]->setPosition(dptr->dialog->getPosition(Modes::Inhale));
    dptr->modeList[Modes::Exhale]->setPosition(dptr->dialog->getPosition(Modes::Exhale));
    dptr->modeList[Modes::HoldIn]->setPosition(dptr->dialog->getPosition(Modes::HoldIn));
    dptr->modeList[Modes::HoldOut]->setPosition(dptr->dialog->getPosition(Modes::HoldOut));

    dptr->modeList[Modes::Inhale]->setDirection(dptr->dialog->getDirection(Modes::Inhale));
    dptr->modeList[Modes::Exhale]->setDirection(dptr->dialog->getDirection(Modes::Exhale));
    dptr->modeList[Modes::HoldIn]->setDirection(dptr->dialog->getDirection(Modes::HoldIn));
    dptr->modeList[Modes::HoldOut]->setDirection(dptr->dialog->getDirection(Modes::HoldOut));

    dptr->modeList[Modes::Inhale]->setTimeMS(dptr->dialog->getTimeMS(Modes::Inhale));
    dptr->modeList[Modes::Exhale]->setTimeMS(dptr->dialog->getTimeMS(Modes::Exhale));
    dptr->modeList[Modes::HoldIn]->setTimeMS(dptr->dialog->getTimeMS(Modes::HoldIn));
    dptr->modeList[Modes::HoldOut]->setTimeMS(dptr->dialog->getTimeMS(Modes::HoldOut));

    dptr->modeList[Modes::Inhale]->setColor(dptr->dialog->getColor(Modes::Inhale));
    dptr->modeList[Modes::Exhale]->setColor(dptr->dialog->getColor(Modes::Exhale));
    dptr->modeList[Modes::HoldIn]->setColor(dptr->dialog->getColor(Modes::HoldIn));
    dptr->modeList[Modes::HoldOut]->setColor(dptr->dialog->getColor(Modes::HoldOut));

    dptr->modeList[Modes::Inhale]->setUserScaling(dptr->dialog->getUserScaling(Modes::Inhale));
    dptr->modeList[Modes::Exhale]->setUserScaling(dptr->dialog->getUserScaling(Modes::Exhale));
    dptr->modeList[Modes::HoldIn]->setUserScaling(dptr->dialog->getUserScaling(Modes::HoldIn));
    dptr->modeList[Modes::HoldOut]->setUserScaling(dptr->dialog->getUserScaling(Modes::HoldOut));

    dptr->modeList[Modes::Inhale]->setTransparency(255-dptr->dialog->getShapeTransparency()*2.55 );
    dptr->modeList[Modes::Exhale]->setTransparency(255-dptr->dialog->getShapeTransparency()*2.55 );
    dptr->modeList[Modes::HoldIn]->setTransparency(255-dptr->dialog->getShapeTransparency()*2.55 );
    dptr->modeList[Modes::HoldOut]->setTransparency(255-dptr->dialog->getShapeTransparency()*2.55 );

    this->setWindowOpacity(1- ((float)dptr->dialog->getWindowTransparency()* PERCENT_INV_MULT) );

    qInfo() << Q_FUNC_INFO
            << dptr->modeList[Modes::Inhale]->getUserScaling()
            << dptr->modeList[Modes::Exhale]->getUserScaling()
            << dptr->modeList[Modes::HoldIn]->getUserScaling()
            << dptr->modeList[Modes::HoldOut]->getUserScaling();

}

/*!
 * \brief MainWindow::~MainWindow Destructor
 */
MainWindow::~MainWindow()
{
    delete ui;
}

