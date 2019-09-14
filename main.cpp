#include "mainwindow.h"

#include "filehelper.h"

#include <QApplication>
#include <QDebug>
#include <QTime>
#include <qfile.h>
#include <qdir.h>
#include <QTextStream>

static QString logFilePath;
static QFile* outFile_p(nullptr);

void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // secure this code with a critical section in case we start logging from multiple threads
    if(!outFile_p)
    {
        outFile_p = new QFile(logFilePath);
        // this file will only be closed by the system at process end
        if (!outFile_p->open(QIODevice::WriteOnly | QIODevice::Append))
            abort();
    }
    static QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "DBuG"}, {QtInfoMsg, "INFo"}, {QtWarningMsg, "WaRN"}, {QtCriticalMsg, "ERRo"}, {QtFatalMsg, "FaTl"}});

    QTextStream ts(outFile_p);
    ts << QTime::currentTime().toString("hh:mm:ss.zzz") << " " << msgLevelHash[type] << " : " << msg << " (" << context.file << ")" << endl;

    if (type == QtFatalMsg)
        abort();
}

void initLogging()
{
    logFilePath = QDir::tempPath() + QDir::separator() + "dkv2.log";
    if( backupFile(logFilePath))
        QFile::remove(logFilePath);
    qInstallMessageHandler(logger);
}

int main(int argc, char *argv[])
{
    initLogging();

    qDebug() << "DKV2 started";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int ret = a.exec();
    qDebug() << "DKV2 finished";

    return ret;

}
