#include "qtmagnifier.h"

QTMagnifier::QTMagnifier(QWidget *parent, QApplication *app)
    : QWidget(parent)
{
    this->app = app;

    settings = new QSettings(R"(./settings.ini)", QSettings::IniFormat);
    uiScale = settings->value("ui_scale", 1.0f).toFloat();
    zoomFactor = settings->value("zoom_factor", 2.0f).toFloat();
    frameWidth = settings->value("frame_width", 5.0f).toFloat();
    windowX = settings->value("window_x", 0).toInt();
    windowY = settings->value("window_y", 0).toInt();
    windowWidth = settings->value("window_width", 600).toInt();
    windowHeight = settings->value("window_height", 200).toInt();
    refreshInterval = settings->value("refresh_interval_ms", 17).toInt();

    this->setGeometry(windowX, windowY, windowWidth, windowHeight);
    this->setMouseTracking(true);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QTMagnifier::update));
    timer->start(refreshInterval);

    dragging = false;
    resizingLeft = false;
    resizingRight = false;
    resizingTop = false;
    resizingBottom = false;
    constrainToScreenBorder = false;

    this->setStyleSheet("background-color:gray;");
}

void QTMagnifier::paintEvent(QPaintEvent *event)
{
    QPoint mousePos = QCursor::pos();
    QScreen *currentScreen = app->screenAt(mousePos);

    if (currentScreen == nullptr)
        return;

    QRect currentScreenGeometry = currentScreen->geometry();
    QPoint localCursorPos = mousePos - currentScreenGeometry.topLeft();

    float x = mousePos.x() - ((this->width() - (frameWidth * 2.0f)) / (zoomFactor * 2.0f));
    float y = mousePos.y() - ((this->height() - (frameWidth * 2.0f)) / (zoomFactor * 2.0f));
    float w = (this->width() - (frameWidth * 2.0f)) / zoomFactor;
    float h = (this->height() - (frameWidth * 2.0f)) / zoomFactor;

    if (localCursorPos.x() < w / 2.0f)
        x = currentScreenGeometry.x();

    if (localCursorPos.y() < h / 2.0f)
        y = currentScreenGeometry.y();

    if (localCursorPos.x() > currentScreenGeometry.width() - (w / 2.0f))
        x = currentScreenGeometry.x() + currentScreenGeometry.width() - w;

    if (localCursorPos.y() > currentScreenGeometry.height() - (h / 2.0f))
        y = currentScreenGeometry.y() + currentScreenGeometry.height() - h;

    QRectF magnifyArea(x, y, w, h);

    QPixmap screenshot = app->primaryScreen()->grabWindow(0, magnifyArea.x(), magnifyArea.y(),
                                                          magnifyArea.width(), magnifyArea.height());

    QRectF inter = magnifyArea.intersected(
                QRectF(this->geometry().x() + frameWidth, this->geometry().y() + frameWidth,
                       this->geometry().width() - (frameWidth * 2.0f),
                       this->geometry().height() - (frameWidth * 2.0f)));
    QPixmap blackMap(inter.width(), inter.height());
    blackMap.fill(Qt::black);
    QRectF interSrc(0.0f, 0.0f, inter.width(), inter.height());
    QRectF interTarget((inter.x() - magnifyArea.x()) * zoomFactor + frameWidth,
                       (inter.y() - magnifyArea.y()) * zoomFactor + frameWidth,
                       inter.width() * zoomFactor,
                       inter.height() * zoomFactor);

    QRectF src(0.0, 0.0, magnifyArea.width(), magnifyArea.height());
    QRectF target(frameWidth, frameWidth, this->width() - (frameWidth * 2.0f),
                  this->height() - (frameWidth * 2.0f));

    QPainter painter(this);
    painter.drawPixmap(target, screenshot, src);
    painter.drawPixmap(interTarget, blackMap, interSrc);

    int xHotspot = 0, yHotspot = 0;
    QPixmap cursorIcon = getMouseCursorIcon(&xHotspot, &yHotspot);

    QRectF cursorSrc(0.0f, 0.0f, cursorIcon.width(), cursorIcon.height());
    QRectF cursorTarget((mousePos.x() - xHotspot - magnifyArea.x()) * zoomFactor + frameWidth,
                        (mousePos.y() - yHotspot - magnifyArea.y()) * zoomFactor + frameWidth,
                        cursorIcon.width() / uiScale * zoomFactor,
                        cursorIcon.height() / uiScale * zoomFactor);

    painter.drawPixmap(cursorTarget, cursorIcon, cursorSrc);
}

void QTMagnifier::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        constrainToScreenBorder = true;

    if (event->button() == Qt::LeftButton)
    {
        startingPos = event->globalPos();
        QPointF localPos = event->localPos();

        dragging = true;

        if (cursor.shape() == Qt::OpenHandCursor)
            cursor.setShape(Qt::ClosedHandCursor);

        if (localPos.x() >= 0 && localPos.x() < frameWidth)
        {
            resizingLeft = true;
            dragging = false;
        }
        if (localPos.x() <= this->geometry().width() && localPos.x() > this->geometry().width() - frameWidth)
        {
            resizingRight = true;
            dragging = false;
        }
        if (localPos.y() >= 0 && localPos.y() < frameWidth)
        {
            resizingTop = true;
            dragging = false;
        }
        if (localPos.y() <= this->geometry().height() && localPos.y() > this->geometry().height() - frameWidth)
        {
            resizingBottom = true;
            dragging = false;
        }

        this->setCursor(cursor);
    }
}

void QTMagnifier::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        constrainToScreenBorder = false;

    if (event->button() == Qt::LeftButton)
    {
        if (cursor.shape() == Qt::ClosedHandCursor)
            cursor.setShape(Qt::OpenHandCursor);

        dragging = false;
        resizingLeft = false;
        resizingRight = false;
        resizingTop = false;
        resizingBottom = false;

        this->setCursor(cursor);
    }
}

void QTMagnifier::mouseMoveEvent(QMouseEvent *event)
{
    QScreen *currentScreen = app->screenAt(this->geometry().center());

    QPoint delta = event->globalPos() - startingPos;
    startingPos = event->globalPos();
    QPointF localPos = event->localPos();

    cursor.setShape(Qt::OpenHandCursor);

    int i = 0;
    if (localPos.x() >= 0 && localPos.x() < frameWidth)
        i |= 1;
    if (localPos.x() <= this->geometry().width() &&
            localPos.x() > this->geometry().width() - frameWidth)
        i |= 2;
    if (localPos.y() >= 0 && localPos.y() < frameWidth)
        i |= 4;
    if (localPos.y() <= this->geometry().height() &&
            localPos.y() > this->geometry().height() - frameWidth)
        i |= 8;

    if (i == 1 || i == 2)
        cursor.setShape(Qt::SizeHorCursor);
    if (i == 4 || i == 8)
        cursor.setShape(Qt::SizeVerCursor);
    if (i == 5 || i == 10 )
        cursor.setShape(Qt::SizeFDiagCursor);
    if (i == 6 || i == 9 )
        cursor.setShape(Qt::SizeBDiagCursor);

    if (dragging)
    {
        cursor.setShape(Qt::ClosedHandCursor);

        this->setGeometry(this->geometry().x() + delta.x(), this->geometry().y() + delta.y(),
                          this->geometry().width(), this->geometry().height());
    }

    if (resizingLeft)
    {
        cursor.setShape(Qt::SizeHorCursor);

        this->setGeometry(this->geometry().x() + delta.x(), this->geometry().y(),
                          this->geometry().width() - delta.x(), this->geometry().height());
    }
    if (resizingRight)
    {
        cursor.setShape(Qt::SizeHorCursor);

        this->setGeometry(this->geometry().x(), this->geometry().y(),
                          this->geometry().width() + delta.x(), this->geometry().height());
    }
    if (resizingTop)
    {
        cursor.setShape(Qt::SizeVerCursor);

        this->setGeometry(this->geometry().x(), this->geometry().y() + delta.y(),
                          this->geometry().width(), this->geometry().height() - delta.y());
    }
    if (resizingBottom)
    {
        cursor.setShape(Qt::SizeVerCursor);

        this->setGeometry(this->geometry().x(), this->geometry().y(),
                          this->geometry().width(), this->geometry().height() + delta.y());
    }

    if ((resizingLeft && resizingTop) || (resizingRight && resizingBottom))
        cursor.setShape(Qt::SizeFDiagCursor);
    if ((resizingRight && resizingTop) || (resizingLeft && resizingBottom))
        cursor.setShape(Qt::SizeBDiagCursor);

    this->setCursor(cursor);

    if (!constrainToScreenBorder)
        return;

    if (this->geometry().x() < currentScreen->geometry().x())
    {
        this->setGeometry(currentScreen->geometry().x(), this->geometry().y(),
                          this->geometry().width(),
                          this->geometry().height());
    }

    if (this->geometry().y() < currentScreen->geometry().y())
    {
        this->setGeometry(this->geometry().x(), currentScreen->geometry().y(),
                          this->geometry().width(),
                          this->geometry().height());
    }

    if (this->geometry().right() > currentScreen->geometry().right())
    {
        if (dragging)
        {
            this->setGeometry(currentScreen->geometry().right() - this->geometry().width(),
                            this->geometry().y(),
                            this->geometry().width(),
                            this->geometry().height());
        }
        else if (resizingLeft || resizingRight || resizingTop || resizingBottom)
        {
            this->setGeometry(this->geometry().x(),
                            this->geometry().y(),
                            currentScreen->geometry().right() - this->geometry().x(),
                            this->geometry().height());
        }
    }

    if (this->geometry().bottom() > currentScreen->geometry().bottom())
    {
        if (dragging)
        {
            this->setGeometry(this->geometry().x(),
                              currentScreen->geometry().bottom() - this->geometry().height(),
                              this->geometry().width(),
                              this->geometry().height());
        }
        else if (resizingLeft || resizingRight || resizingTop || resizingBottom)
        {
            this->setGeometry(this->geometry().x(),
                            this->geometry().y(),
                            this->geometry().width(),
                            currentScreen->geometry().bottom() - this->geometry().y());
        }
    }
}

QPixmap QTMagnifier::getMouseCursorIcon(int *xHot, int *yHot)
{
    int cursorWidth = GetSystemMetrics(SM_CXCURSOR);
    int cursorHeight = GetSystemMetrics(SM_CYCURSOR);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    HBITMAP hbmCanvas = CreateCompatibleBitmap(hdcScreen, cursorWidth, cursorHeight);

    HGDIOBJ hbmOld = SelectObject(hdcMem, hbmCanvas);

    CURSORINFO ci;
    ci.cbSize = sizeof(ci);
    GetCursorInfo(&ci);

    PICONINFO icInfo = new ICONINFO();
    HICON hicon = nullptr;
    if (ci.hCursor != nullptr)
    {
        hicon = CopyIcon(ci.hCursor);
        bool success = GetIconInfo(hicon, icInfo);
        if (success)
        {
            *xHot = icInfo->xHotspot;
            *yHot = icInfo->yHotspot;
        }
    }

    DeleteObject(icInfo->hbmColor);
    DeleteObject(icInfo->hbmMask);
    delete icInfo;
    DestroyIcon(hicon);

    DrawIcon(hdcMem, 0, 0, ci.hCursor);

    QPixmap cursorPixmap = QtWin::fromHBITMAP(hbmCanvas, QtWin::HBitmapAlpha);

    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmCanvas);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return cursorPixmap;
}

QTMagnifier::~QTMagnifier()
{
    settings->setValue("ui_scale", QString::number(uiScale));
    settings->setValue("zoom_factor", QString::number(zoomFactor));
    settings->setValue("frame_width", QString::number(frameWidth));
    settings->setValue("window_x", this->geometry().x());
    settings->setValue("window_y", this->geometry().y());
    settings->setValue("window_width", this->geometry().width());
    settings->setValue("window_height", this->geometry().height());
    settings->setValue("refresh_interval_ms", refreshInterval);
    settings->sync();

    delete settings;
    delete timer;
}

