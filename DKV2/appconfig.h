#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QStringLiteral>
#include <QVariant>
#include <QMap>
#include <QWidget>
#include <QSqlDatabase>

#include "helper.h"

extern const double CURRENT_DB_VERSION;

// db config info in 'meta' table
// init = write only if not set
void initMetaInfo( const QString& name, const QString& wert, QSqlDatabase db=QSqlDatabase::database());
void initNumMetaInfo( const QString& name, const double& wert, QSqlDatabase db=QSqlDatabase::database());
// reading
QString getMetaInfo(const QString& name, const QString& def=QString(), QSqlDatabase db = QSqlDatabase::database());
double getNumMetaInfo(const QString& name, const double def =0., QSqlDatabase db = QSqlDatabase::database());
// writing
void setMetaInfo(const QString& name, const QString& value, QSqlDatabase db = QSqlDatabase::database());
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db = QSqlDatabase::database(), const QString& tblAlias =QString());

struct appConfig
{
    // global (on program / system level, stored in system registry)
    static void setOutDir(const QString& od);
    static void setOutDirInteractive(QWidget* parent =nullptr);
    static QString Outdir();
    static void delOutDir();

    static void setLastDb(const QString&);
    static QString LastDb();
    static void delLastDb();

    // dynamic config data stored in memory
    static void setRuntimeData( const QString& name, const QString& value);
    static QString getRuntimeData( const QString& name, const QString& defaultvalue ="");

    // for testing only
    static void deleteUserData(const QString& name);
    static void deleteRuntimeData(const QString& name);
private:
    static bool testmode;
    static QString keyOutdir;
    static QString keyLastDb;
    static QString keyCurrentDb;
    static void setUserData(const QString& name, const QString& value);
    static QString getUserData( const QString& name, const QString& defaultvalue ="");
    // QString getNumUserData(QString name);
};
enum projectConfiguration {
   DB_VERSION =0,DKV2_VERSION,
    GMBH_ADDRESS1, GMBH_ADDRESS2, GMBH_STREET, GMBH_PLZ, GMBH_CITY,
   GMBH_EMAIL, GMBH_URL, GMBH_INITIALS,
   STARTINDEX, MIN_PAYOUT, MIN_AMOUNT, MAX_INTEREST,
   DBID,
   GMBH_HRE, GMBH_GEFUE1, GMBH_GEFUE2, GMBH_GEFUE3, GMBH_DKV,
   MAX_INVESTMENT_NBR, MAX_INVESTMENT_SUM,
   MAX_PC_INDEX
};


struct dbConfig
{
    dbConfig() =delete;
    static QVariant readVersion(const QSqlDatabase& db =QSqlDatabase::database());
    static void writeVersion(const QSqlDatabase& db =QSqlDatabase::database(), const QString& tblAlias =QString());
    static QVariant readValue(const projectConfiguration& pc, const QSqlDatabase& db =QSqlDatabase::database());
    static void     writeValue(const projectConfiguration& pc, const QVariant& value, const QSqlDatabase& db =QSqlDatabase::database(), const QString& tblAlias =QString());

    static QString paramName(const projectConfiguration& pc) {
        return defaultParams.value(pc).first;
    }
    static void writeDefaults(const QSqlDatabase& db =QSqlDatabase::database());

private:
    static QMap<projectConfiguration,QPair<QString, QVariant>> defaultParams;
    static bool isValidIndex(projectConfiguration pc) {
        return (pc >= 0) && (pc < projectConfiguration::MAX_PC_INDEX);
    }
};

#endif // APPCONFIG_H
