#include "funnel_hash_table.h"
#include <vulkan/vulkan_core.h>

struct __VkFormatPropertiesMap
{
    _FunnelHashTable    hash_table;
};
typedef struct __VkFormatPropertiesMap _VkFormatPropertiesMap;
qo_stat_t
vk_fmtprops_map_init(
    _VkFormatPropertiesMap * self ,
    qo_size_t                capicity
);

void
vk_fmtprops_map_destroy(
    _VkFormatPropertiesMap * self
);

qo_bool_t
vk_fmtprops_map_insert(
    _VkFormatPropertiesMap *   self ,
    VkFormat                   format ,
    const VkFormatProperties * properties
);

VkFormatProperties const *
vk_fmtprops_map_search(
    _VkFormatPropertiesMap *    self ,
    VkFormat                    format
);

qo_size_t
vk_fmtprops_map_get_count(
    _VkFormatPropertiesMap *    self
);
