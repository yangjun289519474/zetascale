//----------------------------------------------------------------------------
// ZetaScale
// Copyright (c) 2016, SanDisk Corp. and/or all its affiliates.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published by the Free
// Software Foundation;
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License v2.1 for more details.
//
// A copy of the GNU Lesser General Public License v2.1 is provided with this package and
// can also be found at: http://opensource.org/licenses/LGPL-2.1
// You should have received a copy of the GNU Lesser General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA.
//----------------------------------------------------------------------------

#ifndef REPLICATION_TEST_COMMON_H
#define REPLICATION_TEST_COMMON_H 1
/*
 * File:   test_common.h
 * Author: Zhenwei Lu
 *
 * Note:test case for replication test_framework
 * Created on Dec 8, 2008, 12:04 AM
 *
 * Copyright Schooner Information Technology, Inc.
 * http://www.schoonerinfotech.com/
 *
 * $Id: test_common.h 10106 2009-06-18 08:00:17Z lzwei $
 */
#include "protocol/replication/copy_replicator_internal.h"

#include "sys/queue.h"
#include "tlmap3.h"
#include "test_config.h"
#include "test_framework.h"
#include "test_model.h"

#define LOG_ID          PLAT_LOG_ID_INITIAL
#define LOG_DBG         PLAT_LOG_LEVEL_DEBUG
#define LOG_DIAG        PLAT_LOG_LEVEL_DIAGNOSTIC
#define LOG_INFO        PLAT_LOG_LEVEL_INFO
#define LOG_ERR         PLAT_LOG_LEVEL_ERROR
#define LOG_WARN        PLAT_LOG_LEVEL_WARN
#define LOG_TRACE       PLAT_LOG_LEVEL_TRACE
#define LOG_FATAL       PLAT_LOG_LEVEL_FATAL

#define MAX_NODE 4
#define TIMEOUT_FACTOR 2

#define PLAT_OPTS_ITEMS_replication_test_framework_sm() \
    PLAT_OPTS_SHMEM(shmem)

struct plat_opts_config_replication_test_framework_sm {
    struct plat_shmem_config shmem;
};

/*
 * XXX: drew 2009-05-29 Most of the tests cut-and-paste the common code
 * so we don't duplicate it by default.  There's also an added level
 * of indirection here; everything should probably just fold into
 * the replication_test_config structure.
 */
#ifdef RT_USE_COMMON
#define PLAT_OPTS_COMMON_TEST(config_field) \
    PLAT_OPTS_SHMEM(config_field.shmem_config)                                 \
    PLAT_OPTS_FTH()                                                            \
    item("iterations", "how many operations", ITERATIONS,                      \
         parse_int(&config->config_field.test_config.iterations, optarg, NULL),\
         PLAT_OPTS_ARG_REQUIRED)                                               \
    item("replication_type", "replication type", REPLICATION_TYPE,             \
         ({                                                                    \
          config->config_field.test_config.replication_type =                  \
          str_to_sdf_replication(optarg);                                      \
          if (config->config_field.test_config.replication_type ==             \
              SDF_REPLICATION_INVALID) {                                       \
          sdf_replication_usage();                                             \
          }                                                                    \
          config->config_field.test_config.replication_type ==                 \
          SDF_REPLICATION_INVALID ? -EINVAL : 0;                               \
          }), PLAT_OPTS_ARG_REQUIRED)                                          \
    item("num_replicas", "number of replicas", NUM_REPLICAS,                   \
         parse_int(&config->config_field.test_config.iterations, optarg, NULL),\
         PLAT_OPTS_ARG_REQUIRED)                                               \
    item("replication_lease_liveness",                                         \
         "use liveness instead of leases for test",                            \
         REPLICATION_LEASE_LIVENESS,                                           \
         ({ config->config_field.test_config.replicator_config.lease_liveness = 1; 0; \
          }), PLAT_OPTS_ARG_NO)                                                \
    item("replication_initial_preference",                                     \
         "land VIPs on preferred nodes instead of first",                      \
         REPLICATION_INITIAL_PREFERENCE,                                       \
         ({ config->config_field.test_config.replicator_config.initial_preference = 1; 0; \
          }), PLAT_OPTS_ARG_NO)                                                \
    item("new_liveness", "use new liveness", NEW_LIVENESS,                     \
         ({ config->config_field.test_config.new_liveness = 1; 0; }),          \
         PLAT_OPTS_ARG_NO)                                                     \
    item("msg_live", "messaging: node liveness time", MSG_LIVE,                \
         parse_int(&config->config_field.test_config.msg_live_secs, optarg,    \
                   NULL), PLAT_OPTS_ARG_REQUIRED)                              \
    item("msg_ping", "messaging: ping interval for liveness", MSG_PING,        \
         parse_int(&config->config_field.test_config.msg_ping_secs, optarg,    \
                   NULL), PLAT_OPTS_ARG_REQUIRED)                              \
    item("ffdc_buffer_len", "ffdc: per thread buffer size in bytes",           \
         FFDC_BUFFER_LEN, parse_size(&config->config_field.ffdc_buffer_len,    \
                                     optarg, NULL), PLAT_OPTS_ARG_REQUIRED)    \
    item("ffdc_disable", "disable FFDC logging", FFDC_DISABLE,                 \
         ({ config->config_field.ffdc_disable = 1; 0; }), PLAT_OPTS_ARG_NO)

struct rt_common_test_config {
    struct plat_shmem_config shmem_config;
    struct replication_test_config test_config;

    /** @brief FFDC buffer len in bytes */
    int64_t ffdc_buffer_len;

    /** @brief disable FFDC logging */
    unsigned ffdc_disable : 1;
};

__BEGIN_DECLS

/** @brief Initialize default */
void rt_common_test_config_init(struct rt_common_test_config *config);

/** @brief Free deep structures */
void rt_common_test_config_destroy(struct rt_common_test_config *config);

/**
 * @brief Initialize common subsystems
 *
 * Includes shmem, ffdc, fth.  Use #rt_common_detach to detach
 *
 * @return 0 on success, -errno on failure
 */
int rt_common_init(struct rt_common_test_config *config);

/**
 * @brief Detach common subsystems
 *
 * Undoes #rt_common_init.
 *
 * @return 0 on success, -errno on failure
 */
void rt_common_detach(struct rt_common_test_config *config);

__END_DECLS

#endif /* def RT_USE_COMMON */


__BEGIN_DECLS

/**
 * @brief Deprecated - use rt_common_detach and rt_common_test_config_destroy
 *
 * This requires a plat_opts_config_replication_framework_sm structure
 * which is silly.
 */
int
framework_sm_destroy(struct plat_opts_config_replication_test_framework_sm *config);

/**
 * @brief Deprecated - use rt_common_init
 *
 * This does its own command line parsing and spews for options other than
 * in the standard set.
 */
SDF_status_t
framework_sm_init(int argc, char **argv, struct plat_opts_config_replication_test_framework_sm *config);

struct sdf_replicator *
alloc_replicator(const struct sdf_replicator_config *replicator_config,
                 struct sdf_replicator_api *api, void *extra);

int64_t
rt_set_async_timeout(struct replication_test_config config);

/**
 * @brief Initialize shared memory
 *
 * Deprecated - use rt_common_init
 *
 * @return 0 on success, -errno on failure
 */
int rt_sm_init(struct plat_shmem_config *shmem);

/**
 * @brief Detach shared memory
 *
 * Deprecated - use rt_common_init and rt_common_detach
 *
 * @return 0 on success, -errno on failure
 */
int rt_sm_detach();

/**
 * @brief Initialize a sdf_replication_shard_meta
 *
 * @return 0 on success, else 1
 */
int r_shard_meta_init(struct SDF_shard_meta *shard_meta, vnode_t cur_home_node,
                      struct sdf_replication_shard_meta **out, uint32_t nnode);

/**
 * @brief Initialize SDF_shard meta and cr_shard_meta
 */
void
init_meta_data(struct replication_test_framework *test_framework,
               SDF_replication_props_t *replication_props,
               struct sdf_replication_shard_meta **r_shard_meta,
               vnode_t node, SDF_shardid_t shard_id, uint32_t lease_usecs,
               struct SDF_shard_meta **out_shard_meta,
               struct cr_shard_meta **out_cr_meta);

__END_DECLS
#endif /* ndef REPLICATION_TEST_COMMON_H */
