#include "filewriter.h"

#include <QPdfWriter>

#include "helper_core.h"
#include "helperfile.h"
#include "appconfig.h"
#include "mustache.h"

QString appendFilenameToOutputDir(QString filename)
{
    QDir().mkdir(appconfig::Outdir());
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
    if( not file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
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
    if (fi.exists() && fi.size() > 0) {
        qInfo() << "existing template file was not overwritten: " << outname;
        return true;
    }

    QFile resource(qsl(":/res/")+file);
    if( not resource.open(QIODevice::ReadOnly)){
        qCritical() << "extractTemplateFileFromResource: failed to open resource"
                    << (qsl(":/res/") + file) << "-" << resource.errorString();
        return false;
    }
    QByteArray br =resource.readAll ();

    QFile target (fi.absoluteFilePath ());
    if( not target.open(QIODevice::WriteOnly)) {
        qCritical() << "extractTemplateFileFromResource: failed to open target"
                    << fi.absoluteFilePath() << "-" << target.errorString();
        return false;
    }
    const qint64 bytesWritten = target.write(br);
    if (bytesWritten < 0) {
        qCritical() << "extractTemplateFileFromResource: failed to write target"
                    << fi.absoluteFilePath() << "-" << target.errorString();
        return false;
    }
    if (bytesWritten != br.size()) {
        qCritical() << "extractTemplateFileFromResource: incomplete write"
                    << fi.absoluteFilePath() << "- wrote" << bytesWritten
                    << "of" << br.size() << "bytes";
        return false;
    }
    if (!target.flush()) {
        qCritical() << "extractTemplateFileFromResource: failed to flush target"
                    << fi.absoluteFilePath() << "-" << target.errorString();
        return false;
    }
    target.close();

    fi.refresh();
    if (!fi.exists() || fi.size() <= 0) {
        qCritical() << "extractTemplateFileFromResource: created template is missing or empty"
                    << fi.absoluteFilePath();
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

    const QString outdirTemplatePath = appendFilenameToOutputDir(qsl("/vorlagen/") + templateFileName);
    QString content{readFileToString(outdirTemplatePath)};
    if (content == qsl("file open error")) {
        qWarning() << "mustachReplace: failed to load template from outdir, trying resource:"
                   << outdirTemplatePath;
        QFile fallbackTemplate(qsl(":/res/") + templateFileName);
        if (fallbackTemplate.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream ts(&fallbackTemplate);
            ts.setEncoding(QStringConverter::Utf8);
            ts.setAutoDetectUnicode(true);
            content = ts.readAll();
            qWarning() << "template loaded from resources fallback:" << templateFileName;
        } else {
            qCritical() << "mustachReplace: template missing in outdir and resources:"
                        << templateFileName
                        << "| outdir path:" << outdirTemplatePath
                        << "| resource path:" << (qsl(":/res/") + templateFileName);
            return QString();
        }
    }
    // We need 3 passes to replace all mustache variables.
    for (int pass = 1; pass <= 3; pass++)
        content = renderer.render(content, &context);

    return content;
}
} // namespace



bool writeRenderedTemplate(const QString &templateFileName, const QString &outputFileName, const QVariantMap &data)
{   LOG_CALL;
    // render the content.
    QString renderedText = mustachReplace(templateFileName, data);
    // Write the html content to file. (e.g. for editing)
    return !saveStringToUtf8File(outputFileName, renderedText).isEmpty();
}

bool savePdfFromHtmlTemplate(const QString &templateFileName, const QString &outputFileName, const QVariantMap &data)
{   LOG_CALL;
    QFileInfo fi(outputFileName);
    QString fullOutputFileName{outputFileName};
    if (fi.isRelative())
        fullOutputFileName = appendFilenameToOutputDir(outputFileName);

    QString css{readFileToString(appendFilenameToOutputDir(qsl("vorlagen/zinsbrief.css")))};
    if (css == qsl("file open error")) {
        css.clear();
    }
    if (css.isEmpty()) {
        css = readFileToString(appendFilenameToOutputDir(qsl("zinsbrief.css")));
    }
    if (css == qsl("file open error"))
        css.clear();
    // DEBUG   printHtmlToPdf(renderedHtml, css, htmlFileName);

    QFileInfo outInfo(fullOutputFileName);
    if (!outInfo.dir().exists() && !QDir().mkpath(outInfo.dir().absolutePath())) {
        qCritical() << "failed to create output folder for pdf:" << outInfo.dir().absolutePath();
        return false;
    }
    QFile outFile(fullOutputFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "failed to open pdf output file:" << fullOutputFileName << outFile.errorString();
        return false;
    }

    // Prepare PDF output without using printer discovery.
    QPdfWriter pdfWriter(&outFile);
    QPageLayout pl = pdfWriter.pageLayout();
    double leftB   = cm2Pt(3.); // breiter für Lochung
    double topB    = cm2Pt(1.); // logo darf in den oberen Rand reichen
    double rightB  = cm2Pt(0.); // logo darf in den Rand reichen
    double bottomB = cm2Pt(2.);
    pl.setPageSize (QPageSize(QPageSize::A4), QMargins(leftB, topB,rightB, bottomB));
    pdfWriter.setPageLayout(pl);

    //Prepare the document
    QTextDocument doc;
    QString renderedHtml = mustachReplace(templateFileName, data);
    if (renderedHtml.isEmpty()) {
        qCritical() << "rendered html is empty, skip pdf creation for" << outputFileName;
        return false;
    }
    doc.setPageSize(pl.pageSize ().size (QPageSize::Unit::Point));

    // render the content.
    doc.setDefaultStyleSheet (css);
    doc.setHtml(renderedHtml);
    // Write PDF directly (no printer required).
    doc.print(&pdfWriter);
    outFile.flush();
    outFile.close();
    if (QFileInfo(fullOutputFileName).size() <= 0) {
        qCritical() << "pdf file is empty after rendering:" << fullOutputFileName;
        return false;
    }

    // Write the html content to file. (just in case ... e.g. for editing)

    QString htmlFileName {
        qsl("html/") + QFileInfo(outputFileName).completeBaseName() + qsl(".html")
    };
    if (saveStringToUtf8File(htmlFileName, renderedHtml).isEmpty()) {
        qCritical() << "failed to write template";
        return false;
    }
    return true;
}
