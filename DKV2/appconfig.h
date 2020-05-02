#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <Qt>
#include <QMap>

struct appConfig
{
    static void setTestmode()
    {   /* not nice to have test specific code - however we do not want the
            tests to modify the registry keys from the application  */
        keyOutdir = "test-" + keyOutdir;
        keyLastDb = "test-" + keyLastDb;
        keyCurrentDb = "test-" + keyCurrentDb;
    }
    static void setOutDir(const QString& od);
    static QString Outdir();

    static void setLastDb(const QString&);
    static QString LastDb();

    static void setCurrentDb(const QString&);
    static QString CurrentDb();

    // for testing only
    static void deleteUserData(const QString& name);
    static void deleteRuntimeData(const QString& name);
private:
    static QString keyOutdir;
    static QString keyLastDb;
    static QString keyCurrentDb;
    static void setUserData(const QString& name, const QString& value);
    static QString getUserData( const QString& name, const QString& defaultvalue ="");
    // QString getNumUserData(QString name);

    static QMap<QString, QString> runtimedata;
    static void setRuntimeData( const QString& name, const QString& value);
    static QString getRuntimeData( const QString& name, const QString& defaultvalue ="");
};

#endif // APPCONFIG_H
