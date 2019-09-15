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

#include <windows.h>

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
    logFilePath = QDir::toNativeSeparators(QDir::tempPath()) + QDir::separator() + "dkv2.log";
    if( backupFile(logFilePath))
        QFile::remove(logFilePath);
    qInstallMessageHandler(logger);
}

QString getInitialDb()
{
    QSettings config;
    QString dbfile = config.value("db/last").toString();
    qDebug() << "DbFile from configuration: " << dbfile;
    do
    {
        if( dbfile == "")
        {
            dbfile = QFileDialog::getSaveFileName(nullptr, "DkVerarbeitungs Datenbank", "*.s3db", "dk-DB Dateien (*.s3db)", nullptr,QFileDialog::DontConfirmOverwrite);
            qDebug() << "DbFile from user: " << dbfile;
            if( dbfile == "") return QString();
        }
        if(!QFile::exists(dbfile))
        {
            if( !createDKDB(dbfile))
                return QString();
            qDebug() << "created new DbFile: " << dbfile;
        }
        if( isValidDb(dbfile))
        {
            config.setValue("db/last", dbfile);
            return dbfile;
        }
        else
        {
            auto reply = QMessageBox::question(nullptr, "DK Datenbank ungültig", "Möchten Sie eine DK Datenbankdatei auswählen?", QMessageBox::Yes|QMessageBox::No);
            if( reply == QMessageBox::Cancel || reply == QMessageBox::No)
                return QString();
            dbfile = "";
        }
    }
    while(true);
}

int main(int argc, char *argv[])
{
    initLogging();
    initDbHelper();

    qInfo() << "DKV2 started";
    QApplication a(argc, argv);
    a.setOrganizationName("4-MHS"); // used to store our settings
    a.setApplicationName("DKV2");

    if( getInitialDb() == "")
    {
        qCritical() << "No valid DB -> abort";
        return ERROR_FILE_NOT_FOUND;
    }
    MainWindow w;
    w.show();
    int ret = a.exec();
    qInfo() << "DKV2 finished";

    return ret;

}
