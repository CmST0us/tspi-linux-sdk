#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QObject>
#include <QWidget>

class BaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWidget(QWidget *parent = 0);
    void setBackgroundColor(int rValue, int gValue, int bValue);
    static void setWidgetFontBold(QWidget *widget);
    static void setWidgetFontSize(QWidget *widget, int size);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent *);
};
#endif // BASEWIDGET_H
