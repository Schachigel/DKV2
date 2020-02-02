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
