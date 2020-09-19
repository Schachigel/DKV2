
#include <QDate>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTime>

// #include <QMutex>

#include "helper.h"
#include "windows.h"

QFile* outFile_p{nullptr};
int functionlogging::depth =0;
const QDate EndOfTheFuckingWorld = QDate(9999, 12, 31);
const QDate BeginingOfTime = QDate(1900, 1, 1);


#ifdef QT_DEBUG
void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void logger(QtMsgType type, const QMessageLogContext &, const QString &msg)
#endif
{
    // secure this code with a critical section in case we start logging from multiple threads
    if(!outFile_p)
    {
        outFile_p = new QFile(logFilePath());
        // this file will only be closed by the system at process end
        if (!outFile_p->open(QIODevice::WriteOnly | QIODevice::Append))
            abort();
    }

    static QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, qsl("DBuG")}, {QtInfoMsg, qsl("INFo")}, {QtWarningMsg, qsl("WaRN")}, {QtCriticalMsg, qsl("ERRo")}, {QtFatalMsg, qsl("FaTl")}});

    QString endlCorrectedMsg (msg);
    int lfCount = 0;
    while( endlCorrectedMsg.endsWith(QChar::LineFeed) || endlCorrectedMsg.endsWith(QChar::CarriageReturn) )
        {lfCount++; endlCorrectedMsg.chop(1);}

//    static QMutex mutex;
//    QMutexLocker lock(&mutex);

    QTextStream ts(outFile_p);
    ts << QTime::currentTime().toString(qsl("hh:mm:ss.zzz")) << " " << msgLevelHash[type] << " : " << endlCorrectedMsg;
#ifdef QT_DEBUG
    ts << " (" << context.file << ")";
#endif
    while(lfCount-->=0) ts << Qt::endl;

    if (type == QtFatalMsg)
        abort();
}

QString logFilePath()
{
    const DWORD size=256;
    WCHAR exe[size+1];
    QString filename("qtexe");
    if( GetModuleFileNameW(HMODULE(0), exe, size))
    {
        filename = QString::fromWCharArray(exe);
    }
    QFileInfo fi(filename);
    filename = fi.completeBaseName() + ".log";
    static QString logFilePath(QDir::toNativeSeparators(QDir::tempPath()) + QDir::separator() + filename);
    return logFilePath;
}

QMainWindow* getMainWindow()
{
    foreach(QWidget *widget, qApp->topLevelWidgets()) {
        QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget);
        if( mainWindow)
            return mainWindow;
    }
    return nullptr;
}
