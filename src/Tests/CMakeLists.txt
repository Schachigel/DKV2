cmake_minimum_required(VERSION 3.21)

# für github actions: es ist ausreichend, nur test_all soll auszuführen
# die einzelnen Test exe sind zum debuggen
option(DKV2_CTEST_ONLY_TEST_ALL "Register only test_all in CTest" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test Sql Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

set(DKV2_TEST_QT_LIBS Qt6::Test)

set(DKV2_TEST_COMMON_SOURCES
  testhelper.h
  testhelper.cpp
)

set(DKV2_TEST_PCH
  ${CMAKE_CURRENT_SOURCE_DIR}/../DKV2/pch_core.h
)

#
#
#   !! ADD NEW TESTS HERE !!
#
# Format per row:
#   target_name|class_name|main_header|file4|file5|...
set(DKV2_TEST_SPECS
  "test_appconfig|test_appconfig|test_appconfig.h|test_appconfig.cpp"
  "test_annualSettlement|test_annualSettlement|test_annualSettlement.h|test_annualSettlement.cpp"
  "test_booking|test_booking|test_booking.h|test_booking.cpp"
  "test_contract|test_contract|test_contract.h|test_contract.cpp"
  "test_creditor|test_creditor|test_creditor.h|test_creditor.cpp"
  "test_csv|test_csv|test_csv.h|test_csv.cpp"
  "test_db|test_db|test_db.h|test_db.cpp"
  "test_dbfield|test_dbfield|test_dbfield.h|test_dbfield.cpp"
  "test_dkdbcopy|test_dkdbcopy|test_dkdbcopy.h|test_dkdbcopy.cpp"
  "test_dkdbhelper|test_dkdbhelper|test_dkdbhelper.h|test_dkdbhelper.cpp"
  "test_finance|test_finance|test_finance.h|financaltimespan.h|test_finance.cpp|financaltimespan.cpp"
  "test_properties|test_properties|test_properties.h|test_properties.cpp"
  "test_sqlhelper|test_sqlhelper|test_sqlhelper.h|test_sqlhelper.cpp"
  "test_statistics|test_statistics|test_statistics.h|test_statistics.cpp"
  "test_tabledatainserter|test_tableDataInserter|test_tabledatainserter.h|test_tabledatainserter.cpp"
  "test_views|test_views|test_views.h|test_views.cpp"
  "test_tolearn|test_tolearn|test_tolearn.h|test_tolearn.cpp"
  # add more rows here ...
)
#
#
#

function(_dkv2_parse_test_spec spec out_target out_class out_main_header out_files)
  string(REPLACE "|" ";" _parts "${spec}")

  list(LENGTH _parts _len)
  if(_len LESS 4)
    message(FATAL_ERROR "Invalid test spec '${spec}'. Need: target|class|main_header|file...")
  endif()

  list(GET _parts 0 _target)
  list(GET _parts 1 _class)
  list(GET _parts 2 _main_header)
  list(REMOVE_AT _parts 0 1 2) # remaining are files (besides main_header)

  set(${out_target}      "${_target}"      PARENT_SCOPE)
  set(${out_class}       "${_class}"       PARENT_SCOPE)
  set(${out_main_header} "${_main_header}" PARENT_SCOPE)
  set(${out_files}       "${_parts}"       PARENT_SCOPE)
endfunction()

function(_dkv2_write_single_main out_path class_name main_header)
  file(WRITE "${out_path}" "
#include <QCoreApplication>
#include <QtTest>

#include \"../DKV2/appconfig.h\"
#include \"${main_header}\"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Required for tests that indirectly use QSettings
    QCoreApplication::setOrganizationName(QStringLiteral(\"HoMsoft\"));
    QCoreApplication::setApplicationName(QStringLiteral(\"DKV2_All_Tests\"));

    // default: run tests in testmode; keep appconfig tests in normal mode
    if (QStringLiteral(\"${class_name}\") != QStringLiteral(\"test_appconfig\"))
        appconfig::setTestMode(true);

    ${class_name} tc;
    return QTest::qExec(&tc, argc, argv);
}
")
endfunction()

function(_dkv2_write_runall_main out_path class_names main_headers)
  set(_inc "")
  set(_run_table "")

  foreach(_h IN LISTS main_headers)
    string(APPEND _inc "#include \"${_h}\"\n")
  endforeach()

  foreach(_c IN LISTS class_names)
    # Each entry: { "ClassName", &runTest<ClassName> }
    string(APPEND _run_table "        { \"${_c}\", &runTest<${_c}> },\n")
  endforeach()

  file(WRITE "${out_path}" "
#include <QCoreApplication>
#include <QtTest>

#include <vector>
#include <algorithm>
#include <random>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#include <QStringBuilder>

#include \"testhelper.h\"   // dbgTimer (+ addLab)
#include \"../DKV2/appconfig.h\"

${_inc}

using TestFn = void(*)(int&, int, char**);

template<typename T>
static void runTest(int& rc, int argc, char** argv)
{
    T tc;
    rc |= QTest::qExec(&tc, argc, argv);
}

struct TestEntry
{
    const char* name;
    TestFn fn;
};

static int parseIntArg(int argc, char** argv, const char* key, int defaultValue)
{
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], key) == 0) {
            const int v = std::atoi(argv[i + 1]);
            return (v >= 1) ? v : 1;
        }
    }
    return defaultValue;
}

static std::uint32_t parseSeedArg(int argc, char** argv, const char* key, std::uint32_t defaultValue, bool& hasSeed)
{
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], key) == 0) {
            hasSeed = true;
            return static_cast<std::uint32_t>(std::strtoul(argv[i + 1], nullptr, 10));
        }
    }
    hasSeed = false;
    return defaultValue;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral(\"HoMsoft\"));
    QCoreApplication::setApplicationName(QStringLiteral(\"DKV2_All_Tests\"));
    appconfig::setTestMode(true);

    std::vector<TestEntry> tests = {
${_run_table}
    };
    // at 2026-01 about 2,3s per run on powerfull workstation
    const int repetitions = parseIntArg(argc, argv, \"--repeat\", 3);

    bool hasSeed = false;
    const std::uint32_t seedArg = parseSeedArg(argc, argv, \"--seed\", 0u, hasSeed);
    const std::uint32_t effectiveSeed = hasSeed ? seedArg : std::random_device{}();

    qInfo().noquote()
        << (QLatin1String(\"test_all: repetitions = \") % QString::number(repetitions));

    qInfo().noquote()
        << (QLatin1String(\"test_all: RNG seed = \")
            % QString::number(effectiveSeed)
            % QLatin1String(hasSeed ? \" (fixed)\" : \" (random)\")); // ok: const char* literal

    std::mt19937 rng{ effectiveSeed };

    int rc = 0;

    dbgTimer totalTimer(QStringLiteral(\"test_all\"), dbgTimer::initialState::stopped);
    totalTimer.start(QStringLiteral(\"test_all: total duration\"));

    for (int i = 1; i <= repetitions; ++i) {
        dbgTimer roundTimer(QString(), dbgTimer::initialState::stopped);
        roundTimer.start(
            QLatin1String(\"test_all: round \")
            % QString::number(i)
            % QLatin1String(\" / \")
            % QString::number(repetitions)
        );

        std::shuffle(tests.begin(), tests.end(), rng);

        for (const auto& entry : tests) {
            dbgTimer singleTimer(QString(), dbgTimer::initialState::stopped);
            singleTimer.start(QLatin1String(\"test_all: \") % QLatin1String(entry.name));

            entry.fn(rc, argc, argv);

            singleTimer.end();

            // Add single-test as a Lab to the round (indented for readability)
            roundTimer.addLab(
                singleTimer.final.timeInMs,
                QLatin1String(\"  \")
                % QLatin1String(entry.name)
                % QLatin1String(\": \")
                % QString::number(singleTimer.final.timeInMs)
                % QLatin1String(\" ms\")
            );

            if (rc != 0) {
                // Fail-fast: finalize + dump the most relevant hierarchy
                roundTimer.end();
                roundTimer.dump();

                totalTimer.addLab(
                    roundTimer.final.timeInMs,
                    QLatin1String(\"round \")
                    % QString::number(i)
                    % QLatin1String(\" / \")
                    % QString::number(repetitions)
                    % QLatin1String(\": \")
                    % QString::number(roundTimer.final.timeInMs)
                    % QLatin1String(\" ms\")
                );

                totalTimer.end();
                totalTimer.dump();
                return rc;
            }
        }

        roundTimer.end();

        // Add round summary as a Lab to total
        totalTimer.addLab(
            roundTimer.final.timeInMs,
            QLatin1String(\"round \")
            % QString::number(i)
            % QLatin1String(\" / \")
            % QString::number(repetitions)
            % QLatin1String(\": \")
            % QString::number(roundTimer.final.timeInMs)
            % QLatin1String(\" ms\")
        );

        // If you want per-round details even on success, uncomment:
        // roundTimer.dump();
    }

    totalTimer.end();
    totalTimer.dump();
    qInfo() << \"avg Time per run: \" << totalTimer.final.timeInMs/ repetitions;

    return rc;
}
")
endfunction()


function(add_dkv2_test spec_row)
  _dkv2_parse_test_spec("${spec_row}" _target _class _main_header _files)

  set(_gen_main "${CMAKE_CURRENT_BINARY_DIR}/${_target}_main.cpp")
  _dkv2_write_single_main("${_gen_main}" "${_class}" "${_main_header}")

  add_executable(${_target}
    ${_gen_main}
    ${DKV2_TEST_COMMON_SOURCES}
    ${_main_header}
    ${_files} )

  target_precompile_headers(${_target} PRIVATE ${DKV2_TEST_PCH})

  set_target_properties(${_target} PROPERTIES AUTOGEN_USE_PCH ON)

  target_link_libraries(${_target} PRIVATE
    Qt6::Test dkv2_core)

  target_include_directories(${_target} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  if(NOT DKV2_CTEST_ONLY_TEST_ALL)
    add_test(NAME ${_target} COMMAND ${_target})
  endif()
endfunction()

function(add_runall_dkv2_test runall_target)
  set(_all_files "")
  set(_all_classes "")
  set(_all_main_headers "")

  foreach(_spec IN LISTS DKV2_TEST_SPECS)
    _dkv2_parse_test_spec("${_spec}" _target _class _main_header _files)
    list(APPEND _all_classes      "${_class}")
    list(APPEND _all_main_headers "${_main_header}")
    list(APPEND _all_files        "${_main_header}" ${_files})
  endforeach()

  list(REMOVE_DUPLICATES _all_files)
  list(REMOVE_DUPLICATES _all_main_headers)

  set(_gen_main "${CMAKE_CURRENT_BINARY_DIR}/${runall_target}_main.cpp")
  _dkv2_write_runall_main("${_gen_main}" "${_all_classes}" "${_all_main_headers}")

  add_executable(${runall_target}
    ${_gen_main}
    ${DKV2_TEST_COMMON_SOURCES}
    ${_all_files} )

  target_precompile_headers(${runall_target} PRIVATE ${DKV2_TEST_PCH})

  set_target_properties(${runall_target} PROPERTIES AUTOGEN_USE_PCH ON)

  target_link_libraries(${runall_target} PRIVATE
    Qt6::Test dkv2_core)

  target_include_directories(${runall_target} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

  add_test(NAME ${runall_target} COMMAND ${runall_target})
endfunction()


foreach(_spec IN LISTS DKV2_TEST_SPECS)
  add_dkv2_test("${_spec}")
endforeach()

add_runall_dkv2_test(test_all)

#
# in github actions / CI pipelines
# ctest --output-on-failure
#
#bzw
# cmake --build . --config Release --parallel
# ctest --output-on-failure -C Release
#
