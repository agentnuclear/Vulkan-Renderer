﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_loader.h>
#include <vk_types.h>
#include <vk_descriptors.h>
#include <camera.h>


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
	DescriptorAllocatorGrowable _frameDescriptors;
};

struct ComputePushConstants {
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};

struct ComputeEffect {
	const char* name;

	VkPipeline pipeline;
	VkPipelineLayout layout;

	ComputePushConstants data;
};

struct GPUSceneData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection;
	glm::vec4 sunlightColor;
};

struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors;
		glm::vec4 metal_rough_factors;
		//padding, we need it anyway for uniform buffers
		glm::vec4 extra[14];
	};

	struct MaterialResources {
		AllocatedImage colorImage;
		VkSampler colorSampler;
		AllocatedImage metalRoughImage;
		VkSampler metalRoughSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	DescriptorWriter writer;

	void build_pipelines(VulkanEngine* engine);
	void clear_resources(VkDevice device);

	MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
};

//Node stuff
struct MeshNode : public Node {

	std::shared_ptr<MeshAsset> mesh;

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
};

struct RenderObject {
	uint32_t indexCount;
	uint32_t firstIndex;
	VkBuffer indexBuffer;

	MaterialInstance* material;
	Bounds bounds;
	glm::mat4 transform;
	VkDeviceAddress vertexBufferAddress;
};

struct DrawContext {
	std::vector<RenderObject> OpaqueSurfaces;
	std::vector<RenderObject> TransparentSurfaces;

};

struct EngineStats {
	float frametime;
	int triangle_count;
	int drawcall_count;
	float scene_update_time;
	float mesh_draw_time;
};


constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
public:

	bool _isInitialized{ false };
	int _frameNumber {0};
	bool stop_rendering{ false };
	VkExtent2D _windowExtent{ 1000 , 650 };
	bool bUseValidationLayers = false;

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
	AllocatedImage _depthImage;
	VkExtent2D _drawExtent;
	float renderScale = 1.0f;


	DescriptorAllocatorGrowable globalDescriptorAllocator;
	VkDescriptorSet _drawImageDescriptors;
	VkDescriptorSetLayout _drawImageDescriptorLayout;

	VkPipeline _gradientPipeline;
	VkPipelineLayout _gradientPipelineLayout;

	//immidiate submit structures
	VkFence _immFence;
	VkCommandBuffer _immCommandBuffer;
	VkCommandPool _immCommandPool;

	std::vector<ComputeEffect> backgroundEffects;
	int currentBackgroundEffect{ 0 };

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

	////Drawing the triangle :: DELETED
	//VkPipelineLayout _trianglePipelineLayout;
	//VkPipeline _trianglePipeline;
	//void init_triangle_pipeline();

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//buffer creation 
	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void destroy_buffer(const AllocatedBuffer& buffer);
	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);
	GPUSceneData sceneData;
	VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;


	//drawing meshes
	VkPipelineLayout _meshPipelineLayout;
	VkPipeline _meshPipeline;
	std::vector<std::shared_ptr<MeshAsset>> testMeshes;
	bool resize_requested;

	void init_mesh_pipeline();
	void init_default_data();
	//draw loop
	void draw();

	void draw_background(VkCommandBuffer cmd);
	void draw_geometry(VkCommandBuffer cmd);
	void draw_imgui(VkCommandBuffer cmd, VkImageView targetImageView);
	void resize_swapchain();

	//run main loop
	void run();

	//Textures
	AllocatedImage create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	AllocatedImage create_image(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	void destroy_image(const AllocatedImage& img);

	AllocatedImage _whiteImage;
	AllocatedImage _blackImage;
	AllocatedImage _greyImage;
	AllocatedImage _errorCheckerboardImage;
	VkSampler _defaultSamplerLinear;
	VkSampler _defaultSamplerNearest;

	//materials
	MaterialInstance defaultData;
	GLTFMetallic_Roughness metalRoughMaterial;

	VkDescriptorSetLayout _singleImageDescriptorLayout;

	//Node
	DrawContext mainDrawContext;
	std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;

	void update_scene();

	std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;

	//Camera
	Camera mainCamera;

	//Stats
	EngineStats stats;

private:
	void PrintAllGPUDetails(VkInstance instance, VkSurfaceKHR surface);
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();
	void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();
	void init_descriptors();
	void init_pipelines();
	void init_background_pipelines();
	void init_imgui();

	DeletionQueue _mainDeletionQueue;
};

bool is_visible(const RenderObject& obj, const glm::mat4& viewproj);
