#include "qtinputdialog.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

inputDialog* inputDialog::_instance = nullptr;
inputDialog::inputDialog(QWidget *parent) : QDialog(parent),
  m_eventLoop(nullptr)
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry();
    QVBoxLayout *mainLayout = new QVBoxLayout;

    QFont font;
    QFont fontname;
    QFont fontedit;
    QFont fontbutton;
    font.setPixelSize(availableGeometry.height()/20);
    fontname.setPixelSize(availableGeometry.height()/30);
    fontedit.setPixelSize(availableGeometry.height()/15);
    fontbutton.setPixelSize(availableGeometry.height()/25);
    nameLabel = new QLabel(this);
    nameLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    nameLabel->setFont(fontname);
    wordEdit = new QLineEdit(this);
    wordEdit->setFont(fontedit);
    yBtn.setFont(fontbutton);
    nBtn.setFont(fontbutton);

    QHBoxLayout *buttonlayout = new QHBoxLayout;
    buttonlayout->addWidget(&yBtn);
    buttonlayout->addWidget(&nBtn);

    buttonlayout->setStretchFactor(&yBtn,1);
    buttonlayout->setStretchFactor(&nBtn,1);

    mainLayout->addWidget(nameLabel);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(wordEdit);
    mainLayout->addStretch(0);
    mainLayout->addLayout(buttonlayout);
    resize(availableGeometry.width()/3, availableGeometry.height() * 3/20);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
//    setWindowFlags(Qt::FramelessWindowHint);
    setLayout(mainLayout);

    connect(&yBtn, SIGNAL(clicked()), this, SLOT(slot_onYesClicked()));
    connect(&nBtn, SIGNAL(clicked()), this, SLOT(slot_onNoClicked()));
}

inputDialog::~inputDialog()
{
    if(m_eventLoop != nullptr)
        m_eventLoop->exit();
    if(wordEdit != nullptr)
        delete wordEdit;
    if(nameLabel != nullptr)
        delete nameLabel;
    _instance = nullptr;
}

void inputDialog::setText(QString yes, QString no, QString text)
{
        yBtn.setText(yes);
        nBtn.setText(no);
        nameLabel->setText(text);
}

int inputDialog::exec()
{
    if(m_eventLoop != nullptr)
        m_eventLoop->exit();
    setWindowModality(Qt::WindowModal);
    show();
    m_eventLoop = new QEventLoop(this);
    m_eventLoop->exec();
    return m_chooseResult;
}

void inputDialog::exit(bool result)
{
    if(m_eventLoop != nullptr) {
        m_chooseResult = result;
        close();
    }
}

bool inputDialog::isRunning(void)
{
    if(m_eventLoop != nullptr)
        return m_eventLoop->isRunning();
    return false;
}

void inputDialog::slot_onApplicationFocusChanged(QWidget *, QWidget *nowWidget)
{
    if (nowWidget != nullptr && !isAncestorOf(nowWidget)) {
        if (nowWidget->objectName().compare(parent()->objectName())) {
            setVisible(true);
        } else {
            setVisible(false);
        }
    }
}

void inputDialog::slot_onYesClicked()
{
    m_chooseResult = true;
    close();
}

void inputDialog::slot_onNoClicked()
{
    m_chooseResult = false;
    close();
}

void inputDialog::closeEvent(QCloseEvent*)
{
    if(m_eventLoop != nullptr)
        m_eventLoop->exit();
}
