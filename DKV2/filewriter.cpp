#include "filewriter.h"
#include <QPrinter>
#include <QFile>
#include "appconfig.h"
#include "mustache.h"


bool pdfWrite(QString templateName, QString fileName, QVariantMap data) 
{

    QString templateFname = appConfig::Outdir() + "/printres/" + templateName;
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
