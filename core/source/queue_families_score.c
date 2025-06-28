#include "rendering_env.h"

struct __FamilyScore
{
    qo_int8_t   index;
    qo_int16_t  score;
};
typedef struct __FamilyScore _FamilyScore;

struct __QueueFamilyScores
{
    qo_int32_t  compute;
    qo_int32_t  transfer;
    qo_int32_t  graphics;
};
typedef struct __QueueFamilyScores _QueueFamilyScores;

// 索引: (G << 2) | (C << 1) | (T << 0)
QO_GLOBAL_LOCAL
const _QueueFamilyScores  score_table[8] = {
    // Index 0: 000 (---) No capabilities
    {0 , 0 , 0} ,
    // Index 1: 001 (T--) Pure Transfer
    {0 , 200 , 0} ,
    // Index 2: 010 (-C-) Pure Compute
    {200 , 0 , 0} ,
    // Index 3: 011 (TC-) Transfer + Compute
    {150 , 75 , 0} ,
    // Index 4: 100 (--G) Pure Graphics
    {0 , 0 , 100} ,
    // Index 5: 101 (T-G) Transfer + Graphics
    {0 , 50 , 100} ,
    // Index 6: 110 (-CG) Compute + Graphics
    {50 , 0 , 100} ,
    // Index 7: 111 (TCG) All-in-one
    {25 , 25 , 100}
};

// 一个辅助函数，将新的分数插入到已排序的、固定大小的数组中
QO_GLOBAL_LOCAL
void
insert_into_sorted_array(
    _FamilyScore         sorted_array[4] ,
    const _FamilyScore * new_score
) {
    // 从后往前找插入位置
    for (qo_int32_t i = 3; i >= 0; --i)
    {
        // 如果新分数比当前位置的分数低，或者当前位置是空的(-1)
        if (new_score->score <= sorted_array[i].score ||
            sorted_array[i].index == -1)
        {
            // 如果不是最后一个位置，需要把前面的元素往后挪
            if (i < 3)
            {
                memmove(&sorted_array[i + 2] , &sorted_array[i + 1] ,
                    (3 - (i + 1)) * sizeof(_FamilyScore));
            }
            // 插入新元素
            sorted_array[i + 1] = *new_score;
            return;
        }
    }
    // 如果新分数比所有元素都高，则放在数组首位
    memmove(&sorted_array[1] , &sorted_array[0] , 3 * sizeof(_FamilyScore));
    sorted_array[0] = *new_score;
}

// 另一个更简单的插入逻辑，可读性更高
QO_GLOBAL_LOCAL
void
insert_score(
    _FamilyScore  top_scores[4] ,
    _FamilyScore  new_score
) {
    qo_int32_t  insert_pos = -1;
    // 找到第一个比新分数低的位置
    for (qo_int32_t i = 0; i < 4; ++i)
    {
        if (new_score.score > top_scores[i].score)
        {
            insert_pos = i;
            break;
        }
    }

    // 如果没有找到插入点 (新分数太低了)，直接返回
    if (insert_pos == -1)
    {
        return;
    }

    // 将插入点及之后的元素向后移动一位
    // 从后往前移动以避免覆盖
    for (qo_int32_t i = 3; i > insert_pos; --i)
    {
        top_scores[i] = top_scores[i - 1];
    }

    // 插入新分数
    top_scores[insert_pos] = new_score;
}

// Remark: It will discard the lowest score in the array if `family_count` is more than 4
_VkQueueFamilyCounts
score_queue_families(
    VkQueueFamilyProperties * families ,
    qo_uint32_t               family_count ,
    _VkQueueFamilyIndexes *   p_indexes
) {
    // 初始化四个元素的顶级分数数组，分数设为-1表示空位
    _FamilyScore  top_compute[4] = {
        {-1 , -1} , {-1 , -1} , {-1 , -1} , {-1 , -1}
    };
    _FamilyScore  top_transfer[4] = {
        {-1 , -1} , {-1 , -1} , {-1 , -1} , {-1 , -1}
    };
    _FamilyScore  top_graphics[4] = {
        {-1 , -1} , {-1 , -1} , {-1 , -1} , {-1 , -1}
    };

    qo_uint32_t   compute_count  = 0;
    qo_uint32_t   transfer_count = 0;
    qo_uint32_t   graphics_count = 0;

    // --- 遍历、评分并直接插入 ---
    for (qo_uint32_t i = 0; i < family_count; ++i)
    {
        VkQueueFlags  flags = families[i].queueFlags;
        qo_uint32_t   key = ((flags & VK_QUEUE_GRAPHICS_BIT) ? 4 : 0) |
                            ((flags & VK_QUEUE_COMPUTE_BIT) ? 2 : 0) |
                            ((flags & VK_QUEUE_TRANSFER_BIT) ? 1 : 0);
        qo_int32_t  bonus_score = families[i].queueCount;

        // 处理计算能力
        qo_int32_t  score_c = score_table[key].compute;
        if (score_c > 0)
        {
            compute_count++;
            _FamilyScore  new_score = {(qo_int8_t) i , score_c + bonus_score};
            insert_score(top_compute , new_score);
        }

        // 处理传输能力
        qo_int32_t  score_t = score_table[key].transfer;
        if (score_t > 0)
        {
            transfer_count++;
            _FamilyScore  new_score = {(qo_int8_t) i , score_t + bonus_score};
            insert_score(top_transfer , new_score);
        }

        // 处理图形能力
        qo_int32_t  score_g = score_table[key].graphics;
        if (score_g > 0)
        {
            graphics_count++;
            _FamilyScore  new_score = {(qo_int8_t) i , score_g + bonus_score};
            insert_score(top_graphics , new_score);
        }
    }

    // --- 填充输出的索引结构体 ---
    // 无需再次排序，top数组已经是排序好的
    for (qo_int32_t i = 0; i < 4; ++i)
    {
        p_indexes->compute_families[i]  = top_compute[i].index;
        p_indexes->transfer_families[i] = top_transfer[i].index;
        p_indexes->graphics_families[i] = top_graphics[i].index;
    }

    // --- 准备并返回计数的结构体 ---
    _VkQueueFamilyCounts  counts = {0};
    
    counts.graphics_family_count = (qo_uint8_t) (graphics_count >
        4 ? 4 : graphics_count);
    counts.compute_family_count  = (qo_uint8_t) (compute_count >
        4 ? 4 : compute_count);
    counts.transfer_family_count = (qo_uint8_t) (transfer_count >
        4 ? 4 : transfer_count);

    return counts;
}
