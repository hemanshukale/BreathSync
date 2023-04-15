#include "dialog.h"
#include "ui_dialog.h"
#include "mode.h"
#include <QDebug>
#include <QSettings>
#include <QColorDialog>
#include <QSignalMapper>
#include <QMetaEnum>
#include <QDir>

#include "defaults.h"

/*!
 * \brief The DialogData struct.
 */
struct DialogData
{
    QHash<quint16,QRadioButton *>   mapPosition ;  ///< Map Mode and Position to Position Radio Button
    QHash<quint16,QRadioButton *>   mapShape ;     ///< Map Mode and Shape to Shape Radio Button
    QHash<quint16,QRadioButton *>   mapDirection ; ///< Map Mode and Direction to Direction Radio Button
    QHash<quint8, QDoubleSpinBox *> mapTime;       ///< Map Mode to Time Input
    QHash<quint8, QPushButton *>    mapColor;      ///< Map Mode to Color Radio Button
    QHash<quint16, QSpinBox *>      mapSize;       ///< Map Mode to Size Input

    QHash<quint8,QColor> colorMap; ///< Maps Mode to Color values
    QColorDialog *colord;      ///< Pointer to Select Color Dialog Box
    QSignalMapper *mapper;     ///< Pointer to signal mapper class used to set SIGNAL-SLOT mapping to better handle color selection
    quint8 currColorToSet = 0; ///< Stores Mode enum where to store the selected Color
    QHash<quint8,QPointF> userScaling; ///< Map Mode to user set scaling values
    quint8 transparancyShape;  ///< Shape transparancy value
    quint8 transparancyWindow; ///< Window transparancy value

    QHash<quint8,quint8>  stateDirection; ///< Store the last saved direction of a mode
    QHash<quint8,quint8>  stateShape;     ///< Store the last saved shape of a mode
    QHash<quint8,quint8>  statePosition;  ///< Store the last saved position of a mode
    QHash<quint8,QColor>  stateColor;     ///< Store the last saved color of a mode
    QHash<quint8,quint16> stateTime;      ///< Store the last saved time of a mode
    QHash<quint8,QPointF> stateSize;      ///< Store the last saved size of a mode

};

/*!
 * \brief Dialog::Dialog
 * \param parent
 */
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    dptr = new DialogData;
    connect(ui->buttonSave,SIGNAL(released()), this, SLOT(on_SaveClicked()));
    connect(ui->buttonReset,SIGNAL(released()), this, SLOT(on_ResetClicked()));
    connect(ui->buttonDiscard,SIGNAL(released()), this, SLOT(on_DiscardClicked()));
    setHashMapping();
    loadSavedSettings();

    dptr->colord = new QColorDialog(this);
    dptr->colord->setOption(QColorDialog::DontUseNativeDialog);
    dptr->colord->setOption(QColorDialog::ShowAlphaChannel);
    dptr->mapper = new QSignalMapper(this);

    for (quint8 mode : dptr->mapColor.keys() )
    {
        dptr->mapper->setMapping(dptr->mapColor[mode], (int)mode);
        connect(dptr->mapColor[mode],SIGNAL(released()),dptr->mapper,SLOT(map()));
    }

    connect(dptr->mapper,SIGNAL(mapped(int)),this,SLOT(on_ColorSelectClicked(int)) );

    connect(dptr->colord,SIGNAL(colorSelected(QColor)),this,SLOT(on_ColorSelected(QColor)));
    connect(dptr->colord,SIGNAL(currentColorChanged(QColor)),this,SLOT(on_ColorSelected(QColor)));

    for (QRadioButton * instance : dptr->mapPosition.values() + dptr->mapDirection.values() + dptr->mapShape.values())
        connect(instance,SIGNAL(toggled(bool)),this,SLOT(on_SomethingToggled()));

    for (QDoubleSpinBox * instance : dptr->mapTime.values() )
        connect(instance,SIGNAL(valueChanged(double)),this,SLOT(on_SomethingToggled()));

    for (QSpinBox * instance : dptr->mapSize.values())
        connect(instance,SIGNAL(valueChanged(int)),this,SLOT(on_SomethingToggled()));

    connect(ui->transparancyShape,SIGNAL(valueChanged(int)),this,SLOT(on_ShapeTransparancyChanged(int)));
    connect(ui->transparancyWindow,SIGNAL(valueChanged(int)),this,SLOT(on_WindowTransparancyChanged(int)));
}

/*!
 * \brief Dialog::~Dialog
 */
Dialog::~Dialog()
{
    delete ui;
}

/*!
 * \brief Dialog::setHashMapping Set the Hash mapping with the ui objects
 *  Once you set the Hash mapping with the ui objects, you don't need refer to them by their ui name
 *  and can refer by the enums - to prevent hardcoding and for better scalability
 *  You can also loop the iterate this hash map if need to do a change throught instead of calling each ui object individually
 */
void Dialog::setHashMapping()
{
    // Here we are using bitmasking and storing the mode in second byte and position in first byte
    dptr->mapPosition[Modes::Inhale << 8 | Position::TopLeft]     = ui->InhtopLeft;
    dptr->mapPosition[Modes::Inhale << 8 | Position::Top]         = ui->InhtopCentre;
    dptr->mapPosition[Modes::Inhale << 8 | Position::TopRight]    = ui->InhtopRight;
    dptr->mapPosition[Modes::Inhale << 8 | Position::Left]        = ui->InhcentreLeft;
    dptr->mapPosition[Modes::Inhale << 8 | Position::Centred]     = ui->Inhcentre;
    dptr->mapPosition[Modes::Inhale << 8 | Position::Right]       = ui->InhcentreRight;
    dptr->mapPosition[Modes::Inhale << 8 | Position::BottomLeft]  = ui->InhbottomLeft;
    dptr->mapPosition[Modes::Inhale << 8 | Position::Bottom]      = ui->InhbottomCentre;
    dptr->mapPosition[Modes::Inhale << 8 | Position::BottomRight] = ui->InhbottomRight;

    dptr->mapPosition[Modes::HoldIn << 8 | Position::TopLeft]     = ui->HoldtopLeft;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::Top]         = ui->HoldtopCentre;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::TopRight]    = ui->HoldtopRight;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::Left]        = ui->HoldcentreLeft;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::Centred]     = ui->Holdcentre;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::Right]       = ui->HoldcentreRight;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::BottomLeft]  = ui->HoldbottomLeft;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::Bottom]      = ui->HoldbottomCentre;
    dptr->mapPosition[Modes::HoldIn << 8 | Position::BottomRight] = ui->HoldbottomRight;


    dptr->mapShape[Modes::HoldIn << 8 | Shape::Ellipse]          = ui->HoldEllipse;
    dptr->mapShape[Modes::HoldIn << 8 | Shape::Rectangle]        = ui->HoldRectangle;
    dptr->mapShape[Modes::HoldIn << 8 | Shape::RoundedRectangle] = ui->HoldRoundedRect;

    dptr->mapShape[Modes::Inhale << 8 | Shape::Ellipse]          = ui->InhEllipse;
    dptr->mapShape[Modes::Inhale << 8 | Shape::Rectangle]        = ui->InhRectangle;
    dptr->mapShape[Modes::Inhale << 8 | Shape::RoundedRectangle] = ui->InhRoundedRect;


    dptr->mapDirection[Modes::Inhale << 8 | Direction::Horizontal] = ui->InhOnlyHorizontal;
    dptr->mapDirection[Modes::Inhale << 8 | Direction::Vertical]   = ui->InhOnlyVertical;
    dptr->mapDirection[Modes::Inhale << 8 | Direction::Both]       = ui->InhBothChangable;

    dptr->mapDirection[Modes::HoldIn << 8 | Direction::Horizontal] = ui->HoldOnlyHorizontal;
    dptr->mapDirection[Modes::HoldIn << 8 | Direction::Vertical]   = ui->HoldOnlyVertical;
    dptr->mapDirection[Modes::HoldIn << 8 | Direction::Both]       = ui->HoldBothChangable;

    // Here we are storing the mode in first byte
    dptr->mapTime[Modes::Inhale]  = ui->timeInhale;
    dptr->mapTime[Modes::Exhale]  = ui->timeExhale;
    dptr->mapTime[Modes::HoldIn]  = ui->timeHoldIn;
    dptr->mapTime[Modes::HoldOut] = ui->timeHoldOut;

    dptr->mapColor[Modes::Inhale] = ui->colorSetInhale;
    dptr->mapColor[Modes::Exhale] = ui->colorSetExhale;
    dptr->mapColor[Modes::HoldIn] = ui->colorSetHoldIn;
    dptr->mapColor[Modes::HoldOut]= ui->colorSetHoldOut;

    dptr->mapSize[Modes::Inhale << 8 | Direction::Horizontal] = ui->sizeInhHorizontal;
    dptr->mapSize[Modes::Inhale << 8 | Direction::Vertical  ] = ui->sizeInhVertical;
    dptr->mapSize[Modes::HoldIn << 8 | Direction::Horizontal] = ui->sizeHoldInHorizontal;
    dptr->mapSize[Modes::HoldIn << 8 | Direction::Vertical  ] = ui->sizeHoldInVertical;

}

/*!
 * \brief Dialog::setRadioButton Set the parametered radio button as True
 * \param instance QRadioButton
 */
void Dialog::setRadioButton(QRadioButton *instance)
{
    if (!instance) return ;
    #if __cplusplus >= 201703L
        std::invoke(&QRadioButton::setChecked, instance ,true);
    #else
        instance->setChecked(true);
    #endif
}

/*!
 * \brief Dialog::getRadioButtonState Get the current state of the radio button
 * \param instance QRadioButton
 * \return
 */
bool Dialog::getRadioButtonState(QRadioButton *instance) const
{
    if (!instance) return false;
    #if __cplusplus >= 201703L
        return std::invoke(&QRadioButton::isChecked, instance );
    #else
        return instance->isChecked();
    #endif
}

/*!
 * \brief Dialog::loadSavedSettings Load all saved settings from the config file
 */
void Dialog::loadSavedSettings()
{
    qInfo() << Q_FUNC_INFO;
    QSettings settings(QString(qApp->applicationName()));
    settings.setPath(QSettings::IniFormat,QSettings::UserScope,QDir::currentPath());
    settings.beginGroup("InhaleExhale");
    setRadioButton(dptr->mapPosition[Modes::Inhale << 8 | settings.value("Position",Position::TopLeft).toInt()]);
    setRadioButton(dptr->mapShape[Modes::Inhale << 8 | settings.value("Shape",Shape::Ellipse).toInt()]);
    setRadioButton(dptr->mapDirection[Modes::Inhale << 8 | settings.value("Direction",Direction::Horizontal).toInt()]);
    dptr->mapTime.value(Modes::Inhale)->setValue(settings.value("timeInh", 0).toFloat() * MSEC_TO_SEC);
    dptr->mapTime.value(Modes::Exhale)->setValue(settings.value("timeExh", 0).toFloat() * MSEC_TO_SEC);
    on_ColorSelected(Modes::Inhale, QColor(settings.value("colorInh", "#ff00ff").toString()));
    on_ColorSelected(Modes::Exhale, QColor(settings.value("colorExh", "#ffff00").toString()));
    QStringList point = settings.value("scalingInh","1,1").toString().split(",");
//    dptr->userScaling[Modes::Inhale] = (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1);
    setUserScaling(Modes::Inhale, (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1));
    point = settings.value("scalingExh","1,1").toString().split(",");
//    dptr->userScaling[Modes::Exhale] = (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1);
    setUserScaling(Modes::Exhale, (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1));
    qInfo() << Q_FUNC_INFO << settings.value("scalingInh","1,1").toString()
            << settings.value("scalingExh","1,1").toString() << point
            << dptr->userScaling[Modes::Inhale] << dptr->userScaling[Modes::Exhale];

    settings.endGroup();

    settings.beginGroup("HoldInOut");
    setRadioButton(dptr->mapPosition[Modes::HoldIn << 8 | settings.value("Position",Position::TopLeft).toInt()]);
    setRadioButton(dptr->mapShape[Modes::HoldIn << 8 | settings.value("Shape",Shape::Ellipse).toInt()]);
    setRadioButton(dptr->mapDirection[Modes::HoldIn << 8 | settings.value("Direction",Direction::Vertical).toInt()]);
    dptr->mapTime.value(Modes::HoldIn)->setValue(settings.value("timeHoldIn", 0).toFloat()* MSEC_TO_SEC);
    dptr->mapTime.value(Modes::HoldOut)->setValue(settings.value("timeHoldOut", 0).toFloat()* MSEC_TO_SEC);

    on_ColorSelected(Modes::HoldIn, QColor(settings.value("colorHoldIn", "#00ffff").toString()));
    on_ColorSelected(Modes::HoldOut, QColor(settings.value("colorHoldOut", "#00ff00").toString()));
    point = settings.value("scalingHoldIn","1,1").toString().split(",");
//    dptr->userScaling[Modes::HoldIn] = (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1);
    setUserScaling(Modes::HoldIn, (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1));
    point = settings.value("scalingHoldOut","1,1").toString().split(",");
//    dptr->userScaling[Modes::HoldOut] = (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1);
    setUserScaling(Modes::HoldOut, (point.length() == 2) ? QPointF(point.at(0).toFloat(), point.at(1).toFloat()) : QPointF(1,1));
    settings.endGroup();

    settings.beginGroup("Global");
    dptr->transparancyShape  = settings.value("transparancyShape" ,"50").toDouble()  ;
    dptr->transparancyWindow = settings.value("transparancyWindow","50").toDouble() ;
    setShapeTransparency(dptr->transparancyShape);
    setWindowTransparency(dptr->transparancyWindow);
    settings.endGroup();

    qInfo() << Q_FUNC_INFO << settings.value("scalingHoldIn","1,1").toString()
            << settings.value("scalingHoldOut","1,1").toString() << point
             << dptr->userScaling[Modes::HoldIn] << dptr->userScaling[Modes::HoldOut];

    dptr->stateDirection[Modes::Inhale] = getDirection(Modes::Inhale);
    dptr->stateDirection[Modes::HoldIn] = getDirection(Modes::HoldIn);
    dptr->stateShape[Modes::Inhale]     = getShape(Modes::Inhale);
    dptr->stateShape[Modes::HoldIn]     = getShape(Modes::HoldIn);
    dptr->statePosition[Modes::Inhale]  = getPosition(Modes::Inhale);
    dptr->statePosition[Modes::HoldIn]  = getPosition(Modes::HoldIn);

    dptr->stateColor[Modes::Inhale]  = getColor(Modes::Inhale);
    dptr->stateColor[Modes::Exhale]  = getColor(Modes::Exhale);
    dptr->stateColor[Modes::HoldIn]  = getColor(Modes::HoldIn);
    dptr->stateColor[Modes::HoldOut] = getColor(Modes::HoldOut);

    dptr->stateTime[Modes::Inhale]  = getTimeMS(Modes::Inhale);
    dptr->stateTime[Modes::Exhale]  = getTimeMS(Modes::Exhale);
    dptr->stateTime[Modes::HoldIn]  = getTimeMS(Modes::HoldIn);
    dptr->stateTime[Modes::HoldOut] = getTimeMS(Modes::HoldOut);

    dptr->stateSize[Modes::Inhale]  = getUserScaling(Modes::Inhale);
    dptr->stateSize[Modes::Exhale]  = getUserScaling(Modes::Exhale);
    dptr->stateSize[Modes::HoldIn]  = getUserScaling(Modes::HoldIn);
    dptr->stateSize[Modes::HoldOut] = getUserScaling(Modes::HoldOut);

}

/*!
 * \brief Dialog::saveSettings Save settings in the config file
 */
void Dialog::saveSettings()
{
    qInfo() << Q_FUNC_INFO;
    QSettings settings(QString(qApp->applicationName()));
    settings.setPath(QSettings::IniFormat,QSettings::UserScope,QDir::currentPath());
    settings.beginGroup("InhaleExhale");
    settings.setValue("Position",getPosition(Modes::Inhale));
    settings.setValue("Shape",getShape(Modes::Inhale));
    settings.setValue("Direction",getDirection(Modes::Inhale));
    settings.setValue("timeInh",getTimeMS(Modes::Inhale));
    settings.setValue("timeExh",getTimeMS(Modes::Exhale));
    settings.setValue("colorInh",getColor(Modes::Inhale).name());
    settings.setValue("colorExh",getColor(Modes::Exhale).name());
    settings.setValue("scalingInh",QString::number(getUserScaling(Modes::Inhale).x())+","+QString::number(getUserScaling(Modes::Inhale).y()));
    settings.setValue("scalingExh",QString::number(getUserScaling(Modes::Exhale).x())+","+QString::number(getUserScaling(Modes::Exhale).y()));
    settings.endGroup();

    settings.beginGroup("HoldInOut");
    settings.setValue("Position",getPosition(Modes::HoldIn));
    settings.setValue("Shape",getShape(Modes::HoldIn));
    settings.setValue("Direction",getDirection(Modes::HoldIn));
    settings.setValue("timeHoldIn",getTimeMS(Modes::HoldIn));
    settings.setValue("timeHoldOut",getTimeMS(Modes::HoldOut));
    settings.setValue("colorHoldIn",getColor(Modes::HoldIn).name());
    settings.setValue("colorHoldOut",getColor(Modes::HoldOut).name());
    settings.setValue("scalingHoldIn",QString::number(getUserScaling(Modes::HoldIn).x())+","+QString::number(getUserScaling(Modes::HoldIn).y()));
    settings.setValue("scalingHoldOut",QString::number(getUserScaling(Modes::HoldOut).x())+","+QString::number(getUserScaling(Modes::HoldOut).y()));
    settings.endGroup();

    settings.beginGroup("Global");
    settings.setValue("transparancyShape", ui->transparancyShape->value()) ;
    settings.setValue("transparancyWindow", ui->transparancyWindow->value()) ;
    settings.endGroup();

    dptr->stateDirection[Modes::Inhale] = getDirection(Modes::Inhale);
    dptr->stateDirection[Modes::HoldIn] = getDirection(Modes::HoldIn);
    dptr->stateShape[Modes::Inhale]     = getShape(Modes::Inhale);
    dptr->stateShape[Modes::HoldIn]     = getShape(Modes::HoldIn);
    dptr->statePosition[Modes::Inhale]  = getPosition(Modes::Inhale);
    dptr->statePosition[Modes::HoldIn]  = getPosition(Modes::HoldIn);

    dptr->stateColor[Modes::Inhale]  = getColor(Modes::Inhale);
    dptr->stateColor[Modes::Exhale]  = getColor(Modes::Exhale);
    dptr->stateColor[Modes::HoldIn]  = getColor(Modes::HoldIn);
    dptr->stateColor[Modes::HoldOut] = getColor(Modes::HoldOut);

    dptr->stateTime[Modes::Inhale]  = getTimeMS(Modes::Inhale);
    dptr->stateTime[Modes::Exhale]  = getTimeMS(Modes::Exhale);
    dptr->stateTime[Modes::HoldIn]  = getTimeMS(Modes::HoldIn);
    dptr->stateTime[Modes::HoldOut] = getTimeMS(Modes::HoldOut);

    dptr->stateSize[Modes::Inhale]  = getUserScaling(Modes::Inhale);
    dptr->stateSize[Modes::Exhale]  = getUserScaling(Modes::Exhale);
    dptr->stateSize[Modes::HoldIn]  = getUserScaling(Modes::HoldIn);
    dptr->stateSize[Modes::HoldOut] = getUserScaling(Modes::HoldOut);

}

/*!
 * \brief Dialog::getPosition Get position of the parametered mode from UI
 * \param mode
 * \return
 */
quint8 Dialog::getPosition(quint8 mode)
{
    if (mode == Modes::Exhale) mode = Modes::Inhale;
    if (mode == Modes::HoldOut) mode = Modes::HoldIn;

    for (quint16 keys : dptr->mapPosition.keys())
        if ((keys >> 8) == mode && getRadioButtonState(dptr->mapPosition.value(keys)))
            return (keys & 0xFF) ; // extract Position byte from the key

    return 0;
}

/*!
 * \brief Dialog::getShape Get shape of the parametered mode from UI
 * \param mode
 * \return
 */
quint8 Dialog::getShape(quint8 mode)
{
    if (mode == Modes::Exhale) mode = Modes::Inhale;
    if (mode == Modes::HoldOut) mode = Modes::HoldIn;

    for (quint16 keys : dptr->mapShape.keys())
        if ((keys >> 8) == mode && getRadioButtonState(dptr->mapShape.value(keys)))
            return (keys & 0xFF) ; // extract Shape byte from the key
    return 0;
}

/*!
 * \brief Dialog::getDirection Get direction of the parametered mode from ui
 * \param mode
 * \return
 */
quint8 Dialog::getDirection(quint8 mode)
{
    if (mode == Modes::Exhale) mode = Modes::Inhale;
    if (mode == Modes::HoldOut) mode = Modes::HoldIn;

    for (quint16 keys : dptr->mapDirection.keys())
        if ((keys >> 8) == mode && getRadioButtonState(dptr->mapDirection.value(keys)))
            return (keys & 0xFF) ; // extract Direction byte from the key
    return 0;
}

/*!
 * \brief Dialog::getTimeMS Get time (ms) to show of the parametered mode from UI
 * \param mode
 * \return
 */
quint16 Dialog::getTimeMS(quint8 mode)
{
    if (dptr->mapTime.contains(mode))
        return (quint16)(dptr->mapTime.value(mode)->value()*SEC_TO_MSEC);
    return 0;
}

/*!
 * \brief Dialog::getColor Get color of the parametered mode
 * \param mode
 * \return
 */
QColor Dialog::getColor(quint8 mode)
{
    return dptr->colorMap[mode];
}

/*!
 * \brief Dialog::getShapeTransparency Get Shape transparancy from UI
 * \return
 */
quint8 Dialog::getShapeTransparency()
{
     return ui->transparancyShape->value();
}

/*!
 * \brief Dialog::setShapeTransparency Set Shape transparancy value in UI
 * \param transparancy
 */
void Dialog::setShapeTransparency(quint8 transparancy)
{
     ui->transparancyShape->setValue(transparancy);
}

/*!
 * \brief Dialog::getWindowTransparency Get Window transparancy from UI
 * \return
 */
quint8 Dialog::getWindowTransparency()
{
     return ui->transparancyWindow->value();
}

/*!
 * \brief Dialog::setWindowTransparency Set Window transparancy value in UI
 * \param transparancy
 */
void Dialog::setWindowTransparency(quint8 transparancy)
{
     ui->transparancyWindow->setValue(transparancy);
}

/*!
 * \brief Dialog::setUserScaling Set the scaling i.e. shape size multiplier selected by user in the UI
 * \param mode
 * \param scaling
 */
void Dialog::setUserScaling(quint8 mode, QPointF scaling)
{
    dptr->userScaling[mode] = scaling;
    if (mode == Modes::Exhale)  mode = Modes::Inhale;
    if (mode == Modes::HoldOut) mode = Modes::HoldIn;
    dptr->mapSize.value(mode << 8 | Direction::Horizontal)->setValue(scaling.x()*PERCENT_MULT);
    dptr->mapSize.value(mode << 8 | Direction::Vertical)->setValue(scaling.y()*PERCENT_MULT);
}

/*!
 * \brief Dialog::saveUserScaling Save the scaling i.e. shape size multiplier selected by user
 */
void Dialog::saveUserScaling()
{
    QSettings settings(QString(qApp->applicationName()));
    settings.beginGroup("InhaleExhale");

    settings.setValue("scalingInh",QString::number(getUserScaling(Modes::Inhale).x())+","+QString::number(getUserScaling(Modes::Inhale).y()));
    settings.setValue("scalingExh",QString::number(getUserScaling(Modes::Exhale).x())+","+QString::number(getUserScaling(Modes::Exhale).y()));
    settings.endGroup();

    settings.beginGroup("HoldInOut");
    settings.setValue("scalingHoldIn",QString::number(getUserScaling(Modes::HoldIn).x())+","+QString::number(getUserScaling(Modes::HoldIn).y()));
    settings.setValue("scalingHoldOut",QString::number(getUserScaling(Modes::HoldOut).x())+","+QString::number(getUserScaling(Modes::HoldOut).y()));
    settings.endGroup();
}

/*!
 * \brief Dialog::setPosition Set the radio button for the parametered mode in the UI
 * \param mode
 * \param position
 */
void Dialog::setPosition(quint8 mode, quint8 position)
{
    setRadioButton(dptr->mapPosition[mode << 8 | position]);
}

/*!
 * \brief Dialog::savePosition Save the position of shapes in the config file
 */
void Dialog::savePosition()
{
    qInfo() << Q_FUNC_INFO ;
    QSettings settings(QString(qApp->applicationName()));
    settings.beginGroup("InhaleExhale");

    settings.setValue("Position",getPosition(Modes::Inhale));
    settings.endGroup();

    settings.beginGroup("HoldInOut");
    settings.setValue("Position",getPosition(Modes::HoldIn));
    settings.endGroup();
}

/*!
 * \brief Dialog::getUserScaling Get user set scalin values from the UI
 * \param mode
 * \return
 */
QPointF Dialog::getUserScaling(quint8 mode)
{
    if (mode == Modes::Exhale)  mode = Modes::Inhale;
    if (mode == Modes::HoldOut) mode = Modes::HoldIn;

    QPointF scale ;
    scale.setX(((float)(dptr->mapSize.value(mode << 8 | Direction::Horizontal)->value())) * PERCENT_INV_MULT);
    scale.setY(((float)(dptr->mapSize.value(mode << 8 | Direction::Vertical)->value())) * PERCENT_INV_MULT);
    dptr->userScaling[mode] = scale;
    qInfo() << Q_FUNC_INFO << dptr->userScaling[mode];
    return dptr->userScaling[mode];
}

/*!
 * \brief Dialog::on_ColorSelected Unused Alternate method to set color for a mode
 * \param mode
 */
void Dialog::on_ColorSelected(int mode)
{
    dptr->colorMap[mode] = dptr->colord->selectedColor();
    if (mode == Modes::Inhale)
    {
        QColor coo = dptr->colorMap[Modes::Inhale];
        QString cols = colStart.arg(colStyleSheet.arg(coo.red()).arg(coo.green()).arg(coo.blue()));
    }
    dptr->mapper->removeMappings(dptr->colord);
}

/*!
 * \brief Dialog::on_ColorSelected SLOT triggers when color selected
 * \param color
 */
void Dialog::on_ColorSelected(QColor color)
{

    on_ColorSelected(dptr->currColorToSet, color);
    this->show();
}

/*!
 * \brief Dialog::on_ColorSelected Sets the color for the mode
 * \param mode
 * \param color
 */
void Dialog::on_ColorSelected(quint8 mode, QColor color)
{
    dptr->colorMap[mode] = color;
    QString cols = colStart.arg(colStyleSheet.arg(color.red()).arg(color.green()).arg(color.blue()));
    dptr->mapColor.value(mode)->setStyleSheet(cols);
    on_SomethingToggled();
}

/*!
 * \brief Dialog::on_ShapeTransparancyChanged
 * \param transparancy
 */
void Dialog::on_ShapeTransparancyChanged(int transparancy)
{
    on_SomethingToggled();
}

/*!
 * \brief Dialog::on_WindowTransparancyChanged
 * \param transparancy
 */
void Dialog::on_WindowTransparancyChanged(int transparancy)
{
    on_SomethingToggled();
}

/*!
 * \brief Dialog::on_ColorSelectClicked SLOT triggers User clicks on select color; Opens a dialog box to select it and saves the mode for which it was clicked
 * \param mode
 */
void Dialog::on_ColorSelectClicked(int mode)
{
    dptr->currColorToSet = mode;
    this->hide();
    dptr->colord->open();
}

/*!
 * \brief Dialog::on_ColorInhaleClicked Unused Alternate method to set color for a mode
 */
void Dialog::on_ColorInhaleClicked()
{
    dptr->mapper->setMapping(dptr->colord, (int)Modes::Inhale);
    connect(dptr->colord,SIGNAL(colorSelected(QColor)),dptr->mapper,SLOT(map()));
    connect(dptr->mapper,SIGNAL(mapped(int)),this,SLOT(on_ColorSelected(int)) );
    dptr->colord->open();
}

/*!
 * \brief Dialog::revertSettings When discard is pressed, revert all the settings to last saved ones
 */
void Dialog::revertSettings()
{
    setRadioButton(dptr->mapPosition[Modes::Inhale << 8  | dptr->statePosition[Modes::Inhale]]);
    setRadioButton(dptr->mapShape[Modes::Inhale << 8     | dptr->stateShape[Modes::Inhale]]);
    setRadioButton(dptr->mapDirection[Modes::Inhale << 8 | dptr->stateDirection[Modes::Inhale]]);

    setRadioButton(dptr->mapPosition[Modes::HoldIn << 8  | dptr->statePosition[Modes::HoldIn]]);
    setRadioButton(dptr->mapShape[Modes::HoldIn << 8     | dptr->stateShape[Modes::HoldIn]]);
    setRadioButton(dptr->mapDirection[Modes::HoldIn << 8 | dptr->stateDirection[Modes::HoldIn]]);


    on_ColorSelected(Modes::Inhale, QColor(dptr->stateColor[Modes::Inhale]));
    on_ColorSelected(Modes::Exhale, QColor(dptr->stateColor[Modes::Exhale]));

    on_ColorSelected(Modes::HoldIn, QColor(dptr->stateColor[Modes::HoldIn]));
    on_ColorSelected(Modes::HoldOut, QColor(dptr->stateColor[Modes::HoldOut]));


    dptr->mapTime.value(Modes::Inhale)->setValue(((float)dptr->stateTime[Modes::Inhale])*SEC_TO_MSEC);
    dptr->mapTime.value(Modes::Exhale)->setValue(((float)dptr->stateTime[Modes::Exhale])*SEC_TO_MSEC);

    dptr->mapTime.value(Modes::HoldIn)->setValue(((float)dptr->stateTime[Modes::HoldIn])*SEC_TO_MSEC);
    dptr->mapTime.value(Modes::HoldOut)->setValue(((float)dptr->stateTime[Modes::HoldOut])*SEC_TO_MSEC);


    setUserScaling(Modes::Inhale,dptr->stateSize[Modes::Inhale]);
    setUserScaling(Modes::HoldIn,dptr->stateSize[Modes::HoldIn]);

    setShapeTransparency(dptr->transparancyShape);
    setWindowTransparency(dptr->transparancyWindow);

}

/*!
 * \brief Dialog::on_ResetClicked SLOT fired when Reset clicked
 */
void Dialog::on_ResetClicked()
{
    qInfo() << Q_FUNC_INFO;
    setRadioButton(dptr->mapPosition[Modes::Inhale << 8 | Position::TopLeft]);
    setRadioButton(dptr->mapPosition[Modes::HoldIn << 8 | Position::TopLeft]);

    setRadioButton(dptr->mapShape[Modes::Inhale << 8 | Shape::Ellipse]);
    setRadioButton(dptr->mapShape[Modes::HoldIn << 8 | Shape::Ellipse]);

    setRadioButton(dptr->mapDirection[Modes::Inhale << 8 | Direction::Vertical ]);
    setRadioButton(dptr->mapDirection[Modes::HoldIn << 8 | Direction::Horizontal]);

    dptr->mapTime.value(Modes::Inhale)->setValue(DEF_INHALE_TIME);
    dptr->mapTime.value(Modes::Exhale)->setValue(DEF_INHALE_TIME);
    dptr->mapTime.value(Modes::HoldIn)->setValue(DEF_HOLD_TIME);
    dptr->mapTime.value(Modes::HoldOut)->setValue(DEF_HOLD_TIME);

    setShapeTransparency(50);
    setWindowTransparency(50);
}

/*!
 * \brief Dialog::on_DiscardClicked SLOT fired when Discard clicked
 */
void Dialog::on_DiscardClicked()
{
    qInfo() << Q_FUNC_INFO;
    revertSettings();
    this->hide();
    emit settingsChanged();
    emit settingsClosed();
}

/*!
 * \brief Dialog::on_SomethingToggled SLOT fired when any setting is changed so that MainWindow should know to update
 */
void Dialog::on_SomethingToggled()
{
    emit settingsChanged();
}

/*!
 * \brief Dialog::on_SaveClicked SLOT fired when Save clicked
 */
void Dialog::on_SaveClicked()
{
    qInfo() << Q_FUNC_INFO;
    this->hide();
    saveSettings();
    emit settingsChanged();
    emit settingsClosed();
}
