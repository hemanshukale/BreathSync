#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QDialog>
#include <QObject>
#include <QRadioButton>
#include <QColor>
#include <QPointF>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

struct DialogData;
class Dialog: public QDialog
{
    Q_OBJECT

private :
    Ui::Dialog *ui;
    DialogData *dptr;
    typedef QDialog inherited;
    void loadSavedSettings();
    void saveSettings();
//    typedef void (*radioSet)(QRadioButton *);

    void setRadioButton(QRadioButton *instance);
    bool getRadioButtonState(QRadioButton *instance) const;

    void setHashMapping();
    const QString colStart = "* { background-color: rgb%1; }";
    const QString colStyleSheet = "(%1, %2, %3)";
    void on_ColorSelected(quint8, QColor);
    void revertSettings();

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    quint8 getPosition(quint8 mode);
    quint8 getShape(quint8 mode);
    quint8 getDirection(quint8 mode);
    quint16 getTimeMS(quint8 mode);
    QColor getColor(quint8 mode);
    QPointF getUserScaling(quint8 mode);
    void setUserScaling(quint8 mode, QPointF scaling);
    void setPosition(quint8 mode, quint8 position);
    void savePosition();
    void saveUserScaling();

private slots:
    void on_SaveClicked();
    void on_ResetClicked();
    void on_DiscardClicked();
    void on_ColorInhaleClicked();
    void on_ColorSelected(int);
    void on_ColorSelected(QColor);
    void on_ColorSelectClicked(int);
    void on_SomethingToggled();

signals:
    void settingsChanged();
    void settingsClosed();

};

#endif // DIALOG_H
