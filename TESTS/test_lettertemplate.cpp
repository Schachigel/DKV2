#include <QtSql>
#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/sqlhelper.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/letterTemplate.h"

#include "test_lettertemplate.h"


test_letterTemplate::test_letterTemplate(QObject *p) : QObject(p)
{
}

void test_letterTemplate::initTestCase()
{
    initAdditionalTables();
}

void test_letterTemplate::init()
{LOG_ENTRY_and_EXIT;
    if (QFile::exists(filename))
        QFile::remove(filename);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(filename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());

}

void test_letterTemplate::cleanup()
{LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
}

void test_letterTemplate::test_save_letter_template()
{LOG_ENTRY_and_EXIT;
    letterTemplate temp(letterTemplate::templateId::JA_thesa);
    temp.saveTemplate(testCon);
    QVERIFY2(tableExists("Briefvorlagen", testCon), "save letter template: die Tabelle Briefvorlagen wurde nicht angelegt");
}

void test_letterTemplate::test_load_letter_template()
{LOG_ENTRY_and_EXIT;
    letterTemplate src(letterTemplate::templateId::JA_thesa);
    src.saveTemplate(testCon);
    letterTemplate dst;
    dst.loadTemplate(letterTemplate::templateId::JA_thesa, testCon);
    QVERIFY2(src == dst, "save letter template: die Tabelle Briefvorlagen wurde nicht angelegt");
}

void test_letterTemplate::test_applyPlaceholders()
{LOG_ENTRY_and_EXIT;
    QMap<int, QString> result;
    QMap<int, bool> res;
    letterTemplate tlate;
    tlate.Html().insert(0, "{{");            result.insert(  0, "..");     res.insert( 0, false);
    tlate.Html().insert( 1, " {{");          result.insert( 1, " ..");     res.insert( 1, false);
    tlate.Html().insert( 2, " {{ ");         result.insert( 2, " .. ");    res.insert( 2, false);
    tlate.Html().insert( 3, "}}");           result.insert( 3, "}}");      res.insert( 3, false);
    tlate.Html().insert( 4, "{{}}");         result.insert( 4, "");    res.insert( 4, false);
    tlate.Html().insert( 5, " {{}}");        result.insert( 5, " ");   res.insert( 5, false);
    tlate.Html().insert( 6, " {{ }}");       result.insert( 6, " ");  res.insert( 6, false);
    tlate.Html().insert( 7, "{{a}}");        result.insert( 7, "i");       res.insert( 7, true);
    tlate.Html().insert( 8, " {{a}}");       result.insert( 8, " i");      res.insert( 8, true);
    tlate.Html().insert( 9, "{{a}} ");       result.insert( 9, "i ");      res.insert( 9, true);
    tlate.Html().insert(10, " {{a}} ");      result.insert(10, " i ");     res.insert(10, true);
    tlate.Html().insert(11, " {{a}}{{b}} "); result.insert(11, " ix ");    res.insert(11, true);
    tlate.Html().insert(12, " {{a}} {{b}} "); result.insert(12, " i x ");  res.insert(12, true);
    tlate.Html().insert(13, " {{ {{a}} b}} "); result.insert(13, "  b}} ");  res.insert(13, false);

    tlate.setPlaceholder("a", "i");
    tlate.setPlaceholder("b", "x");
    tlate.applyPlaceholders();

    int testfails = 0;
    for( int i = 0; i < tlate.Html().count(); i++)
    {

        qDebug() << "comparing " << tlate.Html()[i] << " to " << result [i];
        if(  tlate.Html()[i] != result[i] )
            qDebug() << "applyPlaceholders failed in test " << i << ": " << tlate.Html()[i] << "!= " << result[i] << " (error #" << ++testfails << ")";
    }
    if( testfails) qDebug() << "applyPlacehoders had " << testfails << "errors";
    QVERIFY(testfails == 0);
}
