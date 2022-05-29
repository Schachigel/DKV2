#include "filewriter.h"
#include <QPrinter>
#include <QFile>
#include <QFileInfo>
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


QString renderContent(const QString &templateName, const QVariantMap &data)
{
    QFile templateFile(appConfig::Outdir() + "/vorlagen/" + templateName);

    templateFile.open(QFile::ReadOnly);
    
    Mustache::Renderer renderer;
    Mustache::QtVariantContext context(data);
    QString renderedContent = templateFile.readAll();

    // We need 3 passes to replace all mustache variables.
    for (int pass = 1; pass <= 3; pass++)
        renderedContent = renderer.render(renderedContent, &context);

    // close files
    templateFile.close();

    return renderedContent;
}

bool textFileWrite(const QString &templateName, const QString &fileName, const QVariantMap &data)
{
    // get file extension from templateName
    QFileInfo fi(templateName);
    QString ext = qsl(".") + fi.completeSuffix();

    // render the content.
    QString renderedText = renderContent(templateName, data);
    // Write the html content to file. (e.g. for editing)

    QFile textFile(appConfig::Outdir() + fileName + ext);
    textFile.open(QFile::WriteOnly);
    textFile.write(renderedText.toUtf8());
    textFile.close();

    return true;
}

bool pdfWrite(const QString &templateName, const QString &fileName, const QVariantMap &data)
{
    //get QTextDocument reference
    QTextDocument doc;

    // render the content.
    QString renderedHtml = renderContent(templateName, data);
    doc.setHtml(renderedHtml);
    // doc.setPageSize(QPageSize(QPageSize::A4));

    // Write the html content to file. (e.g. for editing)
    QFile html(appConfig::Outdir () + qsl("/html/") + fileName + qsl(".html"));
    html.open(QFile::WriteOnly);
    html.write(renderedHtml.toUtf8 ());
    html.close();

    // Write the PDF using a QPrinter
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    QPageLayout pl =printer.pageLayout ();
    double leftB   = cm2Pt(3.); // breiter für Lochung
    double topB    = cm2Pt(1.); // logo darf in den oberen Rand reichen
    double rightB  = cm2Pt(0.); // logo darf in den Rand reichen
    double bottomB = cm2Pt(2.);
    pl.setPageSize (QPageSize(QPageSize::A4), QMargins(leftB, topB,rightB, bottomB));
    printer.setPageLayout (pl);
    printer.setOutputFileName(appConfig::Outdir() + "/" + fileName + qsl(".pdf"));

    QSizeF htmlSize = pl.pageSize ().size (QPageSize::Unit::Point);
    htmlSize.setWidth  (htmlSize.width() -leftB -rightB);
    htmlSize.setHeight (htmlSize.height() -topB -bottomB);
    doc.setPageSize(htmlSize);
    doc.print(&printer);


    return true;
}
