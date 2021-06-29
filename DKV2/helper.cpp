
#include <QDate>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTime>

// #include <QMutex>

#include "helper.h"

QFile* outFile_p{nullptr};
int functionlogging::depth =0;
const QDate EndOfTheFuckingWorld =QDate(9999, 12, 31);
const QDate BeginingOfTime =QDate(1900, 1, 1);
const int daysUntilTheEndOfTheFuckingWorld =2916000;

void setFontPs(QWidget* w, int ps)
{
    QFont f =w->font();
    f.setPointSize(ps);
    w->setFont(f);
}

#ifdef QT_DEBUG
void logger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void logger(QtMsgType type, const QMessageLogContext &, const QString &msg)
#endif
{
    // secure this code with a critical section in case we start logging from multiple threads
    if( not outFile_p)
    {
        outFile_p = new QFile(logFilePath());
        // this file will only be closed by the system at process end
        if ( not outFile_p->open(QIODevice::WriteOnly | QIODevice::Append))
            abort();
    }

    static QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, qsl("DBuG")}, {QtInfoMsg, qsl("INFo")}, {QtWarningMsg, qsl("WaRN")}, {QtCriticalMsg, qsl("ERRo")}, {QtFatalMsg, qsl("FaTl")}});

    QString endlCorrectedMsg (msg);
    int lfCount = 0;
    while( endlCorrectedMsg.endsWith(QChar::LineFeed) or endlCorrectedMsg.endsWith(QChar::CarriageReturn) )
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
    QString filename(QCoreApplication::applicationFilePath());
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

QString getDbFileFromCommandline()
{//   LOG_CALL;
    // command line argument 1 has precedence
    QStringList args =QApplication::instance()->arguments();
    QString dbfileFromCmdline = args.size() > 1 ? args.at(1) : QString();

    if( dbfileFromCmdline.isEmpty())
        return QString();


    qInfo() << "dbfile taken from command line " << dbfileFromCmdline;
    return dbfileFromCmdline;
}
