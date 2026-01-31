#include "helper.h"

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

