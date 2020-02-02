#include "mainwindow.h"

#include "filehelper.h"

#include "dkdbhelper.h"

#include <QDebug>
#include <QDirIterator>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
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

QString initDb()
{
    QString dbfile =getInitialDb();
    if( dbfile == "")
    {
        qCritical() << "No valid DB -> abort";
        exit (ERROR_FILE_NOT_FOUND);
    }
    return dbfile;
}

QSplashScreen* doSplash()
{
    QPixmap pixmap(":/res/splash.png");
    QSplashScreen *splash = new QSplashScreen(pixmap, Qt::SplashScreen|Qt::WindowStaysOnTopHint);
    splash->show();
    return splash;
}

int main(int argc, char *argv[])
{
    initLogging();
    LOG_ENTRY_and_EXIT;
    QLocale locale(QLocale::German, QLocale::LatinScript, QLocale::Germany);
    QLocale::setDefault(locale); // do before starting the event loop

    qInfo() << "DKV2 started " << QDate::currentDate().toString("dd.MM.yyyy") << "-" << QTime::currentTime().toString();

    QApplication a(argc, argv);
    a.setOrganizationName("4-MHS"); // used to store our settings
    a.setApplicationName("DKV2");

#ifndef QT_DEBUG
    QSplashScreen* splash = doSplash(); // do only AFTER having an app. object
#else
    QSplashScreen* splash = nullptr;
#endif

    initDKDBStruktur();

    QSettings config;
    config.setValue("db/last", initDb());

    MainWindow w;
    w.setSplash(splash);
    w.show();

    int ret = a.exec();

    qInfo() << "DKV2 finished";
    return ret;

}
