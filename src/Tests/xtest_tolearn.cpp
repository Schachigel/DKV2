#include "xtest_tolearn.h"
#include "../DKV2/helper.h"

void test_toLearn::test_sql_with_parameter_binding()
{
    {
        QSqlDatabase db =QSqlDatabase::addDatabase("QSQLITE");

        db.setDatabaseName(qsl(":memory:"));
        db.open();
        // clazy:exclude=unused-non-trivial-variable
        QString sqlCreateTable =QStringLiteral("CREATE TABLE testt (s TEXT, i INTEGER)");
        QSqlQuery qCreate; qCreate.prepare(sqlCreateTable);
        qCreate.exec();
    }
    // positional binding works
    {
        QString sqlInsertData =QStringLiteral("INSERT INTO testt (s, i) VALUES (?, ?)");
        QSqlQuery q;
        q.prepare (sqlInsertData);
        q.addBindValue (QVariant("text1"));
        q.addBindValue (QVariant(13));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
        q.addBindValue (QVariant("text2"));
        q.addBindValue (QVariant(14));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
    }
    // positional binding works with named parameter
    {
        QString sqlInsertData =QStringLiteral("INSERT INTO testt (s, i) VALUES (:p1, :p2)");
        QSqlQuery q;
        q.prepare (sqlInsertData);
        q.addBindValue (QVariant("text1"));
        q.addBindValue (QVariant(13));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
        q.addBindValue (QVariant("text2"));
        q.addBindValue (QVariant(14));
        if( not q.exec ())
            qInfo() << sqlInsertData << q.boundValues () << q.lastError () << q.lastQuery ();
    }
    // How about named binding?
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=:val1 AND i=:val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue (":val1", QVariant("text2"));
        qSelect.bindValue (":val2", QVariant(14));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qInfo() << qSelect.lastQuery ();
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
    // change order: OK!
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=:val1 AND i=:val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue (":val2", QVariant(14));
        qSelect.bindValue (":val1", QVariant("text2"));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
    // change placeholder mark to @ -> will not work
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=@val1 AND i=@val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue ("@val1", QVariant("text2"));
        qSelect.bindValue ("@val2", QVariant(14));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
    // change placeholder mark to $ -> will not work
    {
        QString sqlSelect =QStringLiteral("SELECT * FROM testt WHERE s=$val1 AND i=$val2");
        QSqlQuery qSelect;
        qSelect.prepare (sqlSelect);
        qSelect.bindValue ("$val1", QVariant("text2"));
        qSelect.bindValue ("$val2", QVariant(14));
        qInfo() << qSelect.boundValues ();
        if( qSelect.exec ()) {
            qSelect.first();
            qInfo() << qSelect.record ().value (0);
        } else
            qInfo() << sqlSelect << qSelect.boundValues () << qSelect.lastError () << qSelect.lastQuery ();
    }
}
