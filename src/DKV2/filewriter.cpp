#include "filewriter.h"

#include <QPrinter>

#include "helper_core.h"
#include "helperfile.h"
#include "appconfig.h"
#include "mustache.h"

QString appendFilenameToOutputDir(QString filename)
{
    if( filename.startsWith("/"))
        return appconfig::Outdir() + filename;
    else
        return appconfig::Outdir() + qsl("/") + filename;
}

QString saveStringToUtf8File(const QString& filename, const QString& content)
{   LOG_CALL_W(filename);

    QString fqFilePath {appendFilenameToOutputDir(filename)};
    moveToBackup (fqFilePath);

    QFile file(fqFilePath);
    if( not file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)) {
        qCritical() << "File Open of " << fqFilePath << " failed: " << file.errorString();
        return QString();
    }
        //RETURN_ERR(false, qsl("could not open csv file for writing: "), filename);

    QTextStream s(&file);
    s.setEncoding(QStringConverter::Utf8);
    s.setGenerateByteOrderMark(true);
    s << content;
    if( s.status() == QTextStream::Ok)
        return fqFilePath;
    else{
        qCritical() << qsl("writing file %1 failed in TextSteam").arg(fqFilePath);
        return QString();
    }
}

bool extractTemplateFileFromResource(const QString& path, const QString& file, const QString& outname)
{   LOG_CALL_W(file);
    QFileInfo fi( QDir(path), outname.isEmpty() ? file : outname);
    if( fi.exists ()) {
        qInfo() << "existing template file was not overwritten: " << outname;
        return true;
    }

    QFile resource(qsl(":/res/")+file);
    if( not resource.open(QIODevice::ReadOnly)){
        qCritical() << "resource could not be opened from file";
        return false;
    }
    QByteArray br =resource.readAll ();

    QFile target (fi.absoluteFilePath ());
    if( not target.open(QIODevice::WriteOnly)) {
        qCritical() << "target of resource could not be written";
        return false;
    }
    if( -1 <= target.write (br)) {
        qCritical() << "failed to write to template file";
        return false;
    }

    if( not fi.exists()) {
        qCritical() << "failed to write template files";
        return false;
    } else {
        qInfo() << "newly created from resource: " << file;
        return true;
    }
}

namespace {

QString mustachReplace(const QString &templateFileName, const QVariantMap &data)
{   LOG_CALL;
    Mustache::QtVariantContext context(data);
    Mustache::Renderer renderer;

    QString Content{ readFileToString (appendFilenameToOutputDir(qsl("/vorlagen/") + templateFileName))};
    // We need 3 passes to replace all mustache variables.
    for (int pass = 1; pass <= 3; pass++)
        Content = renderer.render( Content, &context);

    return Content;
}
QString replaceExtension(const QString& pdfFileName, const QString& ext)
{   LOG_CALL;

    QFileInfo fi(pdfFileName);
    QString newPath = fi.path() + "/" + fi.completeBaseName() + ext;

    return newPath;
}
} // namespace



bool writeRenderedTemplate(const QString &templateFileName, const QString &outputFileName, const QVariantMap &data)
{   LOG_CALL;
    // get file extension from templateName
    QFileInfo fi(outputFileName);
    QString fullOutputFileName{outputFileName};
    if (fi.isRelative())
        fullOutputFileName = appendFilenameToOutputDir( outputFileName);

    // render the content.
    QString renderedText = mustachReplace(templateFileName, data);
    // Write the html content to file. (e.g. for editing)
    saveStringToUtf8File(renderedText, appendFilenameToOutputDir(outputFileName));

    return true;
}

bool savePdfFromHtmlTemplate(const QString &templateFileName, const QString &outputFileName, const QVariantMap &data)
{   LOG_CALL;
    QFileInfo fi( outputFileName);
    QString css{ readFileToString (appendFilenameToOutputDir(qsl("zinsbrief.css")))};
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
    printer.setOutputFileName(outputFileName);

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

    QString htmlFileName {appendFilenameToOutputDir(qsl("html/") +replaceExtension(outputFileName, qsl(".html")))};
    if( saveStringToUtf8File( renderedHtml, outputFileName).isEmpty()){
        qCritical() << "failed to write template";
        return false;
    }
    return true;
}
