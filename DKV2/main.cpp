#include "mainwindow.h"

#include "filehelper.h"

#include "dkdbhelper.h"

#include <QApplication>
#include <QSettings>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QDebug>
#include <QTime>
#include <qfile.h>
#include <qdir.h>
#include <QTextStream>

#if defined (Q_OS_WIN)
#include <windows.h>
#else
#define ERROR_FILE_NOT_FOUND 5
#endif

#include <helper.h>

void initLogging()
{
    if( backupFile(logFilePath()))
        QFile::remove(logFilePath());
    qInstallMessageHandler(logger);
}

QString getInitialDb()
{   LOG_ENTRY_and_EXIT;
    QSettings config;
    QString dbfile = config.value("db/last").toString();
    qDebug() << "DbFile from configuration: " << dbfile;
    if(  dbfile != "" && QFile::exists(dbfile) && istValideDatenbank(dbfile))
        return dbfile;
    do
    {
        dbfile = QFileDialog::getSaveFileName(nullptr,
                 "Wähle eine Datenbank oder gib einen Namen für eine Neue ein",
                 "..\\data", "dk-DB Dateien (*.dkdb)", nullptr,QFileDialog::DontConfirmOverwrite);

        qDebug() << "DbFile from user: " << dbfile;
        if( dbfile == "") return QString();  // canceled by user

        if( QFile::exists(dbfile))
        {
            if( istValideDatenbank(dbfile))
                return dbfile;
            else
            {
                if( QMessageBox::Yes == QMessageBox::information(nullptr, "Die gewählte Datenbank ist ungültig", "Soll die Datei für eine neue DB überschrieben werden?"))
                {
                    QFile::remove(dbfile);
                    if( DKDatenbankAnlegen(dbfile))
                         return dbfile;
                    else
                    {
                        qCritical() << "Overwrite of existing db failed";
                        QMessageBox::critical(nullptr, "Feher", "Die Datei konnte nicht überschrieben werden. Wähle eine andere!");
                        continue;
                    }
                }
            }
        }
        // new file ...
        if( DKDatenbankAnlegen(dbfile))
            return dbfile;
        else
        {
            qCritical() << "Creating of existing db failed";
            if( QMessageBox::Yes != QMessageBox::question(nullptr, "Feher", "Die Datei konnte nicht angelegt werden. Möchten Sie eine andere auswählen?"))
                return QString();
            continue;
        }
    }
    while(true);
}

int main(int argc, char *argv[])
{
    initLogging();
    LOG_ENTRY_and_EXIT;
    initDKDBStruktur();
    qInfo() << "DKV2 started " << QDate::currentDate().toString("dd.MM.yyyy") << "-" << QTime::currentTime().toString();
    QApplication a(argc, argv);
    a.setOrganizationName("4-MHS"); // used to store our settings
    a.setApplicationName("DKV2");
    QString dbfile =getInitialDb();
    if( dbfile == "")
    {
        qCritical() << "No valid DB -> abort";
        return ERROR_FILE_NOT_FOUND;
    }

    QSettings config; config.setValue("db/last", dbfile);
    MainWindow w;
    w.show();
    int ret = a.exec();
    qInfo() << "DKV2 finished";

    return ret;

}
