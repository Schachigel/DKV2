#include <QtCore>

#include "dkdbhelper.h"
#include "filehelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qfiledialog.h>
#include <QMessageBox>

void closeDbConnection()
{
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count() == 0)
        return;
    if( cl.count() > 1)
    {
        qWarning() << "Found " << cl.count() << "connections open, when there should be 1 or 0";
        return;
    }
    QSqlDatabase::removeDatabase(cl[0]);
    qInfo() << "Database connection " << cl[0] << " removed";
}

void MainWindow::openAppDefaultDb( QString newDbFile)
{
    closeDbConnection();
    QSettings config;
    if( newDbFile == "")
    {
        newDbFile = config.value("db/last").toString();
        qInfo() << "opening DbFile read from configuration: " << newDbFile;
    }
    else
    {
        config.setValue("db/last", newDbFile);
    }

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    ui->statusBar->showMessage(newDbFile);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openAppDefaultDb();
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
    QString dbfile = QFileDialog::getOpenFileName(this, "DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr);
    if( dbfile == "")
    {
        qDebug() << "keine Datei wurde ausgewählt";
        return;
    }
    openAppDefaultDb(dbfile);
}
