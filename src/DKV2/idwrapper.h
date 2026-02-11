#ifndef IDWRAPPER_H
#define IDWRAPPER_H

#pragma once
#include "helpersql.h"
#include <QtGlobal>

struct contractId_t { tableindex_t v;};
inline constexpr contractId_t Invalid_contract_id {SQLITE_invalidRowId};
inline constexpr contractId_t Minimal_contract_id {SQLITE_minimalRowId};
inline bool operator==(contractId_t a, contractId_t b) { return a.v == b.v; }
inline bool operator!=(contractId_t a, contractId_t b) { return a.v != b.v; }

struct bookingId_t  { tableindex_t v{-1}; };
inline bool operator==(bookingId_t a, bookingId_t b) { return a.v == b.v; }
inline bool operator!=(bookingId_t a, bookingId_t b) { return a.v != b.v; }

struct creditorId_t { tableindex_t v{-1}; };
inline constexpr creditorId_t Invalid_creditor_id {SQLITE_invalidRowId};
inline constexpr creditorId_t Minimal_creditor_id {SQLITE_minimalRowId};
inline bool operator==(creditorId_t a, creditorId_t b) { return a.v == b.v; }
inline bool operator!=(creditorId_t a, creditorId_t b) { return a.v != b.v; }

#endif // IDWRAPPER_H
