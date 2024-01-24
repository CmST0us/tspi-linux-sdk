#ifndef QTAUDIO_H
#define QTAUDIO_H

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>

class qtAudio : public QWidget
{
    Q_OBJECT

public:
    qtAudio(QWidget *parent = 0);
    ~qtAudio();

    QListWidget *listWidget;
    QVBoxLayout *vLayout;
    QSlider *slider;
private slots:
    void on_itemClicked(QListWidgetItem *item);
    void changeVolume(int value);
};

#endif // QTAUDIO_H
