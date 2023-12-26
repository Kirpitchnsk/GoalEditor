#include "helpinfo.h"
#include "ui_helpinfo.h"

#include <QFile>
#include <QTextStream>
#include<QDebug>
#include <QTextBrowser>

HelpInfo::HelpInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpInfo)
{
    ui->setupUi(this);

    QFile file(":/styles/help.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString htmlContent = in.readAll();
        ui->textBrowser->setHtml(htmlContent);
        file.close();
    } else {
        qDebug()<<"Не открывается файл";
    }
}

HelpInfo::~HelpInfo()
{
    delete ui;
}
