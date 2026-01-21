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
#  "test_sqlhelper|test_sqlhelper|test_sqlhelper.h|test_sqlhelper.cpp"
  "test_statistics|test_statistics|test_statistics.h|test_statistics.cpp"
  "test_tabledatainserter|test_tableDataInserter|test_tabledatainserter.h|test_tabledatainserter.cpp"
  "test_views|test_views|test_views.h|test_views.cpp"
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

#include \"${main_header}\"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    ${class_name} tc;
    return QTest::qExec(&tc, argc, argv);
}
")
endfunction()

function(_dkv2_write_runall_main out_path class_names main_headers)
  set(_inc "")
  set(_exec "")

  foreach(_h IN LISTS main_headers)
    string(APPEND _inc "#include \"${_h}\"\n")
  endforeach()

  foreach(_c IN LISTS class_names)
    string(APPEND _exec "    { ${_c} tc; rc |= QTest::qExec(&tc, argc, argv); }\n")
  endforeach()

  file(WRITE "${out_path}" "
#include <QCoreApplication>
#include <QtTest>

${_inc}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    int rc = 0;
    dbgTimer time_all_tests(\"All tests runtime\");
${_exec}
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
