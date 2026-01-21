#ifndef TESTHELPER_H
#define TESTHELPER_H

#include "../DKV2/helper_core.h"

struct timerData {
    qint64 timeInMs = 0;
    QString message;
};

class dbgTimer
{
public:
    QVector<timerData> labs;
    timerData final;

    enum class initialState : bool { stopped = 0, running = 1 };

    explicit dbgTimer(QString name = {}, initialState is = initialState::running)
        : fname(std::move(name))
    {
        if (is == initialState::running)
            start(); // start new run immediately
    }

    dbgTimer(const dbgTimer&) = delete;
    dbgTimer& operator=(const dbgTimer&) = delete;

    ~dbgTimer() noexcept
    {
        // bewusst: KEIN Output im dtor. Nur finalisieren.
        end();
    }

    // A) start() begins a new run (resets stored data)
    void start(const QString& newName = {})
    {
        if (!newName.isEmpty())
            fname = newName;

        started = true;
        done = false;
        dumped = false;

        labs.clear();
        final = {};

        labcount = 0;
        lastLab = 0;

        t.start();
    }

    void end()
    {
        if (done) return;
        done = true;

        Q_ASSERT(started);
        if (!started) return;

        final.timeInMs = t.elapsed();
        final.message =
            QLatin1String("Timer ")
            % fname
            % QLatin1String(" ended. Elapsed Time: ")
            % QString::number(final.timeInMs)
            % QLatin1String(" ms");
    }

    void lab(const QString& msg = {})
    {
        Q_ASSERT(started);
        if (!started || done) return;

        const qint64 now = t.elapsed();
        const qint64 labTime = now - lastLab;
        lastLab = now;

        const QString labMsg =
            QLatin1String("Lab# ")
            % QString::number(labcount++)
            % QLatin1String(" (")
            % msg
            % QLatin1String("): ")
            % QString::number(labTime)
            % QLatin1String(" ms (overall: ")
            % QString::number(now)
            % QLatin1String(" ms)");
       labs.push_back({ labTime, labMsg });
    }

    void addLab(qint64 ms, const QString& message)
    {
        labs.push_back({ ms, message });
    }

    void dump()
    {
        if (dumped) return;
        dumped = true;

        end();

        for (const auto& lab : std::as_const(labs))
            qInfo().noquote() << lab.message;

        if (!final.message.isEmpty())
            qInfo().noquote() << final.message;
    }

private:
    QElapsedTimer t;
    QString fname;
    qint64 labcount {0};
    qint64 lastLab {0};

    bool started {false};
    bool done {false};
    bool dumped {false};
};

inline const QString testDbFilename {qsl("./data/testdb.sqlite")};
inline const QString testTemplateDb {qsl("./data/template.sqlite")};

void getRidOfFile(QString fn);
void initTestDkDb_InMemory();
void initTestDkDb();
void createTestDkDb_wData();
void createTestDkDbTemplate();
void cleanupTestDkDbTemplate();
void initTestDkDbFromTemplate();

void createTestDb_withRandomData();
void cleanupTestDb_InMemory();
void cleanupTestDkDb();
void openDefaultDbConnection_InMemory();
void openDefaultDbConnection(QString file =testDbFilename);
void closeDefaultDbConnection();
void createEmptyFile(const QString& fn);
//int tableRecordCount(const QString& table, const QSqlDatabase& db =QSqlDatabase::database());
bool dbHasTable(const QString& tname, const QSqlDatabase& db =QSqlDatabase::database());
bool dbTableHasField(const QString& tname, const QString& fname, const QSqlDatabase &db =QSqlDatabase::database());
bool dbsHaveSameTables(const QString &fn1, const QString &fn2);
bool dbsHaveSameTables(const QSqlDatabase& db1, const QSqlDatabase& db2);
bool dbTablesHaveSameFields(const QString& table1, const QString& table2, const QSqlDatabase& db =QSqlDatabase::database());

int doAnnualSettlementAllContracts (int year);
bool dbCompare(const QString& left, const QString& right);

#define dbgDumpDB() dbgDumpDatabase(QString(__FUNCTION__));
void dbgDumpDatabase(const QString &testname);

#endif // TESTHELPER_H
