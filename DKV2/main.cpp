#include <QDebug>
// #include <QDirIterator>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QTranslator>
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

QSplashScreen* doSplash()
{   LOG_CALL;
    QPixmap pixmap(qsl(":/res/splash.png"));
    QSplashScreen *splash = new QSplashScreen(pixmap, Qt::SplashScreen|Qt::WindowStaysOnTopHint);
    splash->show();
    return splash;
}

// QTranslator trans;

void installTranslateFile(const QString& translationFile)
{
    QTranslator* pTrans = new QTranslator();
    if( pTrans->load(translationFile))
        if( QCoreApplication::installTranslator(pTrans)) {
            qInfo() << "Successfully installed language file " << translationFile;
            return;
        }
    qCritical() << "failed to load translations " << translationFile;
}

void setGermanUi()
{   LOG_CALL;
    QString translationFolder = QApplication::applicationDirPath() + qsl("/translations/%1");
    installTranslateFile(translationFolder.arg(qsl("qt_de.qm")));
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
    qInfo() << "************************************************";
    qInfo() << "DKV2 started " << QDate::currentDate().toString(qsl("dd.MM.yyyy")) << qsl("-") << QTime::currentTime().toString();
    qInfo() << "DKV2 Version: " << QCoreApplication::applicationVersion();
    qInfo() << "DKV2 latest commit" << GIT_COMMIT;
    qInfo() << "************************************************";

    init_DKDBStruct();

#ifndef QT_DEBUG
    QSplashScreen* splash = doSplash(); // do only AFTER having an app. object
    splash->show();
    QTimer::singleShot(3500, splash, &QWidget::close);
#endif

    MainWindow w;
    if( !w.dbLoadedSuccessfully) return -1;

    w.show();
    int ret = a.exec();

    qInfo() << "DKV2 finished";
    return ret;

}
