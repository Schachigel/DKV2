#include "filewriter.h"
#include <QPrinter>
#include <QFile>
#include "appconfig.h"
#include "mustache.h"

bool extractTemplateFileFromResource(const QString& path, const QString& file)
{   LOG_CALL_W(file);
    if( QFile(path +file).exists ())
        return true;
    QFile resource(qsl(":/res/")+file);
    resource.open(QIODevice::ReadOnly);
    QByteArray br =resource.readAll ();
    QFile target (path +file);
    target.open(QIODevice::WriteOnly);
    target.write (br);
    if( not QFile(path+file).exists()) {
        qCritical() << "failed to write template files";
        return false;
    } else {
        qInfo() << "newly created " << file;
        return true;
    }
}

namespace {

double cm2Pt(double cm)
{
    return cm * 28.3465;
}

}

bool pdfWrite(const QString& templateName, const QString& fileName, const QVariantMap& data)
{
    QString templateFname = appConfig::Outdir() + "/vorlagen/" + templateName;
    QFile templateFile(templateFname + ".html");
    QFile cssFile(templateFname + ".css");

    templateFile.open(QFile::ReadOnly);
    cssFile.open(QFile::ReadOnly);

    //get QTextDocument reference
    QTextDocument doc;

    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(data);
    QString renderedHtml = templateFile.readAll();

    // We need 3 passes to replace all mustache variables.
    for (int pass = 1; pass <= 3; pass++)
        renderedHtml = renderer.render(renderedHtml, &context);

    //Set CSS and HTML
    // doc.setPageSize(QPageSize(QPageSize::A4));
    doc.setDefaultStyleSheet(cssFile.readAll());
    doc.setHtml(renderedHtml);
#ifdef QT_DEBUG
    {
        QFile html(appConfig::Outdir () + "/" +fileName +qsl(".html"));
        html.open(QFile::WriteOnly);
        html.write(renderedHtml.toUtf8 ());
    }
#endif

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    QPageLayout pl =printer.pageLayout ();
    double leftB   = cm2Pt(3.); // breiter für Lochung
    double topB    = cm2Pt(2.);
    double rightB  = cm2Pt(0.); // logo darf in den Rand reichen
    double bottomB = cm2Pt(2.);
    pl.setPageSize (QPageSize(QPageSize::A4), QMargins(leftB, topB,rightB, bottomB));
    printer.setPageLayout (pl);
    printer.setOutputFileName(appConfig::Outdir() + "/" + fileName);

    QSizeF htmlSize = pl.pageSize ().size (QPageSize::Unit::Point);
    htmlSize.setWidth  (htmlSize.width() -leftB -rightB);
    htmlSize.setHeight (htmlSize.height() -topB -bottomB);
    doc.setPageSize(htmlSize);
    doc.print(&printer);

    //close files
    templateFile.close();
    cssFile.close();

    return true;
}
