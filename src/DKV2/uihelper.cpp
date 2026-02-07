#include "uihelper.h"
#include "filewriter.h"
#include "helper_core.h"

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

bool showInExplorer(const QString &pathOrFilename, showObject fileOrFolder)
{   LOG_CALL_W(pathOrFilename);
#ifdef _WIN32    //Code for Windows
    QString fullPath {pathOrFilename};
    QFileInfo fi(fullPath);
    if(fi.isRelative ())
        fullPath = appendFilenameToOutputDir(pathOrFilename);

    QString explorerW_selectedFile = QDir::toNativeSeparators(fullPath);
    if( fileOrFolder == showObject::file)
        explorerW_selectedFile =qsl(" /select,\"%1\"").arg(explorerW_selectedFile);
    if( fileOrFolder == showObject::folder)
        explorerW_selectedFile =qsl(" \"%1\"").arg(explorerW_selectedFile);

    QProcess p;
    p.setNativeArguments(explorerW_selectedFile);
    p.setProgram(qsl("explorer.exe"));
    qint64 pid;
    return p.startDetached(&pid);
//  ?! debugging showed, that .waitForStarted always returns an error (on my system...)
//    if( not p.waitForStarted(-1))
//        qDebug().noquote ()<< "failed to start explorer. Arg was: " << explorerW_selectedFile << " Error was " << p.error ();

#elif defined(__APPLE__)    //Code for Mac
    Q_UNUSED(fileOrFolder);

    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + pathOrFilename + "\""});
    QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
#else
    Q_UNUSED (fileOrFolder);
    return true;
#endif
}


