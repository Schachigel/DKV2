#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class db : public QObject
{
    Q_OBJECT

public:
    db();
    ~db();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

db::db()
{

}

db::~db()
{

}

void db::initTestCase()
{

}

void db::cleanupTestCase()
{

}

void db::test_case1()
{

}

QTEST_MAIN(db)

#include "tst_db.moc"
