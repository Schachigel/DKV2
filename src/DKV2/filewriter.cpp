#include "filewriter.h"

#include "helper_core.h"
#include "helperfile.h"
#include "appconfig.h"
#include "mustache.h"

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
    QFileInfo fi(outputFileName);
    QString fullOutputFileName{outputFileName};
    if (fi.isRelative())
        fullOutputFileName = appConfig::Outdir() + qsl("/") + outputFileName;

    // render the content.
    QString renderedText = mustachReplace(templateFileName, data);
    // Write the html content to file. (e.g. for editing)
    stringToFile(renderedText, appConfig::Outdir() +qsl("/") +outputFileName);

    return true;
}

QString replaceExtension(const QString& pdfFileName, const QString& ext)
{   LOG_CALL;

    QFileInfo fi(pdfFileName);
    QString newPath = fi.path() + "/" + fi.completeBaseName() + ext;

    return newPath;
}

