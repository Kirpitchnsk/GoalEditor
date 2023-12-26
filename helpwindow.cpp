#include "helpwindow.h"
#include "ui_helpwindow.h"
#include <QTextBrowser>

#include <QFile>
#include <QTextStream>
#include<QDebug>

HelpWindow::HelpWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::helpwindow)
{
    ui->setupUi(this);



}

HelpWindow::~HelpWindow()
{
    delete ui;
}
