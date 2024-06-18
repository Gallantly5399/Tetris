//
// Created by amemiya7 on 2024/5/3.
//

#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"
#include <optional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <filesystem>
#include <unordered_map>




class Render {
public:
    void run();
    void setFrameBufferResized(bool frameBufferResized_);
private:
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCoord;
        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && texCoord == other.texCoord;
        }

        struct hashFunction {
            std::size_t operator()(const Vertex& vertex) const{
                return ((std::hash<glm::vec3>()(vertex.position) ^
                         (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                       (std::hash<glm::vec2>()(vertex.texCoord) << 1);
            }
        };
        static std::array<vk::VertexInputBindingDescription, 1> getBindingDescription() {
            std::array<vk::VertexInputBindingDescription, 1> bindingDescription{
                    vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex)};
            return bindingDescription;
        }
        static std::array<vk::VertexInputAttributeDescription,3> getAttributeDescription() {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescription{
                    vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
                    vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
                    vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))};
            return attributeDescription;
        }
    };


    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t, Vertex::hashFunction> uniqueVertex{};

//    const std::vector<Vertex> vertices = {
//            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
//
//            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
//    };
//    const std::vector<uint16_t> indices = {
//            0, 1, 2, 2, 3, 0,
//            4, 5, 6, 6, 7, 4
//    };
    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::PhysicalDevice physicalDevice;
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const int MAX_FRAMES_IN_FLIGHT = 2;
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
    const int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 800;

    //file path
    const std::filesystem::path projectPath = static_cast<std::filesystem::path>(FILE_LOCATION);
    const std::filesystem::path resourcesPath = projectPath / "resources";
    const std::filesystem::path modelPath = resourcesPath / "models/viking_room.obj";
    const std::filesystem::path texturePath = resourcesPath / "models/viking_room.png";

    GLFWwindow* window;
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::DebugUtilsMessengerEXT callback;
    vk::Device device;
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    uint32_t currentFrame = 0;
    bool frameBufferResized = false;

    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;
    vk::CommandPool commandPool;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;

    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    void initWindow();
    void initVulkan();
    void createSwapChain();
    void mainLoop();
    void cleanup();
    void createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(vk::PhysicalDevice device);
    void pickPhysicalDevice();
    int rateDeviceSuitable(vk::PhysicalDevice device);
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
    void createLogicalDevice();
    void createSurface();
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
    [[nodiscard]] vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    [[nodiscard]] vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    [[nodiscard]] vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    void createImageViews();
    void createGraphicsPipeline();
    static std::vector<uint32_t> readFile(const std::string &filename);
    [[nodiscard]] vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code);
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void createSyncObjects();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createVertexBuffers();
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    [[nodiscard]] std::tuple<vk::Buffer, vk::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;


    void createIndexBuffer();

    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };
    vk::DescriptorSetLayout descriptorSetLayout;
    void createDescriptorSetLayout();
    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentFrame);
    void createDescriptorPool();
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
    void createDescriptorSets();
    void createTextureImage();

    uint32_t mipLevels;
    vk::Image textureImage;
    vk::ImageView textureImageView;
    vk::Sampler textureSampler;
    vk::DeviceMemory textureImageMemory;
    [[nodiscard]] std::tuple<vk::Image, vk::DeviceMemory> createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount,
                                                                      vk::Format format, vk::ImageTiling imageTiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags memoryProperty);
    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
    [[nodiscard]] vk::CommandBuffer beginCommands();
    void endCommands(vk::CommandBuffer commandBuffer);
    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
    void createTextureImageView();
    [[nodiscard]] vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask, uint32_t mipLevels);
    void createTextureSampler();
    vk::Image depthImage;
    vk::ImageView depthImageView;
    vk::DeviceMemory depthImageMemory;
    [[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    [[nodiscard]] vk::Format findDepthFormat();
    bool hasStencilComponent(vk::Format format);
    void createDepthResources();
    std::vector<uint32_t> readFile(const std::filesystem::path &filePath);
    void loadModel();
    void generateMipmaps(vk::Image image, vk::Format format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    vk::SampleCountFlagBits msaaCount;
    vk::SampleCountFlagBits getMaxUsableSampleCount();
    vk::Image colorImage;
    vk::DeviceMemory colorImageMemory;
    vk::ImageView colorImageView;
    void createColorResources();
};
