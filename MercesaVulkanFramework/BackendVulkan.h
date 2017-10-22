#pragma once

#include "RenderingIncludes.h"
#include "GraphicsParameters.h"

#include "vk_mem_alloc.h"


// Backend has all the necessary data, instance, swapchain etc
class iLowLevelWindow;

static const int32_t NUM_FRAMES = 2;
static const vk::SampleCountFlagBits MULTISAMPLES = vk::SampleCountFlagBits::e1;
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

	GPUinfo* gpu;
	std::vector<GPUinfo> gpus;

	vk::Device device;
	int graphicsFamilyIndex;
	int presentFamilyIndex;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

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

private:

	void CreateInstance(iLowLevelWindow* const iWindow);
	void CreateSurface(iLowLevelWindow* const iWindow);

	void EnumeratePhysicalDevices();
	void SelectPhysicalDevice();

	void CreateLogicalDeviceAndQueues();
	void CreateMemoryAllocator();

	void CreateSemaphores();

	void CreateSwapChain();
	void DestroySwapchain();

	void CreateRenderTargets();
	void DestroyRenderTargets();

	void CreateFramebuffers();
	void DestroyFramebuffers();

	void CreateDepthbuffer();
	void DestroyDepthBuffer();

	void CreateRenderpass();
	void DestroyRenderpass();

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

	VmaAllocator allocator;

	std::vector<const char*> instanceExtensions;
	std::vector<const char*> instanceLayers;
	std::vector<const char*> deviceExtensions;
	// Device layers are deprecated

	bool fullscreen = false;

	vulkanContext context;

	vk::SwapchainKHR swapchain;
	vk::PresentModeKHR presentMode;
	vk::Format swapchainFormat;
	vk::Extent2D swapchainExtent;

	// msaa stuff
	vk::Image msaaImage;
	vk::ImageView msaaImageView;
	VmaAllocation msaaVmaAllocation;
	VmaAllocationInfo msaaAllocation;

	vk::Image depthImage;
	vk::ImageView depthView;
	VmaAllocation depthAllocation;

	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainViews;

	std::vector<vk::Framebuffer> framebuffers;

	std::vector<vk::Semaphore> acquireSemaphores;
	std::vector<vk::Semaphore> renderCompleteSemaphore;

	VkDebugReportCallbackEXT callBack;


};