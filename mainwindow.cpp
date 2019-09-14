#include <QtCore>

#include "dkdbhelper.h"
#include "filehelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Neue_DB_anlegen_triggered()
{
    const QString dbfile("dkv2.s3db");
    backupFile(dbfile);
    createDKDB(dbfile);
}
