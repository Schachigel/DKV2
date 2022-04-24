
#include "pch.h"
#include "appconfig.h"
#include "helper.h"
#include "helpersql.h"
#include "busycursor.h"
#include "dbstructure.h"
#include "dkdbhelper.h"
#include "wizopenornewdatabase.h"
#include "wiznewdatabase.h"
#include "opendatabase.h"

namespace
{
bool tryToUseDb(const QString dbPath) {

    if( not treat_DbIsAlreadyInUse_File(dbPath)) {
        expected_error (qsl("Datenbank %1 ist bereits geöffnet und kann nicht verwendet werden.").arg(dbPath));
        return false;
    }

    if( not checkSchema_ConvertIfneeded (dbPath)) {
        expected_error (qsl("Datenbank %1 kann nicht kovertiert werden.").arg(dbPath));;
        return false;
    }

    if( not open_databaseForApplication (dbPath)) {
        expected_error (qsl("Datenbank %1 kann nicht für die Anwendung geöffnet werden.").arg(dbPath));
        return false;
    }
    // propagate new open db to UI
    appConfig::setLastDb (dbPath);
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
                                                       ? zs30360 : zs_actact))) {
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
    if( not dbPath.isEmpty ()) {
        // there was a command line arguemnt and we assume it is a file path
        qInfo() << "Path from command line: " << dbPath;
        dbPath = QFileInfo (dbPath).canonicalFilePath ();
        if( dbPath.isEmpty ()) {
            qInfo() << "The given filename is invalid or the file does not exist";
            return false;
        } else
            return tryToUseDb(dbPath);
        // NOTE: we will not continue, if the command line was invalid
    }

    // no command line argument -> check registry
    dbPath =appConfig::LastDb();
    QString dbCanonicalPath {QFileInfo(dbPath).canonicalFilePath ()};

    if( dbPath.isEmpty ()) {
        QMessageBox::information (NULL, qsl("Fehler beim Öffnen der Datenbank"),
                                  qsl("Die zuletzt geöffnete Datenbank Datei wurde nicht gefunden"));
        // continue interactively
    } else {
        if( tryToUseDb(dbCanonicalPath)) {
            return true;
        } else {
            QMessageBox::information (NULL, qsl("Fehler beim Öffnen der Datenbank"),
                                      qsl("Die zuletzt geöffnete Datenbank konnte nicht geöffnet werden"));
            // continue interactivly
        }
    }

    // use UI to get a db name
    return askUserForNextDb();
}

bool openDB_MRU( const QString path)
{
    QString cPath = QFileInfo(path).canonicalFilePath ();
    if( cPath.isEmpty ()) {
        expected_error (qsl("Die ausgewählte Datei %1 existiert nicht mehr").arg(path));
        return false;
    }
    if( not tryToUseDb (path)) {
        QMessageBox::information (NULL, qsl("Schwerer Fehler"),
                                  qsl("Beim Öffnen der Datenbank %1 ist ein schwerwiegender Fehler aufgetreten. Die Anwendung wird beendet").arg(path));
        exit(1);
    }
    return true;
}
