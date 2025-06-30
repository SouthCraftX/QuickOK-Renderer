#include "staging_allocation_deque.h"
#include <mimalloc.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

QO_GLOBAL_LOCAL
_StagingAllocationBlock *
block_new() {
    return mi_malloc_tp(_StagingAllocationBlock);
}

void
sadeque_init(
    _StagingAllocationDeque * self
) {
    self->head_block = NULL;
    self->tail_block = NULL;
    self->head = NULL;
    self->tail = NULL;
    self->num_allocations = 0;
}

void
sadeque_destroy(
    _StagingAllocationDeque * self
) {
    _StagingAllocationBlock * current = self->head_block;
    while (current)
    {
        _StagingAllocationBlock * next = current->next;
        mi_free(current);
        current = next;
    }
    mi_free(self);
}

qo_bool_t
sadeque_push_back(
    _StagingAllocationDeque *  self ,
    _StagingAllocation const * allocation
) {
    if (sadeque_is_empty(self))
    {
        _StagingAllocationBlock * new_block = block_new();
        if (!new_block)
        {
            return qo_false;
        }
        new_block->prev  = new_block->next = NULL;
        self->head_block = self->tail_block = new_block;
        self->head = &new_block->allocations[STAGING_ALLOCATION_BLOCK_SIZE / 2];
        self->tail = self->head;
    }
    else if (self->tail ==
             &self->tail_block->allocations[STAGING_ALLOCATION_BLOCK_SIZE])
    {
        if (self->tail_block->next)
        {
            self->tail_block = self->tail_block->next;
        }
        else {
            _StagingAllocationBlock * new_block = block_new();
            if (!new_block)
            {
                return qo_false;
            }
            new_block->prev = self->tail_block;
            new_block->next = NULL;
            self->tail_block->next = new_block;
            self->tail_block = new_block;
        }
        self->tail = &self->tail_block->allocations[0];
    }
    *self->tail++ = *allocation;
    self->tail++;
    self->num_allocations++;
    return qo_true;
}

qo_bool_t
sadeque_push_front(
    _StagingAllocationDeque *  self ,
    _StagingAllocation const * allocation
) {
    if (sadeque_is_empty(self))
    {
        _StagingAllocationBlock * new_block = block_new();
        if (!new_block)
        {
            return qo_false;
        }
        new_block->prev  = new_block->next = NULL;
        self->head_block = self->tail_block = new_block;
        self->head = self->tail =
            &new_block->allocations[STAGING_ALLOCATION_BLOCK_SIZE / 2];
        self->tail = self->head + 1;
        *(self->head) = *allocation;
        self->num_allocations++;
        return qo_true;
    }

    if (self->head == &self->head_block->allocations[0])
    {
        if (self->head_block->prev)
        {
            self->head_block = self->head_block->prev;
        }
        else {
            _StagingAllocationBlock * new_block = block_new();
            if (!new_block)
            {
                return qo_false;
            }
            new_block->prev = NULL;
            new_block->next = self->head_block;
            self->head_block->prev = new_block;
            self->head_block = new_block;
        }
        self->head =
            &self->head_block->allocations[STAGING_ALLOCATION_BLOCK_SIZE];
    }
    self->head--;
    *self->head = *allocation;
    self->num_allocations++;
    return qo_true;
}

qo_bool_t
sadeque_pop_front(
    _StagingAllocationDeque * self ,
    _StagingAllocation *      p_allocation
) {
    if (sadeque_is_empty(self))
    {
        return qo_false;
    }


    self->head++;
    self->num_allocations--;

    if (self->head ==
        &self->head_block->allocations[STAGING_ALLOCATION_BLOCK_SIZE])
    {
        if (self->num_allocations > 0)
        {
            self->head_block = self->head_block->next;
            self->head = &self->head_block->allocations[0];
        }
    }
    *p_allocation = *self->head;
    return qo_true;
}

qo_bool_t
sadeque_pop_back(
    _StagingAllocationDeque * self ,
    _StagingAllocation *      p_allocation
) {
    if (sadeque_is_empty(self))
    {
        return qo_false;
    }
    self->tail--;
    self->num_allocations--;
    if (self->tail == &self->tail_block->allocations[0])
    {
        if (self->num_allocations > 0)
        {
            self->tail_block = self->tail_block->prev;
            self->tail =
                &self->tail_block->allocations[STAGING_ALLOCATION_BLOCK_SIZE];
        }
    }
    *p_allocation = *self->tail;
    return qo_true;
}
