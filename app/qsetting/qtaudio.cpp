#include "qtaudio.h"
#include <QApplication>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <QDesktopWidget>

qtAudio::qtAudio(QWidget *parent)
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(parent);
    resize(availableGeometry.width(), availableGeometry.height());
    QFont f;
    f.setPixelSize(availableGeometry.height()/20);
    listWidget = new QListWidget();

    QList<QAudioDeviceInfo> alist = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo defaultinfo(QAudioDeviceInfo::defaultOutputDevice());
    qDebug() << "default device name: " << defaultinfo.deviceName();
    for(auto info : alist){
        QListWidgetItem *audioItem = new QListWidgetItem();
        audioItem->setText(info.deviceName());
        audioItem->setFont(f);
        audioItem->setFlags(audioItem->flags() | Qt::ItemIsUserCheckable);
        if(defaultinfo == info){
            audioItem->setCheckState(Qt::Checked);
        } else {
            audioItem->setCheckState(Qt::Unchecked);
        }
        qDebug() << info.deviceName();
        listWidget->addItem(audioItem);
    }
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setMinimumHeight(listWidget->sizeHintForRow(0) * listWidget->count());
    listWidget->setMaximumHeight(listWidget->sizeHintForRow(0) * listWidget->count());

    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(10);
    QAudioOutput out(defaultinfo);
    qreal v = out.volume();
    slider->setValue(qRound(v*10));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeVolume(int)));

    vLayout = new QVBoxLayout();
    vLayout->addWidget(listWidget);
    vLayout->addWidget(slider);
    vLayout->setAlignment(Qt::AlignTop);

    setLayout(vLayout);
    setStyleSheet("background-color:rgb(204,228,247)");
    setObjectName("Audio");
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(on_itemClicked(QListWidgetItem *)));
}

qtAudio::~qtAudio()
{
    listWidget->clear();
    delete listWidget;
    delete slider;
    delete vLayout;
}

void qtAudio::on_itemClicked(QListWidgetItem *item)
{
    qDebug() << item->text();

}

void qtAudio::changeVolume(int value)
{
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    QAudioOutput out(info);
    qreal linearVolume = QAudio::convertVolume(value / qreal(100.0),
                                               QAudio::LinearVolumeScale,
                                               QAudio::LogarithmicVolumeScale);
    qDebug() << value;
    qreal v = qreal(value) / qreal(10.0);
    qDebug() << v;
    out.setVolume(v);
    QApplication::beep();
}
