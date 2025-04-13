﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vk_descriptors.h>

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;
	//understand what a lambda function is
	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)();
		}
		deletors.clear();
	}
};

struct FrameData {
	VkCommandPool _commandpool;
	VkCommandBuffer _mainCommandBuffer;
	VkSemaphore _swapchainSemaphore, _renderSemaphore;
	VkFence _renderFence;
	DeletionQueue _deletionQueue;
};



constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
public:

	bool _isInitialized{ false };
	int _frameNumber {0};
	bool stop_rendering{ false };
	VkExtent2D _windowExtent{ 1000 , 650 };
	bool bUseValidationLayers = true;

	VkInstance _instance; // Vulkan Lib Handle
	VkDebugUtilsMessengerEXT _debug_messenger; //Vulkan debug output handle
	VkPhysicalDevice _chosenGPU; //GPU chosen as the default device
	VkDevice _device; //Vulkan device for commands
	VkSurfaceKHR _surface; //Vulkan Window Surface

	//Vars for swapchain
	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchianImageViews;
	VkExtent2D _swapchainExtent;

	//For Vulkan Commands
	FrameData _frames[FRAME_OVERLAP];
	FrameData& get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;
	struct SDL_Window* _window{ nullptr };

	static VulkanEngine& Get();
	VmaAllocator _allocator;

	AllocatedImage _drawImage;
	VkExtent2D _drawExtent;

	DescriptorAllocator globalDescriptorAllocator;
	VkDescriptorSet _drawImageDescriptors;
	VkDescriptorSetLayout _drawImageDescriptorLayout;

	VkPipeline _gradientPipeline;
	VkPipelineLayout _gradientPipelineLayout;

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	void draw_background(VkCommandBuffer cmd);

	//run main loop
	void run();

private:
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();
	void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();
	void init_descriptors();
	void init_pipelines();
	void init_background_pipelines();

	DeletionQueue _mainDeletionQueue;
};
