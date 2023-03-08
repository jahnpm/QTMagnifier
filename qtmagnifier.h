#ifndef QTMAGNIFIER_H
#define QTMAGNIFIER_H

#include <QWidget>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QtGui>
#include <QtDebug>
#include <QtWinExtras>
#include <Windows.h>

class QTMagnifier : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    QPixmap getMouseCursorIcon(int *xHot, int *yHot);

    QSettings *settings;
    float zoomFactor, uiScale, frameWidth;
    int windowX, windowY, windowWidth, windowHeight;
    int refreshInterval;

    QApplication *app;
    QTimer *timer;

    QPoint startingPos;
    bool dragging;
    bool resizingLeft;
    bool resizingRight;
    bool resizingTop;
    bool resizingBottom;
    bool constrainToScreenBorder;

    QCursor cursor;
public:
    QTMagnifier(QWidget *parent = nullptr, QApplication *app = nullptr);
    ~QTMagnifier();
};
#endif // QTMAGNIFIER_H
