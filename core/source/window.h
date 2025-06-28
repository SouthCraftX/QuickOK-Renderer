#pragma once

#include <vulkan/vulkan_core.h>
#define __QOR_WINDOW_SRC__
#include "rendering_env.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

struct __Window;
typedef struct __Window _Window;

struct __Window
{
    int                     glfw_error;
    GLFWwindow *            window;
    GLFWmonitor *           monitor; // NULL if not full screen
    GLFWframebuffersizefun  framebuffer_resize_callback;
};

void
glfw_error_callback(
    int            error ,
    qo_ccstring_t  description
) {
    // TODO: Use vulkan debug messager
}

void
set_glfw_error_callback() {
}

qo_stat_t
window_init(
    _Window *               self ,
    const char *            title ,
    VkExtent2D              extent ,
    qo_bool_t               full_screen ,
    GLFWframebuffersizefun  framebuffer_resize_callback
) {
    // TODO: Move GLFW init to engine init
    int  ret = glfwInit();
    if (ret == GLFW_FALSE)
    {
        return QO_UNKNOWN_ERROR;
    }
    // Not on OpenGL
    glfwWindowHint(GLFW_CLIENT_API , GLFW_NO_API);
    self->monitor = full_screen ? glfwGetPrimaryMonitor() : NULL;
    self->window  = glfwCreateWindow(extent.width , extent.height , title ,
        self->monitor , NULL);
    glfwSetWindowUserPointer(self->window , self);
    glfwSetFramebufferSizeCallback(self->window , framebuffer_resize_callback);
    self->framebuffer_resize_callback = framebuffer_resize_callback;
}

void
window_destroy(
    _Window * self
) {
    // unneeded, because glfwTerminate() will destroy everything
    // glfwDestroyWindow(self->window);
    glfwTerminate();
    memset(self, 0 , sizeof(*self));
}

qo_bool_t
window_should_close(
    _Window * self
) {
    return glfwWindowShouldClose(self->window);
}

void
window_poll_events(
    _Window * self
) {
    glfwPollEvents();
}

VkResult
window_create_surface(
    _Window * self,
    VkInstance instance,
    VkSurfaceKHR * surface
) {
    return glfwCreateWindowSurface(instance , self->window , get_vk_allocator() , surface);
}

VkExtent2D
window_get_extent(
    _Window * self
) {
    VkExtent2D extent;
    glfwGetFramebufferSize(self->window , (int*)&extent.width , (int*)&extent.height);
    return extent;
}

GLFWwindow *
window_get_glfw_window(
    _Window * self
) {
    return self->window;
}

