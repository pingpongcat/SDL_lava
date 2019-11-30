#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdalign.h>
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include "HandmadeMath.h"

typedef struct{
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
}VK_Rect;

typedef enum{
	VK_CTX_DEBUG = 1,
	VK_CTX_DOUBLEBUF = 2,
	VK_CTX_MIPMAPS = 4,
	VK_CTX_MULTISAMPLING = 8,
}VK_ContextMask;

typedef enum{
	VK_PIP_CULLING_CLOCKWISE = 1,
	VK_PIP_DOUBLEBUF = 2,
	VK_PIP_MIPMAPS = 4,
	VK_PIP_MULTISAMPLING = 8,
}VK_PipelineMask;

typedef struct{
	float position[3];
	//float normals[3];
	float texcoord[2];
}vertex_t;

typedef struct{
	vertex_t *vertices;
	uint32_t *indices;
	size_t vertices_size;
	size_t indices_size;
}mesh_t;

typedef struct ubo_t {
	alignas(16) hmm_mat4 model;
	alignas(16) hmm_mat4 view;
	alignas(16) hmm_mat4 proj;
}ubo_t;

typedef struct{
	VkDescriptorSetLayout descriptor_layout;
	VkPipelineLayout pipeline_layout;
}VK_PipelineLayout;

typedef struct{
	VkPipeline graphics_pipeline;
}VK_Pipeline;


typedef struct{
	int window_width, window_height;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physical_device;	
	VkPhysicalDeviceType physical_device_type;
	VkDevice device;
	uint32_t queue_family_index;
	VkColorSpaceKHR swapchain_color_space;
	VkFormat swapchain_image_format;
	uint32_t swapchain_images_count;
	VkSwapchainKHR swapchain;
	VkQueue device_queue;
	VkImage swapchain_images[2];
	VkImageView swapchain_images_views[2];
	VkFormat depth_image_format;
	VkSampleCountFlagBits sample_count;
	VkRenderPass render_pass;
	
	//VK_PipelineLayout *layout;
	VK_Pipeline *pip;
	//VkDescriptorSetLayout descriptor_layout;
	//VkPipelineLayout pipeline_layout;
	//VkPipeline graphics_pipeline;

	VkCommandPool command_pool;
	VkImage color_image;
	VkImage depth_image;
	VkImageView color_image_view;
	VkImageView depth_image_view;
	VkDeviceMemory color_image_allocation;
	VkDeviceMemory depth_image_allocation;
	VkFramebuffer swapchain_frame_buffers[2];
	bool gen_mips;
	float mips_max_level;
	//VK_Image
	uint32_t mips_level;

    VkImage texture_image;
    VkDeviceMemory texture_image_allocation;
    VkImageView texture_image_view;
    VkSampler texture_sampler;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_allocation;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_allocation;

	uint32_t vertices;

	VkBuffer uniform_buffer[2];
	VkDeviceMemory uniform_buffer_allocation[2];
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_sets[2];

	VkCommandBuffer command_buffers[2];

	VkSemaphore image_available_semaphore[2];
	VkSemaphore render_finished_semaphore[2];
	VkFence in_flight_fence[2];

}VK_Context;


VK_PipelineLayout* VK_CreatePipelineLayout(VK_Context *ctx, uint32_t bindings_count, VkDescriptorSetLayoutBinding bindings_description[], uint32_t push_constants_count, const VkPushConstantRange push_constants[]);

VkDescriptorSetLayoutBinding VK_CreateBindingDescriptor(uint32_t binding, uint32_t count, VkDescriptorType type, VkShaderStageFlags flag);

VK_Context* VK_CreateContext(SDL_Window* window, const char *window_title, uint32_t instance_layers_count, const char *instance_layers[], uint32_t device_extensions_count, const char *device_extensions[], VK_ContextMask context_mask);
void VK_DestroyContext(VK_Context *ctx, VK_PipelineLayout *layout);


void VK_Rest(VK_Context *ctx, VK_PipelineLayout *layout);

void VK_CreateImage(VK_Context *ctx, VkImage *image, VkDeviceMemory *data,  uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits sample_count, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
VkImageView VK_CreateImageView(VK_Context *ctx, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels, bool swizzle);
VkCommandBuffer VK_BeginSingleTimeCommands(VK_Context *ctx);
void VK_EndSingleTimeCommands(VK_Context *ctx, VkCommandBuffer *command_buffer);

void VK_RecreateSwapchain(VK_Context *ctx, SDL_Window *window, VK_PipelineLayout *layout);


