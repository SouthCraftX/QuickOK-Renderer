#include "wvkimage.h"
#include <vulkan/vulkan_core.h>

struct __SamplerInfo
{
    VkSampler sampler;
    qo_uint64_t id;
};
typedef struct __SamplerInfo _WVkSamplerInfo;

struct __WVkTexture
{
    qo_ref_count_t          refernce_count;
    _WVkImage *             image;
    VkSampler               sampler;
    VkImageView             default_view;
    _VkImageViewMap         custom_views_map;
    qo_uint64_t             id;
    _VkDeviceContext *      device_context;
    //VkImageViewCreateInfo   image_info;
    //XXH64_hash_t       image_info_hash;
};
typedef struct __WVkTexture _WVkTexture;

QO_NODISCARD QO_NONNULL(1 , 2 , 3)
VkResult
wvktexture_new(
    _WVkTexture **  p_self ,
    _WVkImage *     image ,
    _VkDeviceContext *  device_context ,
    VkSampler       sampler
);

QO_GLOBAL_UNIQUE QO_NODISCARD QO_FORCE_INLINE QO_NONNULL(1)
_WVkImage *
wvktexture_get_image(
    _WVkTexture *  self
) {
    return self->image;
}

QO_GLOBAL_UNIQUE QO_NODISCARD QO_FORCE_INLINE QO_NONNULL(1)
VkSampler
wvktexture_get_sampler(
    _WVkTexture *  self
) {
    return self->sampler;
}

QO_GLOBAL_UNIQUE QO_NODISCARD QO_FORCE_INLINE QO_NONNULL(1)
VkResult
wvktexture_get_default_view(
    _WVkTexture *  self ,
    VkImageView *  p_view
);

QO_NONNULL(1 , 3)
VkResult
wvktexture_get_view(
    _WVkTexture *   self ,
    VkImageViewCreateInfo  partial_view_info ,
    VkImageView * p_view
);

QO_NONNULL(1 , 2)
VkResult
wvktexture_get_view2(
    _WVkTexture *   self ,
    VkImageSubresourceRange const * subresource_range ,
    VkImageViewType                 view_type ,
    VkImageView *   p_view
);

QO_NONNULL(1)
VkDescriptorImageInfo
wvktexture_get_default_descriptor_info(
    _WVkTexture *  self
);

QO_NONNULL(1 , 2)
VkDescriptorImageInfo
wvktexture_get_descriptor_info(
    _WVkTexture *  self ,
    VkImageView    view
);
