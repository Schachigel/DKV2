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
    setStatus(newDbFile);
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

void MainWindow::setStatus(QString statustext)
{
    ui->statusLabel->setText(statustext);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef QT_DEBUG
    ui->menuDebug->setTitle("Debug");
#endif
    ui->statusBar->addPermanentWidget(ui->statusLabel);
    setCentralWidget(ui->stackedWidget);
    openAppDefaultDb();
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
    setStatus(dbfile);
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
    preparePersonTableView();
    ui->stackedWidget->setCurrentIndex(PersonListIndex);
}

QString single_quoted(QString s)
{
    return "'" + s + "'";
}
struct PersonData
{
    QString Vorname;
    QString Nachname;
    QString Strasse;
    QString Plz;
    QString Stadt;
    QString Iban;
    QString Bic;
};

bool savePersonDataToDatabase(const PersonData& p)
{
    QSqlQuery query("", QSqlDatabase::database());
    QString sql = QString("INSERT INTO DKGeber (Vorname, Nachname, Strasse, Plz, Stadt, IBAN, BIC) VALUES ( :vorn, :nachn, :strasse, :plz, :stadt, :iban, :bic)");
    query.prepare(sql);
    query.bindValue(":vorn", p.Vorname);
    query.bindValue(":nachn", p.Nachname);
    query.bindValue(":strasse", p.Strasse);
    query.bindValue(":plz",  p.Plz);
    query.bindValue(":stadt", p.Stadt);
    query.bindValue(":iban", p.Iban);
    query.bindValue(":bic", p.Bic);
    if( !query.exec())
    {
        qWarning() << "Creating demo data failed\n" << query.lastQuery() << endl << query.lastError().text();
        return false;
    }
    else
    {
        qDebug() << query.lastQuery() << "executed successfully\n" << sql;
        return true;
    }
}

void MainWindow::on_actioncreateSampleData_triggered()
{
    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse"};
    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
    QRandomGenerator rand(::GetTickCount());
    for( int i = 0; i<30; i++)
    {
        PersonData p;
        p.Vorname  =  Vornamen [rand.bounded(Vornamen.count ())];
        p.Nachname = Nachnamen[rand.bounded(Nachnamen.count())];
        p.Strasse =  Strassen[rand.bounded(Strassen.count())];
        p.Plz = Cities[rand.bounded(Cities.count())].first;
        p.Stadt = Cities[rand.bounded(Cities.count())].second;
        p.Iban = "iban xxxxxxxxxxxxxxxxx";
        p.Bic = "BICxxxxxxxx";
        savePersonDataToDatabase(p);
    }
    static_cast<QSqlTableModel*>(ui->PersonsTable->model())->select();
}

bool MainWindow::savePerson()
{
    PersonData p{ ui->leVorname->text(),
                ui->leNachname->text(),
                ui->leStrasse->text(),
                ui->lePlz->text(),
                ui->leStadt->text(),
                ui->leIban->text(),
                ui->leBic->text()};
    if( p.Vorname == "" || p.Nachname == "" || p.Strasse =="" || p.Plz == "" || p.Stadt == "")
    {
        QMessageBox(QMessageBox::Warning, "Daten nicht gespeichert", "Namens - und Adressfelder dürfen nicht leer sein");
        return false;
    }
    savePersonDataToDatabase(p);
    return true;
}

void MainWindow::on_actionNeuer_DK_Geber_triggered()
{
    ui->stackedWidget->setCurrentIndex(newPersonIndex);
}

void MainWindow::on_saveExit_clicked()
{
    if( savePerson())
    {
        emptyEditPersonFields();
    }
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}

void MainWindow::emptyEditPersonFields()
{
    ui->leVorname->setText("");
    ui->leNachname->setText("");
    ui->leStrasse->setText("");
    ui->lePlz->setText("");
    ui->leStadt->setText("");
    ui->leIban->setText("");
    ui->leBic->setText("");
}

void MainWindow::on_saveNew_clicked()
{
    if( savePerson())
    {
        emptyEditPersonFields();
    }
}

void MainWindow::on_saveList_clicked()
{
    if( savePerson())
    {
        emptyEditPersonFields();
        on_action_Liste_triggered();
    }
}

void MainWindow::on_cancel_clicked()
{
    emptyEditPersonFields();
    ui->stackedWidget->setCurrentIndex(emptyPageIndex);
}
