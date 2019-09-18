#include <QtCore>

#include "dkdbhelper.h"
#include "filehelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "windows.h"
#include <qpair.h>
#include <qfiledialog.h>
#include <QRandomGenerator>
#include <QMessageBox>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qsqlquerymodel.h>
#include <qsqltablemodel.h>


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
    db.open();
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    ui->statusBar->showMessage(newDbFile);
}

void MainWindow::preparePersonTableView()
{
    QSqlTableModel* model = new QSqlTableModel(ui->PersonsTable);
    //model->setQuery("SELECT Vorname, Name, Strasse, PLZ, Stadt FROM DkGeber");
    model->setTable("DKGeber");
    model->select();
    ui->PersonsTable->setModel(model);
    ui->PersonsTable->hideColumn(0);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef QT_DEBUG
    ui->menuDebug->setTitle("Debug");
#endif
    setCentralWidget(ui->stackedWidget);
    openAppDefaultDb();
    preparePersonTableView();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Neue_DB_anlegen_triggered()
{
    QString dbfile = QFileDialog::getSaveFileName(this, "Neue DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr);
    if( dbfile == "")
        return;

    backupFile(dbfile);
    createDKDB(dbfile);
    QSettings config;
    config.setValue("db/last", dbfile);
    openAppDefaultDb();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    ui->statusBar->showMessage(dbfile);
}

void MainWindow::on_actionProgramm_beenden_triggered()
{
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
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
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
    openAppDefaultDb(dbfile);
}

void MainWindow::on_action_Liste_triggered()
{
    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}

QString single_quoted(QString s)
{
    return "'" + s + "'";
}

void MainWindow::on_actioncreateSampleData_triggered()
{
    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse"};
    QList <QPair<int, QString>> Cities {{68305, "Mannheim"}, {69123, "Heidelberg"}, {69123, "Karlsruhe"}, {90345, "Hamburg"}};
    QRandomGenerator rand(::GetTickCount());
    QSqlQuery query("", QSqlDatabase::database());
    for( int i = 0; i<30; i++)
    {
        QString sql = QString("INSERT INTO DKGeber (Vorname, Nachname, Strasse, Plz, Stadt, IBAN, BIC) VALUES ( :vorn, :nachn, :strasse, :plz, :stadt, :iban, :bic)");
        QString Vorname  = single_quoted( Vornamen [rand.bounded(Vornamen.count ())]);
        QString Nachname = single_quoted( Nachnamen[rand.bounded(Nachnamen.count())]);
        QString Strasse =  single_quoted( Strassen[rand.bounded(Strassen.count())]);
        int stadtindex = rand.bounded(Cities.count());
        //sql = sql.replace(":vorn", Vorname);
        sql = sql.replace(":nachn", Nachname);
        sql = sql.replace(":strasse", Strasse);
        sql = sql.replace(":plz", single_quoted( QString::number(Cities[stadtindex].first)));
        sql = sql.replace(":stadt", single_quoted(  Cities[stadtindex].second));
        sql = sql.replace(":iban", single_quoted("iban xxxxxxxxxxxxxxxxx"));
        sql = sql.replace(":bic", single_quoted("BICxxxxxxxx"));

/*        query.bindValue(":nn", Nachname);
        query.bindValue(":s", Strasse);
        query.bindValue(":plz", single_quoted( QString::number(Cities[stadtindex].first)));
        query.bindValue(":st", single_quoted(  Cities[stadtindex].second));
        query.bindValue(":iban", "'iban'");
        query.bindValue(":bic", "'bic'");
*/
        query.prepare(sql);
        query.bindValue(":vorn", Vorname);
        if( !query.exec())
            qWarning() << "Creating demo data failed\n" << query.lastQuery() << endl << query.lastError().text();
        else
            qDebug() << query.lastQuery() << "executed successfully\n" << sql;
    }


}
