#include "opendatabase.h"

#include "helper_core.h"
#include "dkdbhelper.h"
#include "dkdbcopy.h"
#include "appconfig.h"
#include "dbstructure.h"
#include "helperfile.h"
#include "dkv2version.h"
#include "busycursor.h"

#include "wizopenornewdatabase.h"
#include "wiznewdatabase.h"


bool checkSchema_ConvertIfneeded(const QString &origDbFile)
{
    LOG_CALL;
    busyCursor bc;
    int version_of_original_file = get_db_version(origDbFile);
    if (version_of_original_file < CURRENT_DB_VERSION)
    {
        qInfo() << "lower version -> converting";
        bc.finish (); // normal cursor during message box
        if (QMessageBox::Yes not_eq QMessageBox::question(getMainWindow(), qsl("Achtung"), qsl("Das Format der Datenbank \n%1\nist veraltet.\nSoll die Datenbank konvertiert werden?").arg(origDbFile)))
            RETURN_OK(false, qsl("conversion rejected by user"));

        QString backup = convert_database_inplace(origDbFile);
        if (backup.isEmpty()) {
            bc.finish (); // normal cursor during message box
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            RETURN_ERR(false, qsl("db converstion of older DB failed"));
        }
        // actions which depend on the source version of the db
        if (not postDB_UpgradeActions(version_of_original_file, origDbFile)) {
            bc.finish (); // normal cursor during message box
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            RETURN_ERR(false, qsl("db converstion of older DB failed"));
        }
        //        bc.finish ();
        QMessageBox::information(nullptr, qsl("Erfolgsmeldung"), qsl("Die Konvertierung ware erfolgreich. Eine Kopie der ursprünglichen Datei liegt unter \n") + backup);
        return true;
    }

    if (version_of_original_file == CURRENT_DB_VERSION)
        return validateDbSchema(origDbFile, dkdbstructur);

    // VERSION is higher?!
    RETURN_ERR(false, qsl("higher version ? there is no way back"));
}

namespace {

// ToDo: move to other file in GUI, move rest of file to core
bool treat_DbIsAlreadyInUse_File(QString filename)
{
    QMessageBox::StandardButton result = QMessageBox::NoButton;

    while (checkSignalFile(filename))
    {
        result = QMessageBox::information((QWidget *)nullptr, qsl("Datenbank bereits geöffnet?"),
                                          qsl("Es scheint, als sei die Datenbank\n%1\n bereits geöffnet. Das kommt vor, "
                                              "wenn DKV2 abgestürzt ist oder bereits läuft.\n"
                                              "Falls die Datenbank auf einem Fileserver läuft, kann auch eine "
                                              "andere Benutzerin die Datenbank gerade verwenden.\n"
                                              "\nIgnore: Wenn du sicher bist, dass kein anderes "
                                              "Programm läuft. (auf eigene Gefahr!)"
                                              "\nCancel: Um eine andere Datenbank zu wählen."
                                              "\nRetry: Wenn das andere Programm beendet ist."
                                              " ").arg(filename),
                                          QMessageBox::Cancel | QMessageBox::Retry | QMessageBox::Ignore);

        if (result == QMessageBox::Cancel)  {
            return false;
        } else if (result == QMessageBox::Ignore) {
            /* QMessageBox::Ignore leaves the file check loop */
            break;
        } else {
            /* QMessageBox::Retry and other repeats the file check */
        }
    }
    createSignalFile (filename);
    return true;
}


bool tryToUseDb(const QString dbPath) {

    if( not treat_DbIsAlreadyInUse_File(dbPath)) {
        qInfo() << qsl("Datenbank %1 ist bereits geöffnet und kann nicht verwendet werden.").arg(dbPath);
        return false;
    }

    if( not checkSchema_ConvertIfneeded (dbPath)) {
        expected_error (qsl("Datenbank %1 kann nicht kovertiert werden.").arg(dbPath));;
        return false;
    }
    // signal that the current db is disconected
    appconfig::delLastDb ();

    if( not open_databaseForApplication (dbPath)) {
        expected_error (qsl("Datenbank %1 kann nicht für die Anwendung geöffnet werden.").arg(dbPath));
        return false;
    }
    // propagate new open db to UI
    appconfig::setLastDb (dbPath);
    qInfo() << qsl("Datenbank %1 wurde erfolgreich für die Anwendung geöffnet.").arg(dbPath);
    return true;
    // db was loaded successfully
}
} // anonymous namespace

bool askUserForNextDb() {
    wizOpenOrNewDb wizOpenOrNew;

    if( QDialog::Accepted not_eq wizOpenOrNew.exec()) {
        qInfo() << "wizard OpenOrNew was canceled by the user";
        return false;
    }

    if( wizOpenOrNew.field(fnCreateNew).toBool ()) {
        if( createNewDatabaseFileWDefaultContent (wizOpenOrNew.selectedFile,
                                                      (wizOpenOrNew.field(qsl("Zinssusance")).toBool()
                                                       ? zs_30360 : zs_actact))) {
            wizConfigureNewDatabaseWiz wizProjectData;
            if( wizProjectData.Accepted not_eq wizProjectData.exec())
                qInfo() << "Project configuration failed or was aborted";

            wizProjectData.updateDbConfig(wizOpenOrNew.selectedFile);
            return tryToUseDb (wizOpenOrNew.selectedFile);
        } else {
                qInfo() << "existing file " << wizOpenOrNew.selectedFile << " was selected";
                return tryToUseDb (wizOpenOrNew.selectedFile);
        }
    } else { // an existing db is to be opened
        return tryToUseDb (wizOpenOrNew.selectedFile);
    }
}

bool openDB_atStartup()
{
    // was a path provided by command line?
    QString dbPath {getDbFileFromCommandline()};
    if( dbPath.size()) {
        // there was a command line arguemnt and we assume it is a file path
        qInfo() << "Path from command line: " << dbPath;
        dbPath = QFileInfo (dbPath).canonicalFilePath ();
        if( dbPath.isEmpty ()) {
            expected_error(qsl("Die an der Kommandozeile angegebene Datei %1 existiert nicht. Die Ausführung des Programms wird beendet").arg(dbPath));
            return false;
        } else
            return tryToUseDb(dbPath);
        // NOTE: we will not continue, if the command line was invalid
    }

    // no command line argument -> check registry
    if( not appconfig::hasLastDb ())
        return askUserForNextDb ();
    dbPath =appconfig::LastDb();
    QString dbCanonicalPath {QFileInfo(dbPath).canonicalFilePath ()};
    if( dbCanonicalPath.isEmpty ()) {
        appconfig::delLastDb (); // do not retry next time
        expected_error(qsl("Die zuletzt geöffnete Datenbank Datei %1 wurde nicht gefunden").arg(dbPath));
        // continue interactively
    } else {
        if( tryToUseDb(dbCanonicalPath))
            return true;
        appconfig::delLastDb (); // do not retry next time
        expected_error(qsl("Die zuletzt geöffnete Datenbank konnte nicht geöffnet werden"));
        // continue interactivly
    }
    // use UI to get a db name
    return askUserForNextDb();
}

bool openDB_MRU( const QString path)
{
    QString cPath = QFileInfo(path).canonicalFilePath ();
    if( tryToUseDb (cPath))
        return true;
    if( appconfig::LastDb().isEmpty()){
        expected_error(qsl("Beim Öffnen der Datenbank %1 ist ein schwerwiegender Fehler aufgetreten. Die Anwendung wird beendet").arg(path));
        exit(1);
    } else {
        expected_error(qsl("Beim Öffnen der Datenbank %1 ist ein Fehler aufgetretet. Die zuletzt geöffnete Datenbank ist noch geöffnet").arg(path));
        return true;
    }
}
