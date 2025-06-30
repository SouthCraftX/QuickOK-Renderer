#include "wvkimage.h"

struct _WVkTexture
{
    _WVkImage               base;
    VkSampler               sampler;
    VkImageView             default_view;
    _VkImageViewMap         custom_views_map;

    //VkImageViewCreateInfo   image_info;
    //XXH64_hash_t       image_info_hash;
};