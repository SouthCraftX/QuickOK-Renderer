#pragma once
#include <vulkan/vulkan_core.h>
#define __QOR_STAGING_ALLOCATION_DEQUE_SRC__

#include "../rendering_env.h"

struct __StagingAllocation
{
    qo_uint64_t   fence_value;
    VkDeviceSize  offset;
    VkDeviceSize  size;
};
typedef struct __StagingAllocation _StagingAllocation;

#define STAGING_ALLOCATION_BLOCK_SIZE (64)

typedef struct __StagingAllocationBlock _StagingAllocationBlock;
struct __StagingAllocationBlock
{
    _StagingAllocationBlock *   prev;
    _StagingAllocationBlock *   next;
    _StagingAllocation          allocations[STAGING_ALLOCATION_BLOCK_SIZE];
};

struct __StagingAllocationDeque 
{
    _StagingAllocationBlock *   head_block;
    _StagingAllocationBlock *   tail_block;

    _StagingAllocation *        head;
    _StagingAllocation *        tail;
    size_t                      num_allocations;
};
typedef struct __StagingAllocationDeque _StagingAllocationDeque;

void
sadeque_init(
    _StagingAllocationDeque *   self
);

void
sadeque_destroy(
    _StagingAllocationDeque *   self
);

qo_bool_t
sadeque_push_back(
    _StagingAllocationDeque *   self,
    _StagingAllocation const *        allocation
);

qo_bool_t
sadeque_push_front(
    _StagingAllocationDeque *   self,
    _StagingAllocation const *        allocation
);

qo_bool_t
sadeque_pop_back(
    _StagingAllocationDeque *   self,
    _StagingAllocation *        allocation
);

qo_bool_t
sadeque_pop_front(
    _StagingAllocationDeque *   self,
    _StagingAllocation *        allocation
);

qo_bool_t
sadeque_peek_back(
    _StagingAllocationDeque *   self,
    _StagingAllocation *        allocation
);

qo_bool_t
sadeque_peek_front(
    _StagingAllocationDeque *   self,
    _StagingAllocation *        allocation
);

qo_bool_t
sadeque_is_empty(
    _StagingAllocationDeque *   self
);

size_t
sadeque_get_count(
    _StagingAllocationDeque *   self
);

