#include "basewidget.h"
#include <QStyleOption>
#include <QPainter>

static int _id_widget = 0;

BaseWidget::BaseWidget(QWidget *parent) : QWidget(parent)
{
#ifndef DEVICE_EVB
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif
    setMouseTracking(true);
}

void BaseWidget::paintEvent(QPaintEvent *)
{
    /* Slove the problem which 'setStyleSheet' and 'Q_OBJECT' can co-exist
       The below code used to repaint widgets when change became. */
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void BaseWidget::setBackgroundColor(int rValue, int gValue, int bValue)
{
    _id_widget++;
    QString styleStr;

    setObjectName(QString::number(_id_widget));
    styleStr.append("#").append(QString::number(_id_widget)).append("{background-color:rgb(")
            .append(QString::number(rValue)).append(",")
            .append(QString::number(gValue)).append(",")
            .append(QString::number(bValue)).append(");")
            .append("}");

    setStyleSheet(styleStr);
}

void BaseWidget::setWidgetFontBold(QWidget *widget)
{
    QFont font = widget->font();
    font.setBold(true);
    widget->setFont(font);
}

void BaseWidget::setWidgetFontSize(QWidget *widget, int size)
{
    QFont font = widget->font();
    font.setPixelSize(size);
    widget->setFont(font);
}

void BaseWidget::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
}

void BaseWidget::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
}

void BaseWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
}
