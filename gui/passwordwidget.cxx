#include "passwordwidget.h"
#include "ui_passwordwidget.h"
#include "settings.h"
#include <QDebug>

passwordWidget::passwordWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::passwordWidget),
    mCopies(0)
{
    ui->setupUi(this);
    if(parent) {
        QRect parentRect = parent->rect();
        setGeometry(parentRect);
    }

    loadSettings();
}

passwordWidget::passwordWidget(QString filename, int copies, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::passwordWidget),
    mFilename(filename),
    mCopies(copies)
{
    ui->setupUi(this);
    if(parent) {
        QRect parentRect = parent->rect();
        setGeometry(parentRect);
    }

    loadSettings();
}

void passwordWidget::loadSettings(void) {
    pbSettings &pbs = pbSettings::getInstance();
    mDigits.clear();
    mPassword.clear();
    mPassword.append(pbs.getInt("archive", "password1"));
    mPassword.append(pbs.getInt("archive", "password2"));
    mPassword.append(pbs.getInt("archive", "password3"));
    mPassword.append(pbs.getInt("archive", "password4"));
    mCurrentDigit = 0;
}

passwordWidget::~passwordWidget()
{
    delete ui;
}

void passwordWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}

void passwordWidget::on_btnNum1_clicked()
{
    mDigits.append(1);
    checkPassword();
}

void passwordWidget::on_btnNum2_clicked()
{
    mDigits.append(2);
    checkPassword();
}

void passwordWidget::on_btnNum3_clicked()
{
    mDigits.append(3);
    checkPassword();
}

void passwordWidget::on_btnNum4_clicked()
{
    mDigits.append(4);
    checkPassword();
}

void passwordWidget::on_btnNum5_clicked()
{
    mDigits.append(5);
    checkPassword();
}

void passwordWidget::on_btnNum6_clicked()
{
    mDigits.append(6);
    checkPassword();
}

void passwordWidget::on_btnNum7_clicked()
{
    mDigits.append(7);
    checkPassword();
}

void passwordWidget::on_btnNum8_clicked()
{
    mDigits.append(8);
    checkPassword();
}

void passwordWidget::on_btnNum9_clicked()
{
    mDigits.append(9);
    checkPassword();
}

void passwordWidget::on_btnNum0_clicked()
{
    mDigits.append(0);
    checkPassword();
}

void passwordWidget::on_btnCancel_clicked()
{
    emit cancelled();
}

void passwordWidget::checkPassword()
{
    qDebug() << "checkPassword";
    if(mDigits.length() != mPassword.length())
        return;
    for(int i=0; i<mPassword.length(); i++) {
        if(mPassword.at(i) != mDigits.at(i))
            return;
    }

    qDebug() << "matched.";
    // Passwords match!
    if(!mFilename.isEmpty())
        emit printArchivePicture(mFilename, mCopies);
    else
        emit printPicture();
}
