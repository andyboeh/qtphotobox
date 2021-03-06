#include "archivewidget.h"
#include "ui_archivewidget.h"
#include "iconlistmodel.h"
#include "storageManager.h"
#include "settings.h"
#include "statemachine.h"
#include "picturedetailwidget.h"
#include "passwordwidget.h"
#include <QDebug>
#include <QDir>

archiveWidget::archiveWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::archiveWidget)
{
    ui->setupUi(this);
    storageManager &sm = storageManager::getInstance();
    mBasepath = sm.getThumbnailStoragePath();
    mStoragepath = sm.getPictureStoragePath();

    pbSettings &pbs = pbSettings::getInstance();
    QString basename = pbs.get("storage", "basename");
    mBasename = basename + "-assembled-";

    QStringList filters;
    filters.append(mBasename + "*");

    iconListModel *model = new iconListModel(this);
    model->setDirPath(mBasepath, filters, mBasename);
    ui->listView->setModel(model);
    ui->listView->setViewMode(QListView::IconMode);
    ui->listView->setIconSize(QSize(150,150));

    mDetailWidget = nullptr;
    mPasswordWidget = nullptr;
}

archiveWidget::~archiveWidget()
{
    delete mDetailWidget;
    delete mPasswordWidget;
    delete ui;
}

void archiveWidget::on_btnClose_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("idle");
}

void archiveWidget::printArchivePicture(QString filename, int copies)
{
    pbSettings &pbs = pbSettings::getInstance();
    if(pbs.getBool("archive", "printingpassword")) {
        delete mPasswordWidget;
        mPasswordWidget = new passwordWidget(filename, copies, this);
        connect(mPasswordWidget, SIGNAL(printArchivePicture(QString,int)), this, SLOT(printFromPasswordDialog(QString,int)));
        connect(mPasswordWidget, SIGNAL(cancelled()), this, SLOT(passwordDialogClosed()));
        mPasswordWidget->show();
    } else {
        emit printAssembledPicture(filename, copies);
    }
}

void archiveWidget::printFromPasswordDialog(QString filename, int copies)
{
    emit printAssembledPicture(filename, copies);
    passwordDialogClosed();
}

void archiveWidget::passwordDialogClosed()
{
    delete mPasswordWidget;
    mPasswordWidget = nullptr;
}

void archiveWidget::on_listView_clicked(const QModelIndex &index)
{
    QMap<int,QVariant> data = ui->listView->model()->itemData(index);
    QString filename = data.value(Qt::DisplayRole).toString();
    QString fullname = mStoragepath + QDir::separator() + mBasename + filename;
    qDebug() << fullname;
    delete mDetailWidget;
    mDetailWidget = new picturedetailWidget(fullname, this);
    connect(mDetailWidget, SIGNAL(printArchivePicture(QString,int)), this, SLOT(printArchivePicture(QString,int)));
    mDetailWidget->show();
}

void archiveWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
