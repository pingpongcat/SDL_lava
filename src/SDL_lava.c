//#include <lavalamp.h>
#include <stdbool.h>

#include <SDL2/SDL_vulkan.h>

#include "SDL_lava.h"
#include "SDL_lava_utils.h"
#include "SDL_lava_memory.h"
#include "SDL_lava_import.h"

#define NUM(a) (sizeof(a)/sizeof(a[0]))


void VK_CreateSurface(VK_Renderer *renderer, SDL_Window* window);
void VK_CreateInstance(SDL_Window *window, VK_Renderer *renderer, const char *window_title, uint32_t instance_layers_count, const char* instance_layers[], VK_RendererMask context_mask);
void VK_GetPhisicalDevice(VK_Renderer *renderer);
void VK_CreateDevice(VK_Renderer *renderer, uint32_t instance_layers_count, const char *instance_layers[], uint32_t  device_extensions_count, const char *device_extensions[], VK_RendererMask context_mask);
void VK_GetSurfaceFormat(VK_Renderer *renderer);
void VK_CreateSwapchain(VK_Renderer *renderer, SDL_Window *window);
void VK_CreateSwapchainImageViews(VK_Renderer *renderer);
void VK_GetSampleCount(VK_Renderer *renderer);
void VK_GetDepthFormat(VK_Renderer *renderer);
void VK_CreateRenderPass(VK_Renderer *renderer);
//void VK_CreateDescriptionSetLayout(VK_Renderer *renderer);
//void VK_CreatePipelineLayout(VK_Renderer *renderer);

VkDescriptorSetLayoutBinding VK_CreateBindingDescriptor(uint32_t binding, uint32_t count, VkDescriptorType type, VkShaderStageFlags flag);

VkDescriptorSetLayout VK_CreateDescriptionSetLayout(VK_Renderer *renderer, uint32_t count, VkDescriptorSetLayoutBinding bindings_description[]);

VkPipelineLayout VK_CreatePipelineLayout(VK_Renderer *renderer, VkDescriptorSetLayout *descriptor_layout);

void VK_CreateGraphicsPipeline(VK_Renderer *renderer);
void VK_CreateCommandPool(VK_Renderer *renderer);
void VK_CreateColorResource(VK_Renderer *renderer);
void VK_CreateDepthResource(VK_Renderer *renderer);
void VK_CreateFramebuffers(VK_Renderer *renderer);
void VK_CreateDescriptionPool(VK_Renderer *renderer);
void VK_CreateDescriptorSets(VK_Renderer *renderer);
void VK_CreateCommandBuffers(VK_Renderer *renderer);
void VK_CreateSyncObjects(VK_Renderer *renderer);

VK_Renderer* VK_CreateRenderer(SDL_Window *window, const char *window_title, uint32_t instance_layers_count, const char *instance_layers[], uint32_t device_extensions_count, const char *device_extensions[], VK_RendererMask context_mask){

	VK_Renderer *renderer = malloc(sizeof(VK_Renderer));

	SDL_Vulkan_GetDrawableSize(window, &renderer->window_width, &renderer->window_height);

	renderer->instance = VK_NULL_HANDLE;
	renderer->physical_device = VK_NULL_HANDLE;
	renderer->surface = VK_NULL_HANDLE;
	renderer->device = VK_NULL_HANDLE;
	renderer->swapchain_images_count = 2;
	renderer->swapchain = VK_NULL_HANDLE;
	renderer->swapchain_image_format = VK_NULL_HANDLE;
	renderer->swapchain_color_space = VK_NULL_HANDLE;
	renderer->depth_image_format = VK_NULL_HANDLE;
	renderer->render_pass = VK_NULL_HANDLE;

	renderer->descriptor_layout = VK_NULL_HANDLE;
	renderer->pipeline_layout = VK_NULL_HANDLE;

	renderer->graphics_pipeline = VK_NULL_HANDLE;
	renderer->command_pool = VK_NULL_HANDLE;
	renderer->mips_max_level = 0.0;
	renderer->device_queue = VK_NULL_HANDLE;
	renderer->gen_mips = true;
	renderer->descriptor_pool = VK_NULL_HANDLE;

	//VK_CreateRenderer
	VK_CreateInstance(window, renderer, window_title, instance_layers_count, instance_layers, context_mask);
	VK_CreateSurface(renderer, window);
	VK_GetPhisicalDevice(renderer);
	VK_CreateDevice(renderer, instance_layers_count, instance_layers, device_extensions_count, device_extensions, context_mask);
	VK_GetSurfaceFormat(renderer);
	VK_CreateSwapchain(renderer, window);
	VK_CreateSwapchainImageViews(renderer);
	VK_GetDepthFormat(renderer);
	VK_GetSampleCount(renderer);
	VK_CreateRenderPass(renderer);

	VkDescriptorSetLayoutBinding description_set_bindigns[] = { // = {ubo_bindig, sampler_bindig};
		VK_CreateBindingDescriptor(0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		VK_CreateBindingDescriptor(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	renderer->descriptor_layout =  VK_CreateDescriptionSetLayout(renderer, NUM(description_set_bindigns), description_set_bindigns);
	renderer->pipeline_layout = VK_CreatePipelineLayout(renderer, &renderer->descriptor_layout);
	/*
	const char *shader_sources[] = { "../assets/shaders/vert.spv",  "../assets/shaders/frag.spv"};

	VkVertexInputAttributeDescription attribute_description[] = {// = {position_attribute_description, textcoord_attribute_description};
		VK_CreateShaderDescriptor(0,0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		VK_CreateShaderDescriptor(0,1, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float))
	};

	renderer->graphics_pipeline = VK_CreateGraphicsPipeline(renderer,
			&renderer->pipeline_layout,
			NUM(shader_sources),
			shader_sources,
			NUM(attribute_description),
			attribute_description,
			0);
	*/
	VK_CreateGraphicsPipeline(renderer);

	VK_CreateCommandPool(renderer);
	VK_CreateColorResource(renderer);
	VK_CreateDepthResource(renderer);
	VK_CreateFramebuffers(renderer);

	VK_CreateTextureImage(renderer, "../assets/images/chalet.jpg");
	//VK_CreateTextureImage(renderer, "../assets/models/DuckCM.png");
	VK_CreateTextureImageView(renderer);
	VK_CreateTextureSampler(renderer);
	VK_LoadModel(renderer, "../assets/models/chalet.obj");
	//VK_LoadModel(renderer, "../assets/models/Duck.obj`");
	VK_CreateUniformBuffer(renderer);

	VK_CreateDescriptionPool(renderer);
	VK_CreateDescriptorSets(renderer);
	VK_CreateCommandBuffers(renderer);
	VK_CreateSyncObjects(renderer);

	return renderer;
}

void VK_CreateInstance(SDL_Window *window, VK_Renderer *renderer, const char *window_title, uint32_t instance_layers_count, const char* instance_layers[], VK_RendererMask context_mask){

	uint32_t instance_extensions_count;
	SDL_Vulkan_GetInstanceExtensions(window, &instance_extensions_count, NULL);
	const char* instance_extensions[instance_extensions_count];
	SDL_Vulkan_GetInstanceExtensions(window, &instance_extensions_count, instance_extensions);

	VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO};
	app_info.pNext = NULL;
	app_info.pApplicationName = window_title;
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.pEngineName = "No name";
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 21);

	VkInstanceCreateInfo instance_create_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instance_create_info.pNext = NULL;
	instance_create_info.flags = 0;
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledLayerCount = context_mask & VK_RENDERER_DEBUG ? instance_layers_count : 0;
	instance_create_info.ppEnabledLayerNames = context_mask & VK_RENDERER_DEBUG ? instance_layers : VK_NULL_HANDLE;
	instance_create_info.enabledExtensionCount = instance_extensions_count;
	instance_create_info.ppEnabledExtensionNames = instance_extensions,

		assert(vkCreateInstance(&instance_create_info, NULL, &renderer->instance) == VK_SUCCESS);
}

void VK_CreateSurface(VK_Renderer *renderer, SDL_Window* window){
	assert(SDL_Vulkan_CreateSurface(window, renderer->instance, &renderer->surface) == 1);
}

void VK_GetPhisicalDevice(VK_Renderer *renderer){

	uint32_t devices_count = 0;
	vkEnumeratePhysicalDevices(renderer->instance, &devices_count, NULL);
	VkPhysicalDevice devices[devices_count];
	vkEnumeratePhysicalDevices(renderer->instance, &devices_count, devices);

	for (uint32_t i = 0; i < devices_count; ++i)
	{
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, NULL);
		VkQueueFamilyProperties queue_family_properties[queue_family_count];
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, queue_family_properties);

		for (uint32_t j = 0; j < queue_family_count; ++j) {

			VkBool32 supports_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, renderer->surface, &supports_present);

			if (supports_present && (queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				renderer->queue_family_index = j;
				renderer->physical_device = devices[i];
				break;
			}
		}
		if (renderer->physical_device)
		{
			break;
		}
	}
}

void VK_CreateDevice(VK_Renderer *renderer, uint32_t instance_layers_count, const char *instance_layers[], uint32_t  device_extensions_count, const char *device_extensions[], VK_RendererMask context_mask){

	VkDeviceQueueCreateInfo device_queue_create_info = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
	device_queue_create_info.pNext = NULL;
	device_queue_create_info.flags = 0;
	device_queue_create_info.queueFamilyIndex = renderer->queue_family_index;
	device_queue_create_info.queueCount	= 1;
	device_queue_create_info.pQueuePriorities = (const float []){1.0f};

	VkPhysicalDeviceFeatures supported_features;
	vkGetPhysicalDeviceFeatures(renderer->physical_device, &supported_features);
	//Anisotropy sampler support
	VkBool32 anisotropy_enable = supported_features.samplerAnisotropy;
	assert(anisotropy_enable);

	VkPhysicalDeviceFeatures device_features = {.samplerAnisotropy = VK_TRUE};

	VkDeviceCreateInfo device_create_info = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	device_create_info.pNext = NULL;
	device_create_info.flags = 0;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &device_queue_create_info;
	device_create_info.enabledLayerCount = context_mask & VK_RENDERER_DEBUG ? instance_layers_count : 0;
	device_create_info.ppEnabledLayerNames =  context_mask & VK_RENDERER_DEBUG ? instance_layers : VK_NULL_HANDLE;
	device_create_info.enabledExtensionCount = device_extensions_count > 0 ? device_extensions_count : 0;
	device_create_info.ppEnabledExtensionNames =device_extensions_count > 0 ? device_extensions : VK_NULL_HANDLE;
	device_create_info.pEnabledFeatures = &device_features;

	assert(vkCreateDevice(renderer->physical_device, &device_create_info, NULL, &renderer->device) == VK_SUCCESS);

	vkGetDeviceQueue(renderer->device,renderer->queue_family_index, 0, &renderer->device_queue);
}

void VK_GetDepthFormat(VK_Renderer *renderer){

	VkFormat depth_formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	for (unsigned long i = 0; i < NUM(depth_formats); i++ )
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(renderer->physical_device, depth_formats[i], &props);

		if ((tiling == VK_IMAGE_TILING_LINEAR) &&
				((props.linearTilingFeatures & features) == features))
		{
			renderer->depth_image_format = depth_formats[i];
			break;
		}
		else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
				((props.optimalTilingFeatures & features) == features))
		{
			renderer->depth_image_format = depth_formats[i];
			break;
		}
	}

}

void VK_GetSurfaceFormat(VK_Renderer *renderer){

	uint32_t surface_formats_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physical_device, renderer->surface, &surface_formats_count, NULL);
	assert(surface_formats_count);

	VkSurfaceFormatKHR surface_formats[surface_formats_count];
	vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physical_device, renderer->surface, &surface_formats_count, surface_formats);

	if ((surface_formats_count == 1) && (surface_formats[0].format == VK_FORMAT_UNDEFINED)){
		renderer->swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
		renderer->swapchain_color_space = surface_formats[0].colorSpace;
	}
	else {
		bool found_B8G8R8A8_UNORM = false;

		for (uint32_t i = 0; i < surface_formats_count; i++){
			if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM){
				renderer->swapchain_image_format = surface_formats[i].format;
				renderer->swapchain_color_space = surface_formats[i].colorSpace;
				found_B8G8R8A8_UNORM = true;
				break;
			}
		}
		if (!found_B8G8R8A8_UNORM){
			renderer->swapchain_image_format = surface_formats[0].format;
			renderer->swapchain_color_space = surface_formats[0].colorSpace;
		}

	}
}

VkImageView VK_CreateImageView(VK_Renderer *renderer, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels, bool swizzle){

	VkImageViewCreateInfo image_view_create_info = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	image_view_create_info.image = image;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = format;
	image_view_create_info.subresourceRange .aspectMask = aspect_flags;
	image_view_create_info.subresourceRange .baseMipLevel = 0;
	image_view_create_info.subresourceRange .levelCount = mip_levels;
	image_view_create_info.subresourceRange .baseArrayLayer = 0;
	image_view_create_info.subresourceRange .layerCount = 1;

	if(swizzle){

		image_view_create_info.components .r = VK_COMPONENT_SWIZZLE_B;
		image_view_create_info.components .g = VK_COMPONENT_SWIZZLE_G;
		image_view_create_info.components .b = VK_COMPONENT_SWIZZLE_R;
		image_view_create_info.components .a = VK_COMPONENT_SWIZZLE_IDENTITY;
	}

	VkImageView image_view;
	assert(vkCreateImageView(renderer->device, &image_view_create_info, NULL, &image_view) == VK_SUCCESS);

	return  image_view;
}

VkExtent2D VK_GetSwapchainExtent(VK_Renderer *renderer, SDL_Window *window, VkSurfaceCapabilitiesKHR *surface_capabilities){

	if (surface_capabilities->currentExtent.width != UINT32_MAX) {
		return surface_capabilities->currentExtent;
	} else {
		SDL_Vulkan_GetDrawableSize(window, &renderer->window_width, &renderer->window_height);

		VkExtent2D extent = {0};
		extent.width = clamp(renderer->window_width, surface_capabilities->minImageExtent.width, surface_capabilities->maxImageExtent.width);
		extent.height = clamp(renderer->window_height, surface_capabilities->minImageExtent.height, surface_capabilities->maxImageExtent.height); 

		return extent;
	}
}

void VK_CreateSwapchain(VK_Renderer *renderer, SDL_Window *window){

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physical_device, renderer->surface, &surface_capabilities);

	renderer->window_width = clamp(renderer->window_width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
	renderer->window_height = clamp(renderer->window_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height); 

	VkSwapchainCreateInfoKHR swapchain_create_info = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	swapchain_create_info.pNext = NULL;
	swapchain_create_info .flags = 0;
	swapchain_create_info .surface = renderer->surface;
	//In case of wayland it's 4
	//swapchain_create_info.minImageCount = renderer->surface_capabilities.minImageCount,
	swapchain_create_info.minImageCount = renderer->swapchain_images_count;
	swapchain_create_info.imageFormat = renderer->swapchain_image_format;
	swapchain_create_info.imageColorSpace = renderer->swapchain_color_space;
	swapchain_create_info.imageExtent.width = renderer->window_width;
	swapchain_create_info.imageExtent.height = renderer->window_height;

	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount = renderer->queue_family_index;
	swapchain_create_info.pQueueFamilyIndices = NULL;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = NULL;

	assert(vkCreateSwapchainKHR(renderer->device, &swapchain_create_info, NULL, &renderer->swapchain) == VK_SUCCESS);

	vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->swapchain_images_count, NULL);
	assert(renderer->swapchain_images_count);
	vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->swapchain_images_count, renderer->swapchain_images);
}

void VK_CreateSwapchainImageViews(VK_Renderer *renderer){

	for (uint32_t i = 0; i < renderer->swapchain_images_count; ++i) {

		renderer->swapchain_images_views[i] = VK_CreateImageView(renderer,
				renderer->swapchain_images[i],
				renderer->swapchain_image_format,
				VK_IMAGE_ASPECT_COLOR_BIT,
				1,
				false);
	}

}

void VK_GetSampleCount(VK_Renderer *renderer){

	renderer->sample_count = VK_SAMPLE_COUNT_1_BIT;

	VkPhysicalDeviceProperties physical_device_properties;
	vkGetPhysicalDeviceProperties(renderer->physical_device, &physical_device_properties);

	VkSampleCountFlags color_sample_counts = physical_device_properties.limits.framebufferColorSampleCounts; 
	VkSampleCountFlags depth_sample_counts = physical_device_properties.limits.framebufferDepthSampleCounts; 
	VkSampleCountFlags counts = color_sample_counts < depth_sample_counts ? color_sample_counts : depth_sample_counts; 

	if (counts & VK_SAMPLE_COUNT_64_BIT) { renderer->sample_count = VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { renderer->sample_count = VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { renderer->sample_count = VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { renderer->sample_count = VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { renderer->sample_count = VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { renderer->sample_count = VK_SAMPLE_COUNT_2_BIT; }
}

void VK_CreateRenderPass(VK_Renderer *renderer){

	VkAttachmentDescription swachain_images = {0};
	swachain_images.format = renderer->swapchain_image_format;
	swachain_images.samples = renderer->sample_count;
	//swachain_images.samples = VK_SAMPLE_COUNT_1_BIT,
	swachain_images.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	swachain_images.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	swachain_images.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	swachain_images.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	swachain_images.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swachain_images.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_resource = {0}; 
	depth_resource.format = renderer->depth_image_format;
	depth_resource.samples = renderer->sample_count;
	//depth_resource.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_resource.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_resource.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_resource.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_resource.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_resource.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_resource.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription color_resource = {0};
	color_resource.format = renderer->swapchain_image_format;
	color_resource.samples = VK_SAMPLE_COUNT_1_BIT;
	color_resource.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_resource.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_resource.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_resource.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_resource.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_resource.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 

	VkAttachmentDescription render_pass_attachments[] = {swachain_images, depth_resource, color_resource};

	VkAttachmentReference swachain_images_reference = {0};
	swachain_images_reference.attachment = 0;
	swachain_images_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_resource_reference = {0};
	depth_resource_reference.attachment = 1;
	depth_resource_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_resource_reference = {0};
	color_resource_reference.attachment = 2;
	color_resource_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &swachain_images_reference;
	subpass.pDepthStencilAttachment = &depth_resource_reference;
	subpass.pResolveAttachments = &color_resource_reference;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	render_pass_info .attachmentCount = NUM(render_pass_attachments);
	render_pass_info .pAttachments = render_pass_attachments;
	render_pass_info .subpassCount = 1;
	render_pass_info .pSubpasses = &subpass;
	render_pass_info .dependencyCount = 1;
	render_pass_info .pDependencies = &dependency;

	assert(vkCreateRenderPass(renderer->device, &render_pass_info, NULL, &renderer->render_pass) == VK_SUCCESS);
}

VkDescriptorSetLayoutBinding VK_CreateBindingDescriptor(uint32_t binding, uint32_t count, VkDescriptorType type, VkShaderStageFlags flag){

	VkDescriptorSetLayoutBinding bindings_description;
	bindings_description.binding = binding;
	bindings_description.descriptorCount = count;
	bindings_description.descriptorType = type;
	bindings_description.pImmutableSamplers = NULL;
	bindings_description.stageFlags = flag;

	return bindings_description;
}

VkDescriptorSetLayout VK_CreateDescriptionSetLayout(VK_Renderer *renderer, uint32_t count, VkDescriptorSetLayoutBinding bindings_description[]){
	VkDescriptorSetLayout bindings;

	VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	descriptor_layout_info.bindingCount = count;
	descriptor_layout_info.pBindings = bindings_description;

	assert(vkCreateDescriptorSetLayout(renderer->device, &descriptor_layout_info, NULL, &bindings) == VK_SUCCESS);

	return bindings;
}

VkPipelineLayout VK_CreatePipelineLayout(VK_Renderer *renderer, VkDescriptorSetLayout *descriptor_layout){

	VkPipelineLayout layout;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	pipeline_layout_info.setLayoutCount = 1; // Optional
	pipeline_layout_info.pSetLayouts = descriptor_layout; // Optional
	pipeline_layout_info.pushConstantRangeCount = 0; // Optional
	pipeline_layout_info.pPushConstantRanges = NULL; // Optional

	assert(vkCreatePipelineLayout(renderer->device, &pipeline_layout_info, NULL, &layout) == VK_SUCCESS);

	return layout;
}

VkShaderModule VK_CreateShaderModule(VkDevice device, char *filename){
	FILE *file = fopen(filename,"r");

	if (!file) {
		printf("Can't open file: %s\n", filename);
		return 0;
	}

	size_t lenght = file_get_lenght(file);

	const uint32_t *shader_descripteion = (const uint32_t *)calloc(lenght + 1, 1);
	if (!shader_descripteion) {
		printf("Out of memory when reading file: %s\n",filename);
		fclose(file);
		file = NULL;
		return 0;;
	}

	fread((void *)shader_descripteion,1,lenght,file);
	fclose(file);

	file = NULL;

	VkShaderModuleCreateInfo shader_module_create_info = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	shader_module_create_info.codeSize = lenght;
	shader_module_create_info.pCode	= shader_descripteion;

	VkShaderModule shader_module;
	assert(vkCreateShaderModule(device, &shader_module_create_info, NULL, &shader_module) == VK_SUCCESS); 
	free((void *)shader_descripteion);
	shader_descripteion = NULL;

	return shader_module;
}

VkVertexInputAttributeDescription VK_CreateShaderDescriptor(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset){

	VkVertexInputAttributeDescription attribute = {0};
	attribute.binding = binding;
	attribute.location = location;
	attribute.format = format;
	attribute.offset = offset;

	return attribute;
}

VkVertexInputBindingDescription VK_CreateVertexInputDescriptor(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate){

	VkVertexInputBindingDescription vertex_input = {0};
	vertex_input.binding = binding;
	vertex_input.stride = stride;
	vertex_input.inputRate = input_rate;

	return vertex_input;
}


VkPipelineShaderStageCreateInfo VK_CreateShaderStage(VK_Renderer *renderer, const char *path){

	if (strstr(path, ".vert") || strstr(path, ".vs") || strstr(path, ".vertex")){
	
		VkShaderModule vert_shader_module = VK_CreateShaderModule(renderer->device, "../assets/shaders/vert.spv");

		VkPipelineShaderStageCreateInfo vert_shader_stage_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName = "main";

		return vert_shader_stage_info;
	};

	if (strstr(path, ".frag") || strstr(path, ".fs") || strstr(path, ".fragment")){

		VkShaderModule frag_shader_module = VK_CreateShaderModule(renderer->device, "../assets/shaders/frag.spv");

		VkPipelineShaderStageCreateInfo frag_shader_stage_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName = "main";

		 return frag_shader_stage_info;
	};

	printf("Error: could not detect shader type of the file %s, please use:\n .vert, .vertex, .vs file extensions for the vertex shader files\n	.frag, .fragment, .fs file extensions for the fragment shader files\n",	path);

	exit (0);

}

void VK_CreateGraphicsPipeline(VK_Renderer *renderer){


	VkShaderModule vert_shader_module = VK_CreateShaderModule(renderer->device, "../assets/shaders/vert.spv");

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkShaderModule frag_shader_module = VK_CreateShaderModule(renderer->device, "../assets/shaders/frag.spv");

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

	VkVertexInputBindingDescription bindings_description = VK_CreateVertexInputDescriptor(0, 5 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX);
	
	VkVertexInputAttributeDescription attribute_description[] = {// = {position_attribute_description, textcoord_attribute_description};
		VK_CreateShaderDescriptor(0,0, VK_FORMAT_R32G32B32_SFLOAT, 0),
		VK_CreateShaderDescriptor(0,1, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(float))
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &bindings_description;
	vertex_input_info.vertexAttributeDescriptionCount = NUM(attribute_description);
	vertex_input_info.pVertexAttributeDescriptions = attribute_description;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) renderer->window_width;
	viewport.height = (float) renderer->window_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors = {0};
	scissors.offset.x = 0;
	scissors.offset.y = 0;
	scissors.extent.width = renderer->window_width;
	scissors.extent.height = renderer->window_height;

	VkPipelineViewportStateCreateInfo viewport_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewport_info.viewportCount = 1;
	viewport_info.pViewports = &viewport;
	viewport_info.scissorCount = 1;
	viewport_info.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterization_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterization_info.depthClampEnable = VK_FALSE;
	rasterization_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_info.lineWidth = 1.0f;
	rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_info.depthBiasEnable = VK_FALSE;
	rasterization_info.depthBiasConstantFactor = 0.0f;
	rasterization_info.depthBiasClamp = 0.0f;
	rasterization_info.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisample_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisample_info.sampleShadingEnable = VK_FALSE;
	//multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_info.rasterizationSamples = renderer->sample_count;
	multisample_info.minSampleShading = 1.0f;
	multisample_info.pSampleMask = NULL;
	multisample_info.alphaToCoverageEnable = VK_FALSE;
	multisample_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	depth_stencil_info.depthTestEnable = VK_TRUE;
	depth_stencil_info.depthWriteEnable = VK_TRUE;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	//depth_stencil_info.minDepthBounds = 0.0f; // Optional
	//depth_stencil_info.maxDepthBounds = 1.0f; // OptionaldepthStencil.stencilTestEnable = VK_FALSE;
	//depth_stencil_info.stencilTestEnable = VK_FALSE;
	//depth_stencil_info.front = {0}; // Optional
	//depth_stencil_info.back = {0}; // Optional

	VkPipelineColorBlendAttachmentState color_blend_attachment_info = {0};
	color_blend_attachment_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment_info.blendEnable = VK_FALSE;
	color_blend_attachment_info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment_info.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment_info.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment_info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment_info.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blend_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	color_blend_info.logicOpEnable = VK_FALSE;
	color_blend_info.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blend_info.attachmentCount = 1;
	color_blend_info.pAttachments = &color_blend_attachment_info;
	color_blend_info.blendConstants[0] = 0.0f; // Optional
	color_blend_info.blendConstants[1] = 0.0f; // Optional
	color_blend_info.blendConstants[2] = 0.0f; // Optional
	color_blend_info.blendConstants[3] = 0.0f; // Optional

	VkGraphicsPipelineCreateInfo pipeline_info = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rasterization_info;
	pipeline_info.pMultisampleState = &multisample_info;		//Optional depth and stencil	
	pipeline_info.pDepthStencilState = &depth_stencil_info;
	pipeline_info.pColorBlendState = &color_blend_info;
	pipeline_info.pDynamicState = NULL; // Optional
	pipeline_info.layout = renderer->pipeline_layout;
	pipeline_info.renderPass = renderer->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipeline_info.basePipelineIndex = -1; // Optional

	assert(vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &renderer->graphics_pipeline) == VK_SUCCESS);

	vkDestroyShaderModule(renderer->device, vert_shader_module, NULL);
	vkDestroyShaderModule(renderer->device, frag_shader_module, NULL);
}

void VK_CreateCommandPool(VK_Renderer *renderer){

	VkCommandPoolCreateInfo pool_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	pool_info.queueFamilyIndex = renderer->queue_family_index; //assuming one graphics queue family
	pool_info.flags = 0; // Optional

	assert(vkCreateCommandPool(renderer->device, &pool_info, NULL, &renderer->command_pool) == VK_SUCCESS);
}


void VK_CreateImage(VK_Renderer *renderer, VkImage *image, VkDeviceMemory *data,  uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits sample_count,  VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties){

	VkImageCreateInfo image_info = {.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = mip_levels;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.samples = sample_count;
	image_info.flags = 0;

	assert(vkCreateImage(renderer->device, &image_info, NULL, image) == VK_SUCCESS);

	VkMemoryRequirements image_mem_requirements;
	vkGetImageMemoryRequirements(renderer->device, *image, &image_mem_requirements);

	VkMemoryAllocateInfo alloc_info = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	alloc_info.allocationSize = image_mem_requirements.size;
	alloc_info.memoryTypeIndex = VK_GetMemoryType(renderer, image_mem_requirements.memoryTypeBits, properties);

	assert(vkAllocateMemory(renderer->device, &alloc_info, NULL, data) == VK_SUCCESS);

	vkBindImageMemory(renderer->device, *image, *data, 0);
}

void VK_CreateColorResource(VK_Renderer *renderer){

	VK_CreateImage(renderer,
			&renderer->color_image, &renderer->color_image_allocation,
			renderer->window_width, renderer->window_height,
			1, renderer->sample_count,
			renderer->swapchain_image_format,  
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	renderer->color_image_view = VK_CreateImageView(renderer,
			renderer->color_image, renderer->swapchain_image_format,
			VK_IMAGE_ASPECT_COLOR_BIT, 1,
			false);
}

void VK_CreateDepthResource(VK_Renderer *renderer){

	VK_CreateImage(renderer,
			&renderer->depth_image, &renderer->depth_image_allocation,
			renderer->window_width, renderer->window_height,
			1, renderer->sample_count,
			renderer->depth_image_format, 
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	renderer->depth_image_view = VK_CreateImageView(renderer,
			renderer->depth_image, renderer->depth_image_format,
			VK_IMAGE_ASPECT_DEPTH_BIT, 1,
			false);
}

void VK_CreateFramebuffers(VK_Renderer *renderer){

	for (uint32_t i = 0; i < renderer->swapchain_images_count; ++i) {

		VkImageView framebuffer_attachments[] = {
			renderer->color_image_view,
			renderer->depth_image_view,
			renderer->swapchain_images_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info = {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebuffer_info.renderPass = renderer->render_pass;
		framebuffer_info.attachmentCount = NUM(framebuffer_attachments);
		framebuffer_info.pAttachments = framebuffer_attachments;
		framebuffer_info.width = renderer->window_width;
		framebuffer_info.height = renderer->window_height;
		framebuffer_info.layers = 1;

		assert(vkCreateFramebuffer(renderer->device, &framebuffer_info, NULL, &renderer->swapchain_frame_buffers[i]) == VK_SUCCESS);
	}
}

VkCommandBuffer VK_BeginSingleTimeCommands(VK_Renderer *renderer) {

	VkCommandBufferAllocateInfo alloc_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = renderer->command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(renderer->device, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

void VK_EndSingleTimeCommands(VK_Renderer *renderer, VkCommandBuffer *command_buffer) {

	vkEndCommandBuffer(*command_buffer);

	VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = command_buffer;

	vkQueueSubmit(renderer->device_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(renderer->device_queue);

	vkFreeCommandBuffers(renderer->device, renderer->command_pool, 1, command_buffer);
}

void VK_CreateDescriptionPool(VK_Renderer *renderer){

	VkDescriptorPoolSize ubo_descriptor = {0};
	ubo_descriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_descriptor.descriptorCount = renderer->swapchain_images_count;

	VkDescriptorPoolSize sampler_descriptor = {0};
	sampler_descriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_descriptor.descriptorCount = renderer->swapchain_images_count;

	VkDescriptorPoolSize descriptor_pool_size[] = {ubo_descriptor, sampler_descriptor};

	VkDescriptorPoolCreateInfo descriptor_pool_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	descriptor_pool_info.poolSizeCount = NUM(descriptor_pool_size);
	descriptor_pool_info.pPoolSizes = descriptor_pool_size;
	descriptor_pool_info.maxSets = renderer->swapchain_images_count;

	assert(vkCreateDescriptorPool(renderer->device, &descriptor_pool_info, NULL, &renderer->descriptor_pool) == VK_SUCCESS);
}

void VK_CreateDescriptorSets(VK_Renderer *renderer){

	VkDescriptorSetLayout descriptor_sets_layout[renderer->swapchain_images_count];

	for (uint32_t i = 0; i < renderer->swapchain_images_count; ++i) {
		descriptor_sets_layout[i] = renderer->descriptor_layout;
	}

	VkDescriptorSetAllocateInfo descriptor_alloc_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	descriptor_alloc_info.descriptorPool = renderer->descriptor_pool,
		descriptor_alloc_info.descriptorSetCount = renderer->swapchain_images_count,
		descriptor_alloc_info.pSetLayouts = descriptor_sets_layout;

	assert(vkAllocateDescriptorSets(renderer->device, &descriptor_alloc_info, renderer->descriptor_sets) == VK_SUCCESS);

	for (int i = 0; i < renderer->swapchain_images_count; ++i) {

		VkDescriptorBufferInfo ubo_info = {0};
		ubo_info.buffer = renderer->uniform_buffer[i];
		ubo_info.offset = 0;
		ubo_info.range = sizeof(ubo_t);

		VkDescriptorImageInfo image_info = {0};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = renderer->texture_image_view;
		image_info.sampler = renderer->texture_sampler;

		VkWriteDescriptorSet ubo_descriptor_write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		ubo_descriptor_write.dstSet = renderer->descriptor_sets[i];
		ubo_descriptor_write.dstBinding = 0;
		ubo_descriptor_write.dstArrayElement = 0;
		ubo_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_descriptor_write.descriptorCount = 1;
		ubo_descriptor_write.pBufferInfo = &ubo_info;
		ubo_descriptor_write.pImageInfo = NULL; // Optional
		ubo_descriptor_write.pTexelBufferView = NULL; // Optional

		VkWriteDescriptorSet image_write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		image_write.dstSet = renderer->descriptor_sets[i];
		image_write.dstBinding = 1;
		image_write.dstArrayElement = 0;
		image_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		image_write.descriptorCount = 1;
		image_write.pImageInfo = &image_info;

		VkWriteDescriptorSet descriptor_write[] = {ubo_descriptor_write, image_write};

		vkUpdateDescriptorSets(renderer->device, NUM(descriptor_write), descriptor_write, 0, NULL);
	}
}

void VK_CreateCommandBuffers(VK_Renderer *renderer){

	VkCommandBufferAllocateInfo alloc_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	alloc_info.commandPool = renderer->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = renderer->swapchain_images_count;

	assert(vkAllocateCommandBuffers(renderer->device, &alloc_info, renderer->command_buffers) == VK_SUCCESS);


	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

	VkClearValue depth_clear_value;
	depth_clear_value.depthStencil.depth = 1.0f;
	depth_clear_value.depthStencil.stencil = 0;

	VkClearValue clear_values[2] = {clear_color, depth_clear_value};

	for (int i = 0; i < renderer->swapchain_images_count; ++i) {

		VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = NULL; // Optional

		assert(vkBeginCommandBuffer(renderer->command_buffers[i], &begin_info) == VK_SUCCESS);

		VkRenderPassBeginInfo render_pass_info = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		render_pass_info.renderPass = renderer->render_pass;
		render_pass_info.framebuffer = renderer->swapchain_frame_buffers[i];
		render_pass_info.renderArea.offset.x = 0.0f;
		render_pass_info.renderArea.offset.y = 0.0f;
		render_pass_info.renderArea.extent.width = renderer->window_width;
		render_pass_info.renderArea.extent.height = renderer->window_height;
		render_pass_info.clearValueCount = NUM(clear_values);
		render_pass_info.pClearValues = clear_values;

		vkCmdBeginRenderPass(renderer->command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(renderer->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->graphics_pipeline);

		VkDeviceSize offsets[] = {0};

		vkCmdBindVertexBuffers(renderer->command_buffers[i], 0, 1, &renderer->vertex_buffer, offsets);
		vkCmdBindIndexBuffer(renderer->command_buffers[i], renderer->index_buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(renderer->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline_layout, 0, 1, &renderer->descriptor_sets[i], 0, NULL);
		vkCmdDrawIndexed(renderer->command_buffers[i], renderer->vertices, 1, 0, 0, 0);
		//vkCmdDraw(renderer->command_buffers[i], renderer->vertices, 1, 0, 0);

		vkCmdEndRenderPass(renderer->command_buffers[i]);
		assert(vkEndCommandBuffer(renderer->command_buffers[i]) == VK_SUCCESS);
	}
};

void VK_CreateSyncObjects(VK_Renderer *renderer){

	VkSemaphoreCreateInfo semaphore_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

	VkFenceCreateInfo fence_info = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < 2; ++i) {
		assert(vkCreateSemaphore(renderer->device, &semaphore_info, NULL, &renderer->image_available_semaphore[i]) == VK_SUCCESS);
		assert(vkCreateSemaphore(renderer->device, &semaphore_info, NULL, &renderer->render_finished_semaphore[i]) == VK_SUCCESS);
		assert(vkCreateFence(renderer->device, &fence_info, NULL, &renderer->in_flight_fence[i]) == VK_SUCCESS);
	}
}
void VK_DestroySwapchain(VK_Renderer *renderer){

	vkDestroyImageView(renderer->device, renderer->depth_image_view, NULL);
	vkDestroyImage(renderer->device, renderer->depth_image, NULL);
	vkFreeMemory(renderer->device, renderer->depth_image_allocation, NULL);

	vkDestroyImageView(renderer->device, renderer->color_image_view, NULL);
	vkDestroyImage(renderer->device, renderer->color_image, NULL);
	vkFreeMemory(renderer->device, renderer->color_image_allocation, NULL);

	for (int i = 0; i < renderer->swapchain_images_count; i++) {
		vkDestroyFramebuffer(renderer->device, renderer->swapchain_frame_buffers[i], NULL);
	}

	vkFreeCommandBuffers(renderer->device, renderer->command_pool,  renderer->swapchain_images_count, renderer->command_buffers); 

	vkDestroyPipeline(renderer->device, renderer->graphics_pipeline, NULL);
	vkDestroyPipelineLayout(renderer->device, renderer->pipeline_layout, NULL);
	vkDestroyRenderPass(renderer->device, renderer->render_pass, NULL);

	for (int i = 0; i < renderer->swapchain_images_count; i++) {
		vkDestroyImageView(renderer->device, renderer->swapchain_images_views[i], NULL);
	}

	vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);

	for (int i = 0; i < renderer->swapchain_images_count; i++) {
		vkDestroyBuffer(renderer->device, renderer->uniform_buffer[i], NULL);
		vkFreeMemory(renderer->device, renderer->uniform_buffer_allocation[i], NULL);
	}

	vkDestroyDescriptorPool(renderer->device, renderer->descriptor_pool, NULL);
}
void VK_DestroyRenderer(VK_Renderer *renderer){

	vkDeviceWaitIdle(renderer->device);

	VK_DestroySwapchain(renderer);

	vkDestroySampler(renderer->device, renderer->texture_sampler, NULL);
	vkDestroyImageView(renderer->device, renderer->texture_image_view, NULL);
	vkDestroyImage(renderer->device, renderer->texture_image, NULL);
	vkFreeMemory(renderer->device, renderer->texture_image_allocation, NULL);

	vkDestroyDescriptorSetLayout(renderer->device, renderer->descriptor_layout, NULL);

	vkDestroyBuffer(renderer->device, renderer->index_buffer, NULL);
	vkFreeMemory(renderer->device, renderer->index_buffer_allocation, NULL);

	vkDestroyBuffer(renderer->device, renderer->vertex_buffer, NULL);
	vkFreeMemory(renderer->device, renderer->vertex_buffer_allocation, NULL);

	for (size_t i = 0; i < renderer->swapchain_images_count; i++) {
		vkDestroySemaphore(renderer->device, renderer->render_finished_semaphore[i], NULL);
		vkDestroySemaphore(renderer->device, renderer->image_available_semaphore[i], NULL);
		vkDestroyFence(renderer->device, renderer->in_flight_fence[i], NULL);
	}

	vkDestroyCommandPool(renderer->device, renderer->command_pool, NULL);

	vkDestroyDevice(renderer->device, NULL);
	vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
	vkDestroyInstance(renderer->instance, NULL);

	free(renderer);
}

void VK_RecreateSwapchain(VK_Renderer *renderer, SDL_Window *window){

	vkDeviceWaitIdle(renderer->device);
	
	VK_DestroySwapchain(renderer);

	VK_CreateSwapchain(renderer, window);
	VK_CreateSwapchainImageViews(renderer);
	VK_CreateRenderPass(renderer);
	VK_CreateGraphicsPipeline(renderer);
	VK_CreateColorResource(renderer);
	VK_CreateDepthResource(renderer);
	VK_CreateFramebuffers(renderer);
	VK_CreateUniformBuffer(renderer);
	VK_CreateDescriptionPool(renderer);
	VK_CreateDescriptorSets(renderer);
	VK_CreateCommandBuffers(renderer);
}


