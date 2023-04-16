#pragma once

#include <cstddef>  // For size_t
#include <cstdint>  // For uintxx_t
#include <unordered_map>
#include <vector>

using namespace std;

// Global specification
using tx_id_t = uint64_t;     // Transaction id type
using t_id_t = uint32_t;      // Thread id type
using coro_id_t = int;        // Coroutine id type
using node_id_t = int;        // Machine id type
using mr_id_t = int;          // Memory region id type
using table_id_t = uint64_t;  // Table id type
using itemkey_t = uint64_t;   // Data item key type, used in DB tables
using offset_t =
    int64_t;  // Offset type. Usually used in remote offset for RDMA
using version_t = uint64_t;  // Version type, used in version checking
using lock_t = uint64_t;     // Lock type, used in remote locking

const int net_handler_thread_num = 2;
// Memory region ids for client's local_mr
const mr_id_t CLIENT_MR_ID = 100;

// Indicating that memory store metas have been transmitted
const uint64_t MEM_STORE_META_END = 0xE0FF0E0F;

// Alias
#define Aligned8 __attribute__((aligned(8)))
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define TID (std::this_thread::get_id())

// Helpful for improving condition prediction hit rate
#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

struct SystemConfig {
  int executor_num;
  vector<string> server_addrs;
  string db_type;
};
