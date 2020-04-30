#include <QDebug>
#include <QDirIterator>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QTextStream>

#include "filehelper.h"
#include "dkdbhelper.h"
#include "mainwindow.h"




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

QString interactW_UserForDB(QString dbfile)
{   LOG_CALL;
    do
    {
        dbfile = QFileDialog::getSaveFileName(nullptr,
                                              "Wähle eine Datenbank oder gib einen Namen für eine Neue ein",
                                              QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +"/DKV2", "dk-DB Dateien (*.dkdb)", nullptr,QFileDialog::DontConfirmOverwrite);
        qDebug() << "DbFile from user: " << dbfile;
        if( dbfile == "") return QString();  // canceled by user

        if( QFile::exists(dbfile))
        {
            if( isValidDatabase(dbfile))
                return dbfile;
            else
            {
                QMessageBox::StandardButton sb = QMessageBox::question(nullptr, "Die gewählte Datenbank ist ungültig", "Soll die Datei für eine neue DB überschrieben werden?");
                qDebug() << "users choice: replace file " << sb;
                if( QMessageBox::Yes != sb)
                    continue;
                else
                {
                    if( create_DK_database(dbfile))
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
        if( create_DK_database(dbfile))
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

QString getInitialDb()
{   LOG_CALL;
    QSettings config;
    QString dbfile = config.value("db/last").toString();
    if(  dbfile != "" && QFile::exists(dbfile) && isValidDatabase(dbfile))
    {
        qDebug() << "DbFile from configuration exists and is valid: " << dbfile;
        return dbfile;
    }

    return interactW_UserForDB(dbfile);
}

QString initDb()
{   LOG_CALL;
    QString dbfile =getInitialDb();
    if( dbfile == "")
    {
        qCritical() << "No valid DB -> abort";
        exit (ERROR_FILE_NOT_FOUND);
    }
    return dbfile;
}

QSplashScreen* doSplash()
{   LOG_CALL;
    QPixmap pixmap(":/res/splash.png");
    QSplashScreen *splash = new QSplashScreen(pixmap, Qt::SplashScreen|Qt::WindowStaysOnTopHint);
    splash->show();
    return splash;
}

void setGermanUi()
{   LOG_CALL;
    QTranslator trans;
    QString translationFile = QDir::currentPath() + "/translations/qt_de.qm";
    if( trans.load(QLocale(),translationFile))
        QCoreApplication::installTranslator(&trans);
    else
        qCritical() << "failed to load translations " << translationFile;
}

int main(int argc, char *argv[])
{
    initLogging();
    LOG_CALL;
    QLocale locale(QLocale::German, QLocale::LatinScript, QLocale::Germany);
    QLocale::setDefault(locale); // do before starting the event loop

    qInfo() << "DKV2 started " << QDate::currentDate().toString("dd.MM.yyyy") << "-" << QTime::currentTime().toString();


    QApplication a(argc, argv);
    a.setOrganizationName("4-MHS"); // used to store our settings
    a.setApplicationName("DKV2");

    setGermanUi();

    init_DKDBStruct();
    init_additionalTables();

    QString db = initDb();
    QSettings config;
    config.setValue("db/last", db);

#ifndef QT_DEBUG
    QSplashScreen* splash = doSplash(); // do only AFTER having an app. object
#else
    QSplashScreen* splash = nullptr;
#endif

    MainWindow w;
    w.setSplash(splash);
    w.show();

    int ret = a.exec();

    qInfo() << "DKV2 finished";
    return ret;

}
