#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QMap>
#include <QWidget>
#include <QSqlDatabase>

const QString DB_VERSION {"Version"};
const double CURRENT_DB_VERSION {2.0};
const QString GMBH_ADDRESS1 {"gmbh.address1"};
const QString GMBH_ADDRESS2 {"gmbh.address2"};
const QString GMBH_STREET {"gmbh.strasse"};
const QString GMBH_PLZ {"gmbh.plz"};
const QString GMBH_CITY{"gmbh.stadt"};
const QString GMBH_EMAIL {"gmbh.email"};
const QString GMBH_URL {"gmbh.url"};
const QString GMBH_PI {"gmbh.Projektinitialen"};
const QString STARTINDEX {"startindex"};
const QString MIN_PAYOUT {"minAuszahlung"};
const QString MIN_AMOUNT {"minVertragswert"};
const QString DBID {"dbId"};
const QString GMBH_HRE {"gmbh.Handelsregister"};
const QString GMBH_GEFUE1{"gmbh.gefue1"};
const QString GMBH_GEFUE2{"gmbh.gefue2"};
const QString GMBH_GEFUE3{"gmbh.gefue3"};
const QString GMBH_DKV{"gmbh.dkv"};

// db config info in 'meta' table
// init = write only if not set
void initMetaInfo( const QString& name, const QString& wert, QSqlDatabase db=QSqlDatabase::database());
void initNumMetaInfo( const QString& name, const double& wert, QSqlDatabase db=QSqlDatabase::database());
// reading
QString getMetaInfo(const QString& name, const QString& def="", QSqlDatabase db = QSqlDatabase::database());
double getNumMetaInfo(const QString& name, const double def =0., QSqlDatabase db = QSqlDatabase::database());
// writing
void setMetaInfo(const QString& name, const QString& value, QSqlDatabase db = QSqlDatabase::database());
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db = QSqlDatabase::database());

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

    static void setCurrentDb(const QString&);
    static QString CurrentDb();
    static void delCurrentDb();

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

    static QMap<QString, QString> runtimedata;
};

struct dbConfig
{
    enum src {
        FROM_DB =1,
        FROM_RTD=2
    };
    dbConfig() =default;
    dbConfig(src s) {
        if( s==FROM_DB) readDb();
        else loadRuntimeData();
    }
    // static dbConfig fromRuntimeData();
    void loadRuntimeData();
    void storeRuntimeData();
    // static dbConfig fromDb(QSqlDatabase db =QSqlDatabase::database());
    void readDb(QSqlDatabase db =QSqlDatabase::database());
    void writeDb(QSqlDatabase db =QSqlDatabase::database());
    // all config defaults come from here
    QString address1    ="Esperanza Franklin GmbH";
    QString address2    ="";
    QString street      ="Turley-Platz 9";
    QString plz         ="68167";
    QString city        ="Mannheim";
    QString email       ="info@esperanza-mannheim.de";
    QString url         ="www.esperanza-mannheim.de";
    QString pi          ="ESP"; // project initials
    QString hre         ="Amtsgericht Mannheim HRB HRB 7-----3";
    QString gefue1      ="...";
    QString gefue2      ="...";
    QString gefue3      ="...";
    QString dkv         ="...";
    int     startindex  =1234;
    int     minPayout   =100;
    int     minContract =500;
    QString dbId        ="ESP1234";
    double  dbVersion=CURRENT_DB_VERSION;

    inline friend bool operator!=(const dbConfig& lhs, const dbConfig& rhs) {
        return !(lhs==rhs);
    }

    inline friend bool operator==(const dbConfig& lhs, const dbConfig& rhs) {
        bool ret =false;
        do{
            if( lhs.address1 != rhs.address1)   break;
            if( lhs.address2 != rhs.address2)   break;
            if( lhs.street   != rhs.street)     break;
            if( lhs.plz      != rhs.plz)        break;
            if( lhs.city     != rhs.city)       break;
            if( lhs.email    != rhs.email)      break;
            if( lhs.url      != rhs.url)        break;
            if( lhs.pi       != rhs.pi)         break;
            if( lhs.hre      != rhs.hre)        break;
            if( lhs.gefue1   != rhs.gefue1)     break;
            if( lhs.gefue2   != rhs.gefue2)     break;
            if( lhs.gefue3   != rhs.gefue3)     break;
            if( lhs.dkv      != rhs.dkv)        break;
            if( lhs.startindex!= rhs.startindex)break;
            if( lhs.dbId     != rhs.dbId)       break;
            if( lhs.minPayout!= rhs.minPayout)  break;
            if( lhs.minContract!= rhs.minContract) break;
            if( lhs.dbVersion != rhs.dbVersion) break;
            ret =true;
        } while(false);
        return ret;
    }
};

#endif // APPCONFIG_H
