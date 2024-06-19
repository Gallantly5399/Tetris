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

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
//    glm::vec2 texCoord;
    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color;
//        && texCoord == other.texCoord;
    }


//    struct hashFunction {
//        std::size_t operator()(const Vertex& vertex) const{
//            return ((std::hash<glm::vec3>()(vertex.position) ^
//                     (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1);
////                   ^(std::hash<glm::vec2>()(vertex.texCoord) << 1);
//        }
//    };
    static std::array<vk::VertexInputBindingDescription, 1> getBindingDescription() {
        std::array<vk::VertexInputBindingDescription, 1> bindingDescription{
                vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex)};
        return bindingDescription;
    }
    static std::array<vk::VertexInputAttributeDescription,2> getAttributeDescription() {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescription{
                vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
                vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
        };
//                vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))};
        return attributeDescription;
    }
};

struct VertexVector {
    std::vector<Vertex> data;
    int index = 0;
    void push_back(Vertex vertex) {
        if (index >= data.size()) {
            data.push_back(vertex);
        }
        data[index] = vertex;
        index ++;
    }
    void clear() {
        index = 0;
        for (auto &i : data) {
            i.position = glm::vec2(0.0f);
            i.color = glm::vec3(0.0f);
        }
    }
};

struct IndexVector {
    std::vector<uint32_t> data;
    int index = 0;
    void push_back(uint32_t index_) {
        if (index >= data.size()) {
            data.push_back(index_);
        }
        data[index] = index_;
        index ++;
    }
    void clear() {
        index = 0;
        memset(data.data(), 0, sizeof(uint32_t) * data.size());
    }
};

class Render {
public:
    void setKeyCallback(GLFWkeyfun key_callback) {
        this->key_callback = key_callback;
        glfwSetKeyCallback(window, key_callback);
    }
    void clear() {
        vertices.clear();
        indices.clear();
    }
    explicit Render(GLFWkeyfun key_callback = nullptr);
    void resizeData(int size) {
        vertices.data.resize(size);
        indices.data.resize(size);
    }
    void resize(const unsigned int Screen_Width, const unsigned int Screen_Height);
    Render(const unsigned int Screen_Width, const unsigned int Screen_Height, GLFWkeyfun key_callback = nullptr);
    void render(std::vector<Vertex> vertices_, std::vector<uint32_t> indices);
    void render();
    ~Render();
    void setFrameBufferResized(bool frameBufferResized_);
    bool shouldStop();
    void inputCube(VertexVector& vertices, IndexVector& indices, glm::vec2 leftCorner_, float width, float height);
    void inputCube(glm::vec2 leftCorner_worldPosition, float width, float height, glm::vec3 color);
    glm::vec2 blockToWorld(float blockRow, float blockColumn ) {
        return {worldStartX + (blockColumn + 1) * stripe + blockColumn * blockWidth,
                         worldStartY + (blockRow + 1) * stripe + blockRow * blockWidth};
    }
    glm::vec2 worldToScreen(glm::vec2);
    //delete move constructor
    Render(Render&&) = delete;
    //delete move assignment
    Render& operator=(Render&&) = delete;

    const unsigned int stripe = 2, blockWidth = 30;
    unsigned int worldStartX = 200, worldStartY = 100;
private:
    GLFWkeyfun key_callback;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    VertexVector vertices;
    IndexVector indices;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;
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
    unsigned int SCREEN_WIDTH, SCREEN_HEIGHT;

    //file path
    const std::filesystem::path projectPath = static_cast<std::filesystem::path>(FILE_LOCATION);
    const std::filesystem::path resourcesPath = projectPath / "resources";

    GLFWwindow* window;
    vk::Instance instance;
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


    void initVulkan();
    void createSwapChain();
    void cleanup();
    void createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
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
    [[nodiscard]] vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask, uint32_t mipLevels);
    std::vector<uint32_t> readFile(const std::filesystem::path &filePath);
    vk::SampleCountFlagBits msaaCount;
    vk::SampleCountFlagBits getMaxUsableSampleCount();
    vk::Image colorImage;
    vk::DeviceMemory colorImageMemory;
    vk::ImageView colorImageView;
    void createColorResources();
//    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    bool firstDrawVertex = false;
    bool firstDrawIndex = false;
    void createIndexBuffer();

};
