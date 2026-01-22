#include "test_properties.h"

#include "../DKV2/appconfig.h"

#include "testhelper.h"

#include <QtTest/QTest>

void test_properties::init()
{
     initTestDkDb_InMemory ();
}

void test_properties::cleanup()
{
    cleanupTestDb_InMemory ();
}

void test_properties::test_setProperty_getProperty()
{
    setMetaInfo("Hallo", "Welt");
    QCOMPARE(getMetaInfo("Hallo"), "Welt");
}
void test_properties::test_initProperty_getProperty()
{
    initMetaInfo("Hallo", "Welt");
    QCOMPARE(getMetaInfo("Hallo"), "Welt");
}
void test_properties::test_set_init_getProperty()
{
    setMetaInfo("Hallo", "Weltall");
    initMetaInfo("Hallo", "Welt");
    QCOMPARE(getMetaInfo("Hallo"), "Weltall");
}
void test_properties::test_get_uninit()
{
    QCOMPARE(getMetaInfo("Hallo"),"");
}
void test_properties::test_set_get_num()
{
    setNumMetaInfo("Pi", 3.1415);
    QCOMPARE(getNumMetaInfo("Pi")*10000, 31415);
}
void test_properties::test_init_get_num()
{
    initNumMetaInfo("twoPi", 6.243);
    QCOMPARE(getNumMetaInfo("twoPi"), 6.243);
}
void test_properties::test_set_init_get_num()
{
    setNumMetaInfo("three", 3.);
    initNumMetaInfo("three", 4);
    QCOMPARE( getNumMetaInfo("three"), 3);
}
void test_properties::test_get_uninit_num()
{
    QCOMPARE(getNumMetaInfo("nix"), 0.);
}

