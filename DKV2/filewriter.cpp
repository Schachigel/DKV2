
#include "helperfile.h"
#include "appconfig.h"
#include "mustache.h"
#include "filewriter.h"

bool extractTemplateFileFromResource(const QString& path, const QString& file)
{   LOG_CALL_W(file);
    QFileInfo fi( QDir(path), file);
    if( fi.exists ())
        return true;

    QFile resource(qsl(":/res/")+file);
    resource.open(QIODevice::ReadOnly);
    QByteArray br =resource.readAll ();

    QFile target (fi.absoluteFilePath ());
    target.open(QIODevice::WriteOnly);
    target.write (br);

    if( not fi.exists()) {
        qCritical() << "failed to write template files";
        return false;
    } else {
        qInfo() << "newly created from resource: " << file;
        return true;
    }
}

namespace {

double cm2Pt(double cm) { return cm * 28.3465; }

} // namespace


QString mustachReplace(const QString &templateFileName, const QVariantMap &data)
{   LOG_CALL;
    Mustache::QtVariantContext context(data);
    Mustache::Renderer renderer;

    QString Content{ fileToString (appConfig::Outdir() + "/vorlagen/" + templateFileName)};
    // We need 3 passes to replace all mustache variables.
    for (int pass = 1; pass <= 3; pass++)
        Content = renderer.render( Content, &context);

    return Content;
}

bool writeRenderedTemplate(const QString &templateFileName, const QString &outputFileName, const QVariantMap &data)
{   LOG_CALL;
    // get file extension from templateName
    QFileInfo fi(templateFileName);

    // render the content.
    QString renderedText = mustachReplace(templateFileName, data);
    // Write the html content to file. (e.g. for editing)
    stringToFile(renderedText, appConfig::Outdir() +qsl("/") +outputFileName);

    return true;
}

QString replaceExtension(const QString& pdfFileName, const QString& ext)
{   LOG_CALL;
    QString out {pdfFileName};
// Todo: replace only at the end
    out =out.replace (qsl(".pdf"), ext, Qt::CaseInsensitive);
    return out;
}

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
