#ifndef TOUCH_H
#define TOUCH_H

#include <QObject>
#include <QTimer>
#include <QPainter>
#include <QEvent>
#include <QTouchEvent>
#include <QDebug>
#include <QPainterPath>
#include "common/common.h"

class Touch:public QWidget
{
    Q_OBJECT
public:
    explicit Touch(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showPoint();
    void paintEvent(QPaintEvent *) override;

    Common common;
    QPushButton *passBtn;
    QPushButton *failBtn;
    QLabel *point;
    QPainterPath path;
    int x=0, y=0, x1=0, y1=0;

protected slots:
    bool handleTouchBegin(QEvent *event);
    bool handleTouchUpdate(QEvent *event);
    bool handleTouchEnd(QEvent *event);
    void handleClick(bool result);

signals:
    void testFinish();
};

#endif // TOUCH_H
