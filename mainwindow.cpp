#include <QtCore>

#include "dkdbhelper.h"
#include "filehelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qfiledialog.h>
#include <QMessageBox>


void MainWindow::initSettings()
{
    QSettings config;
    QString dbfile = config.value("db/last").toString();
    qDebug() << "DbFile read from configuration: " << dbfile;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    initSettings();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Neue_DB_anlegen_triggered()
{
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "", "dkv2.s3db", nullptr);
    if( dbfile == "")
        return;

    backupFile(dbfile);
    createDKDB(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    ui->statusBar->showMessage(dbfile);
}

void MainWindow::on_actionProgramm_beenden_triggered()
{
    this->close();
}

void MainWindow::on_actionDBoeffnen_triggered()
{
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "", "dkv2.s3db", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde ausgewählt";
        return;
    }
    QSettings config;
    config.setValue("db/last", dbfile);
    ui->statusBar->showMessage(dbfile);
}
