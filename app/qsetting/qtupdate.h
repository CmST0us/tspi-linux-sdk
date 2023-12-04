#ifndef QTUPDATE_H
#define QTUPDATE_H

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class qtUpdate : public QWidget
{
    Q_OBJECT

public:
    qtUpdate(QWidget *parent = 0);
    ~qtUpdate();

    QPushButton btn;
    QLabel label;
    QVBoxLayout vLayout;
private slots:
    void on_btnClicked();
};

#endif // QTUPDATE_H
