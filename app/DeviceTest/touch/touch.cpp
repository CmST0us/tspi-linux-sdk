#include "touch.h"

Touch::Touch(QWidget *parent): QWidget(parent)
{
}

void Touch::initWindow(int w, int h)
{
    point = common.getLabel("当前坐标:");
    point->setAlignment(Qt::AlignCenter);
    passBtn = common.getButton("测试通过");
    failBtn = common.getButton("测试失败");
    failBtn->setStyleSheet(failBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(passBtn, &QPushButton::clicked, [=] {this->handleClick(true);});
    connect(failBtn, &QPushButton::clicked, [=] {this->handleClick(false);});

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);


    layout->addWidget(point, Qt::AlignCenter);
    hlayout->addWidget(passBtn, Qt::AlignBottom);
    hlayout->addWidget(failBtn, Qt::AlignBottom);

    layout->addLayout(hlayout);

    this->setLayout(layout);
    this->resize(w, h);
    point->setAttribute(Qt::WA_AcceptTouchEvents);
    point->installEventFilter(this);
    point->resize(w,h);
    point->setContentsMargins(0,0,0,0);
    point->setMargin(0);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    point->setStyleSheet("QLabel{padding:0px;}");
}

bool Touch::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == point){
        switch (event->type()){
            case QEvent::TouchBegin:
                return handleTouchBegin(event);

            case QEvent::TouchUpdate:
                return handleTouchUpdate(event);

            case QEvent::TouchEnd:
                return handleTouchEnd(event);

            default:
                return false;
            }
    }
    return false;
}

bool Touch::handleTouchBegin(QEvent *event)
{
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    QList<QTouchEvent::TouchPoint> touchStartPoints = touchEvent->touchPoints();
    QPoint startPoint = touchStartPoints.at(0).screenPos().toPoint();
    x = startPoint.x();
    y = startPoint.y();
    point->setText("当前坐标:"+QString::number(x)+","+QString::number(y));
    point->update();
    path.moveTo(x,y);
    return true;
}

bool Touch::handleTouchUpdate(QEvent *event)
{
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    QList<QTouchEvent::TouchPoint> touchStartPoints = touchEvent->touchPoints();
    QPoint startPoint = touchStartPoints.at(0).screenPos().toPoint();
    x1 = startPoint.x();
    y1 = startPoint.y();
    point->setText("当前坐标:"+QString::number(x1)+","+QString::number(y1));
    point->update();
    path.lineTo(x1,y1);
    return true;
}

bool Touch::handleTouchEnd(QEvent *event)
{
    point->setText("当前坐标:");

    //clean old path
    //path.clear();
    return true;
}

void Touch::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.drawPath(path);
}

void Touch::handleClick(bool result)
{
    testResult = result;
    //update config file state
    common.setConfig("tp", "state", QString::number(result));
    this->close();
    emit testFinish();
}
