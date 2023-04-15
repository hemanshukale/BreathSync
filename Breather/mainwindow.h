#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMetaEnum>

class Mode;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct MainData;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    enum Focus : quint8
    {
        NoFocus = 0,
        InhaleExhale,
        HoldInOut,
    };
    Q_ENUM(Focus);

private:
    Ui::MainWindow *ui;
    typedef QMainWindow inherited;
    MainData *dptr; // DPointer style of coding
    void drawShape(QPainter &qp, QRect xywh, quint8 shape);

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void timerEvent(QTimerEvent *event);

    void setFocusedModesScaling(qint8 scrollX, qint8 scollY);


    QSize sizeHint() const;
    void keyPressEvent(QKeyEvent *event); //override
    void getNextFocus();
    void setFocusedModesPosition(quint8 position);
    bool isModeInFocus(quint8 mode, quint8 focus);

    QMetaEnum enumFocus  = QMetaEnum::fromType<Focus>();

 private slots:
    void onModeTimeout();
    void showWindow();
    void updateSettings();

};
#endif // MAINWINDOW_H
