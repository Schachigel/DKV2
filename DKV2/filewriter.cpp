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
    {
        renderedHtml = renderer.render(renderedHtml, &context);
    }

    //Set CSS and HTML
    // doc.setPageSize(QPageSize(QPageSize::A4));
    doc.setDefaultStyleSheet(cssFile.readAll());
    doc.setHtml(renderedHtml);

    QPrinter printer;
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMargins(3, 3, 3, 3));

    printer.setOutputFileName(appConfig::Outdir() + "/" + fileName);

    doc.print(&printer);

    //close files
    templateFile.close();
    cssFile.close();

    return true;
}
