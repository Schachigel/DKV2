#include "helper.h"
#include "helper_core.h"
#include "helperfile.h"
#include "appconfig.h"
#include "filewriter.h"
#include "mustache.h"

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

// TODO: maybe this should be in another file, considering so many dependencies
bool savePdfFromHtmlTemplate(const QString &templateFileName, const QString &outputFileName, const QVariantMap &data)
{   LOG_CALL;
    QFileInfo fi( outputFileName);
    QString fullOutputFileName {outputFileName};
    if(fi.isRelative ())
        fullOutputFileName =appConfig::Outdir () +qsl("/") +outputFileName;
    QString css{fileToString (appConfig::Outdir ()+qsl("/vorlagen/") +qsl("zinsbrief.css"))};

    // DEBUG   printHtmlToPdf(renderedHtml, css, htmlFileName);

    // Prepare the printer
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    QPageLayout pl =printer.pageLayout ();
    double leftB   = cm2Pt(3.); // breiter für Lochung
    double topB    = cm2Pt(1.); // logo darf in den oberen Rand reichen
    double rightB  = cm2Pt(0.); // logo darf in den Rand reichen
    double bottomB = cm2Pt(2.);
    pl.setPageSize (QPageSize(QPageSize::A4), QMargins(leftB, topB,rightB, bottomB));
    printer.setPageLayout (pl);
    printer.setOutputFileName(fullOutputFileName);

    //Prepare the document
    QTextDocument doc;
    QString renderedHtml = mustachReplace(templateFileName, data);
    doc.setPageSize(pl.pageSize ().size (QPageSize::Unit::Point));

    // render the content.
    doc.setDefaultStyleSheet (css);
    doc.setHtml(renderedHtml);
    // Write the PDF using a QPrinter
    doc.print(&printer);

    // Write the html content to file. (just in case ... e.g. for editing)
    QString htmlFileName {appConfig::Outdir () +qsl("/html/") +replaceExtension(outputFileName, qsl(".html"))};
    stringToFile( renderedHtml, htmlFileName);

    return true;
}
