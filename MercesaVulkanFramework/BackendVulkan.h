#pragma once

#include <vector>

#include "RenderingIncludes.h"
#include "GraphicsParameters.h"

#include "vk_mem_alloc.h"


// Backend has all the necessary data, instance, swapchain etc
class iLowLevelWindow;

using namespace vk;

static const int32_t NUM_FRAMES = 2;
static const vk::SampleCountFlagBits NUM_MULTISAMPLES = vk::SampleCountFlagBits::e1;
struct GPUinfo
{
	vk::PhysicalDevice device;
	vk::PhysicalDeviceProperties props;
	vk::PhysicalDeviceMemoryProperties memProps;
	vk::SurfaceCapabilitiesKHR surfaceCaps;
	std::vector<vk::SurfaceFormatKHR> surfaceFormats;
	std::vector<vk::PresentModeKHR> presentModes;
	std::vector<vk::QueueFamilyProperties> queueFamilyProps;
	std::vector<vk::ExtensionProperties> extensionProps;
};

struct vulkanContext
{
public:
	vulkanContext() {};
	~vulkanContext() {};

	vk::CommandBuffer commandBuffer;

	std::vector<CommandBuffer> commandBuffers;
	std::vector<Fence> commandBufferFences;
	std::vector<bool> commandBufferRecorded;


	vk::CommandPool cmdPool;

	int64_t currentFrame = 0;
	int64_t lastFrame = 0;
	int64_t counter = 0;

	GPUinfo* gpu;
	std::vector<GPUinfo> gpus;

	vk::Device device;
	int graphicsFamilyIndex;
	int presentFamilyIndex;
	int computeFamilyIndex;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::Queue computeQueue;

	vk::Format depthFormat;
	vk::RenderPass renderpass;
	vk::SampleCountFlagBits sampleCount;
	bool supersampling;


	GFXParams currentParameters;
};


class BackendVulkan
{

public:
	BackendVulkan() = default;
	~BackendVulkan() = default;

	void Init(const GFXParams& iParams, iLowLevelWindow* const iWindow);
	void Shutdown();

	void BeginFrame();
	void EndFrame(vk::CommandBuffer iSceneRenderBuffer, vk::CommandBuffer iGuiBuffer);
	void BlockSwapBuffers();
	void AcquireImage();
	void BlockUntilGpuIdle();

	vulkanContext context;
	VmaAllocator allocator;

	vk::Format swapchainFormat;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainViews;
	uint32_t currentSwapIndex = 0;

private:

	void CreateInstance(iLowLevelWindow* const iWindow);
	void CreateSurface(iLowLevelWindow* const iWindow);

	void EnumeratePhysicalDevices();
	void SelectPhysicalDevice();

	void CreateLogicalDeviceAndQueues();
	void CreateMemoryAllocator();

	void CreateSemaphores();
	void DestroySemaphores();

	void CreateSwapChain();
	void DestroySwapchain();

	void CreateRenderTargets();
	void DestroyRenderTargets();

	void CreateFramebuffers();
	void DestroyFramebuffers();

	void CreateDepthbuffer();
	void DestroyDepthBuffer();

	void CreateRenderpass();

	void CreateCommandBuffer();
	void CreateCommandPool();
	void DestroyCmdPoolAndBuffers();

	// Two functions which are used to get the instance layers we want
	void GetInstanceLayers(std::vector<const char*>& iResult);
	void GetInstanceExtensions(std::vector<const char*>& iResult);
	std::vector<const char*> GetDeviceExtensions();

	void CreateDebugCallbacks();


private:
	vk::Instance instance;
	vk::PhysicalDevice physicalDevice;
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	vk::SurfaceKHR surface;


	std::vector<const char*> instanceExtensions;
	std::vector<const char*> instanceLayers;
	std::vector<const char*> deviceExtensions;
	// Device layers are deprecated

	bool fullscreen = false;


	vk::SwapchainKHR swapchain;
	vk::PresentModeKHR presentMode;
	vk::Extent2D swapchainExtent;

	// msaa stuff
	vk::Image msaaImage;
	vk::ImageView msaaImageView;
	VmaAllocation msaaVmaAllocation;
	VmaAllocationInfo msaaAllocation;

	vk::Image depthImage;
	vk::ImageView depthView;
	VmaAllocation depthAllocation;


	std::vector<vk::Framebuffer> framebuffers;

	std::vector<vk::Semaphore> acquireSemaphores;
	std::vector<vk::Semaphore> renderCompleteSemaphore;

	VkDebugReportCallbackEXT callBack;


};