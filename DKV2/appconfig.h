#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <Qt>
#include <QMap>
#include <QWidget>

struct appConfig
{
//    static void setTestmode()
//    {   /* not nice to have test specific code but tests have to stay noninteractive */
//        testmode = true;
//    }
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
    static void setRuntimeData( const QString& name, const QString& value);
    static QString getRuntimeData( const QString& name, const QString& defaultvalue ="");
};

#endif // APPCONFIG_H
