#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <iostream>
#include "helper.h"
#ifndef QT_DEBUG
#include "helperfile.h"
#endif

QFile* outFile_p{nullptr};
int functionlogging::depth =0;

QString toString(const QBitArray& ba)
{
    QString res;
    for( int i=0; i < ba.size (); i++)
        res.append (ba[i] ? qsl("1") : qsl("0"));
    return res;
}

QBitArray toQBitArray(const QString& s)
{
    QBitArray ba(s.size ());
    for( int i=0; i<s.size (); i++)
        ba[i] = (s[i] == qsl("1")) ? true : false;
    return ba;
}



void centerDlg(QWidget* parent, QWidget* child, int minWidth /*=300*/, int minHeight /*=400*/)
{
    int nWidth =qMax(child->width(), minWidth), nHeight =qMax(child->height(), minHeight);
    if (parent not_eq NULL) {
        QPoint parentPos = parent->mapToGlobal(parent->pos());
        QPoint newPos(parentPos.x()+parent->width()/2 - nWidth/2, parentPos.y() + parent->height()/2 -nHeight/2);
        newPos= parent->mapFromGlobal(newPos);
        child->setGeometry(newPos.x(), newPos.y(),
                    nWidth, nHeight);
    }
}

void logger(QtMsgType type, const QMessageLogContext& c, const QString &msg)
{
    Q_UNUSED(c);
    // secure this code with a critical section in case we start logging from multiple threads
    if( not outFile_p) {
        outFile_p = new QFile(logFilePath());
        // this file will only be closed by the system at process end
        if ( not outFile_p->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            abort();
        }
    }
    if( outFile_p) {
        static QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, qsl("DBuG")}, {QtInfoMsg, qsl("INFo")}, {QtWarningMsg, qsl("WaRN")}, {QtCriticalMsg, qsl("ERRo")}, {QtFatalMsg, qsl("FaIl")}});

        QString endlCorrectedMsg (msg);
        int lfCount = 0;
        while( endlCorrectedMsg.endsWith(QChar::LineFeed) or endlCorrectedMsg.endsWith(QChar::CarriageReturn) )
        {lfCount++; endlCorrectedMsg.chop(1);}

        //    static QMutex mutex;
        //    QMutexLocker lock(&mutex);
        QString out {qsl("%1 %2 %3").arg(QTime::currentTime().toString(qsl("hh:mm:ss.zzz")), msgLevelHash[type], /*code, */endlCorrectedMsg)};
#ifdef Q_OS_WIN
        if( out.length () < 32765)
            OutputDebugString(reinterpret_cast<const wchar_t *>(out.utf16()));
#endif
        QTextStream ts(outFile_p);
        ts << out;
        while(lfCount-->=0) ts << qsl("\n");

        if (type == QtFatalMsg)
            abort();
    }
}

QString getLogFileName()
{
    QFileInfo fi(QCoreApplication::applicationFilePath ());
#ifdef QT_DEBUG
    return qsl("%1/%2.log").arg(QDir::tempPath (), fi.completeBaseName ());
#else
    return getUniqueTempFilename (qsl("%1/%2.log").arg(QDir::tempPath(), fi.completeBaseName ()),
                                             QDateTime::currentDateTime ().toString(qsl("_yyyy_MM_dd_hhmmss_")));
#endif
}

QString logFilePath()
{
    static QString logFilePath{getLogFileName()};
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
{
    // command line argument 1 has precedence
    QStringList args =QApplication::instance()->arguments();
    QString dbfileFromCmdline = args.size() > 1 ? args.at(1) : QString();

    if( dbfileFromCmdline.isEmpty())
        return QString();


    qInfo() << "dbfile taken from command line " << dbfileFromCmdline;
    return dbfileFromCmdline;
}
