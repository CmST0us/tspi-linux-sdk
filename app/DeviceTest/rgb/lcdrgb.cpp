#include "lcdrgb.h"

LcdRGB::LcdRGB(QWidget *parent) : QWidget(parent)
{

}

void LcdRGB::initWindow(int w, int h)
{
//    widget = common.getWidget();
    QHBoxLayout *layout = new QHBoxLayout();
    QVBoxLayout *layout_v = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout_v->setSpacing(0);
    layout_v->setMargin(0);
    this->setContentsMargins(0,0,0,0);

    lable = common.getLabel("检查屏幕颜色是否正常,是否漏光!");
    lable->resize(w, h);
    lable->setAlignment(Qt::AlignCenter);
    passBtn = common.getButton("测试成功");
    passBtn->setVisible(false);
    faildBtn = common.getButton("测试失败");
    faildBtn->setStyleSheet(faildBtn->styleSheet() + "QPushButton{background-color:#f56c6c}");
    faildBtn->setVisible(false);
    connect(passBtn, &QPushButton::clicked, [=] {closeWindow(true);});
    connect(faildBtn, &QPushButton::clicked, [=] {closeWindow(false);});

    layout_v->addWidget(lable);
    layout->setAlignment(lable, Qt::AlignCenter);
    layout->addWidget(passBtn);
    layout->addWidget(faildBtn);
    layout_v->addLayout(layout);

    this->setLayout(layout_v);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);
    lable->setAttribute(Qt::WA_AcceptTouchEvents);
    lable->installEventFilter(this);
}

bool LcdRGB::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == lable){
        switch (event->type()){
            case QEvent::TouchBegin:
                return changeBGColor();

            default:
                return false;
            }
    }
    return false;
}


bool LcdRGB::changeBGColor()
{
    if( clolorIndex > 5 ){
        passBtn->setVisible(true);
        faildBtn->setVisible(true);
        return true;
    }
    lable->setStyleSheet(rgb[clolorIndex]);
    clolorIndex ++ ;
    return true;
}

void LcdRGB::closeWindow(bool result)
{
    testResult = result;
    //update config file state
    common.setConfig("RGB", "state", QString::number(result));

    emit testFinish();
    this->close();

}
