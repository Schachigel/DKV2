#include <QDebug>
#include <QDirIterator>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QTranslator>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QTextStream>
#include <QTimer>

#include "helperfile.h"
#include "appconfig.h"
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

QString getInitialDbFile()
{   LOG_CALL;
    // command line argument 1 has precedence
    QStringList args =QApplication::instance()->arguments();
    QString dbfileFromCmdline = args.size() > 1 ? args.at(1) : QString();

    if( !dbfileFromCmdline.isEmpty()) {
        // if there is a cmd line arg we will not try other
        // and not store in appConfig
        if( isValidDatabase( dbfileFromCmdline)) {
            qInfo() << "valid dbfile from command line " << dbfileFromCmdline;
            return dbfileFromCmdline;
        } else {
            qCritical() << "invalid dbfile from comman line" << dbfileFromCmdline;
            return QString();
        }
    }

    QString dbfile =appConfig::LastDb();
    if( isValidDatabase(dbfile)) {
        // all good then
        qInfo() << "DbFile from configuration exists and is valid: " << dbfile;
        appConfig::setLastDb(dbfile);
        return dbfile;
    } else {
        qInfo() << "invalid DB file from configuration: " << dbfile;
        return QString();
    }
}

QSplashScreen* doSplash()
{   LOG_CALL;
    QPixmap pixmap(qsl(":/res/splash.png"));
    QSplashScreen *splash = new QSplashScreen(pixmap, Qt::SplashScreen|Qt::WindowStaysOnTopHint);
    splash->show();
    return splash;
}

// QTranslator trans;

void installTranslateFile(const QString& translationFile, const QString& path)
{
    QTranslator* pTrans = new QTranslator();
    if( pTrans->load(translationFile, path))
        if( QCoreApplication::installTranslator(pTrans)) {
            qInfo() << "Successfully installed language file " << translationFile;
            return;
        }
    qCritical() << "failed to load translations " << translationFile;
}

void setGermanUi()
{   LOG_CALL;
    QString translationFile = QDir::currentPath() + qsl("/translations/qt_de.qm");
    installTranslateFile(qsl("qt_de.qm"), QDir::currentPath() + qsl("/translations"));
}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    a.setOrganizationName(qsl("4-MHS")); // used to store our settings
    a.setApplicationName(qsl("DKV2"));

    QLocale locale(QLocale::German, QLocale::LatinScript, QLocale::Germany);
    QLocale::setDefault(locale); // do before starting the event loop
    setGermanUi();

    initLogging();

    LOG_CALL;
    qInfo() << "DKV2 started " << QDate::currentDate().toString(qsl("dd.MM.yyyy")) << qsl("-") << QTime::currentTime().toString();

    init_DKDBStruct();

    // let propose a db to mainwindow
    appConfig::setCurrentDb(getInitialDbFile());

#ifndef QT_DEBUG
    QSplashScreen* splash = doSplash(); // do only AFTER having an app. object
    splash->show();
    QTimer::singleShot(3500, splash, &QWidget::close);
#endif

    MainWindow w;
    w.show();
    int ret = a.exec();

    qInfo() << "DKV2 finished";
    return ret;

}
