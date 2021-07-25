#ifndef DKV2VERSION_H
#define DKV2VERSION_H

#define STRINGIFY(s) STRINGIFY_HELPER(s)
#define STRINGIFY_HELPER(s) #s

#define DKV2_VERSION_MAJOR      0
#define DKV2_VERSION_MINOR      16
#define DKV2_VERSION_BUILD      0
#define DKV2_VERSION_LAST       1

// #define DKV2_VERSION_TEXT    "0, 16, 0, 1"
#define DKV2_VERSION_TEXT       STRINGIFY(DKV2_VERSION_MAJOR) "," STRINGIFY(DKV2_VERSION_MINOR) "," STRINGIFY(DKV2_VERSION_BUILD) "," STRINGIFY(DKV2_VERSION_LAST)

// #define DKV2_VERSION_STRING  "0.16.0.1"
#define DKV2_VERSION_STRING     STRINGIFY(DKV2_VERSION_MAJOR) "." STRINGIFY(DKV2_VERSION_MINOR) "." STRINGIFY(DKV2_VERSION_BUILD) "." STRINGIFY(DKV2_VERSION_LAST)

#define CURRENT_DKV2_VERSION    DKV2_VERSION_STRING

#define CURRENT_DB_VERSION      6

#define TARGET_COMPANY "HoMSoft"
#define TARGET_DESCRIPTION "DK Verwaltung f. MHS Projekte"
#define TARGET_PRODUCT "DKV2 Direktkredit Verwaltung"

#endif // DKV2VERSION_H
