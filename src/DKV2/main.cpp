#include "pch.h"

#include "dbstructure.h"
#include "mainwindow.h"
#include "appconfig.h"
#include "dkv2version.h"

#define ERROR_FILE_NOT_FOUND 5

#include "helper.h"

void initLogging()
{
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
{
    QString translationFolder = QApplication::applicationDirPath() + qsl("/translations/%1");
    installTranslateFile(translationFolder.arg(qsl("qtbase_de.qm")));
}

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    // 0 = disable, 1 = dark title/borders only, 2 = follow system + dark palette
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
#endif

    QApplication a(argc, argv);
    a.setOrganizationName(qsl("4-MHS")); // used to store our settings
    a.setApplicationName(qsl("DKV2"));
    a.setApplicationVersion(qsl(DKV2_VERSION_STRING));
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)
    a.setStyle("Fusion");
#endif
    QFont f =a.font();
    double systemFontSize =f.pointSizeF ();
    appConfig::setSystemFontsize( systemFontSize);
    f.setPointSizeF(systemFontSize *appConfig::Zoom());
    a.setFont(f);

    initLogging();

    QLocale locale(QLocale::German, QLocale::LatinScript, QLocale::Germany);
    QLocale::setDefault(locale); // do before starting the event loop
    setGermanUi();

    LOG_CALL;
    qInfo() << "************************************************";
    qInfo().noquote() << "DKV2 started " << QDate::currentDate().toString(qsl("dd.MM.yyyy")) << qsl("-") << QTime::currentTime().toString();
    qInfo().noquote() << "DKV2 Version: " << QCoreApplication::applicationVersion();
    qInfo().noquote() << "DKV2 latest commit" << GIT_COMMIT;
    qInfo() << "************************************************";

    init_DKDBStruct();

#ifndef QT_DEBUG
    QSplashScreen* splash = doSplash(); // do only AFTER having an app. object
    splash->show();
    QTimer::singleShot(3500, splash, &QWidget::close);
#endif

    MainWindow w;
    w.setFont(f);
    if( not w.dbLoadedSuccessfully) return -1;

    w.show();
    int ret = a.exec();

    qInfo() << "DKV2 finished";
    return ret;

}
