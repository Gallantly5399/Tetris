//
// Created by amemiya7 on 2024/4/30.
//

#include "Window.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <map>
#include <fstream>
#include <set>
#include <optional>
#include <algorithm>
#include <limits>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void Render::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

static void frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    app->setFrameBufferResized(true);
}
void Render::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, frameBufferResizedCallback);
}

void Render::initVulkan() {
    createInstance();
//    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
    createVertexBuffers();
    createIndexBuffer();
    createUniformBuffers();
    createCommandBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createSyncObjects();
}

void Render::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }
    device.waitIdle();
}

void Render::cleanup() {
    cleanupSwapChain();
    device.destroy(colorImage);
    device.destroy(colorImageView);
    device.free(colorImageMemory);
    device.destroy(textureSampler);
    device.destroy(textureImageView);
    device.destroy(textureImage);
    device.free(textureImageMemory);
    device.destroy(descriptorPool);
    device.destroy(descriptorSetLayout);
    for (size_t i = 0;i < MAX_FRAMES_IN_FLIGHT;i ++) {
        device.destroy(uniformBuffers[i]);
        device.free(uniformBuffersMemory[i]);
    }
    device.destroy(vertexBuffer);
    device.free(vertexBufferMemory);
    device.destroy(indexBuffer);
    device.free(indexBufferMemory);
    for (int i = 0;i < MAX_FRAMES_IN_FLIGHT; i ++) {
        device.destroy(renderFinishedSemaphores[i]);
        device.destroy(imageAvailableSemaphores[i]);
        device.destroy(inFlightFences[i]);
    }
    device.destroy(commandPool);
    device.destroy(graphicsPipeline);
    device.destroy(pipelineLayout);
    device.destroy(renderPass);
    device.destroy();
//    if (enableValidationLayers) {
//        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldy);
//    }
    instance.destroy(surface);
    instance.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Render::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation_layers_requested_but_not_available");
    }
    vk::ApplicationInfo appInfo("Hello_Triangle", {}, "No_Engine", {}, vk::ApiVersion13);

    std::vector<const char*> requiredExtensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo = {};
    createInfo.setPApplicationInfo(&appInfo)
            .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
            .setEnabledExtensionCount(requiredExtensions.size())
            .setPpEnabledExtensionNames(requiredExtensions.data());

    if (enableValidationLayers) {
        createInfo.setEnabledLayerCount(validationLayers.size())
                .setPpEnabledLayerNames(validationLayers.data());
    } else {
        createInfo.setEnabledLayerCount(0);
    }

    instance = vk::createInstance(createInfo);
}

bool Render::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    auto availableLayers = vk::enumerateInstanceLayerProperties();
//    std::vector<VkLayerProperties> availableLayers(layerCount);
//    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

std::vector<const char *> Render::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
//    if (enableValidationLayers) {
//        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//    }
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    return extensions;
}


void Render::pickPhysicalDevice() {

    physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("failed_to_find_GPUs_with_Vulkan_support");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for(const auto& device: devices) {
        int score = rateDeviceSuitable(device);
        candidates.insert(std::make_pair(score, device));
    }
    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
        msaaCount = getMaxUsableSampleCount();
    } else {
        throw std::runtime_error("failed_to_find_a_suitable_GPU");
    }
}

int Render::rateDeviceSuitable(vk::PhysicalDevice device) {
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
    VkPhysicalDeviceFeatures deviceFeatures = device.getFeatures();
    int score = 0;
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    }
    score += deviceProperties.limits.maxImageDimension2D;
    return score;
}

Render::QueueFamilyIndices Render::findQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indices;
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = device.getSurfaceSupportKHR(i, surface);
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;

}

bool Render::isDeviceSuitable(vk::PhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.isComplete() && extensionSupported && swapChainAdequate;
}

void Render::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo({}, queueFamily, 1, &queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.setSamplerAnisotropy(vk::True);
    deviceFeatures.setSampleRateShading(vk::True);
    vk::DeviceCreateInfo createInfo({}, queueCreateInfos, {}, deviceExtensions, &deviceFeatures);

    if (enableValidationLayers) {
        createInfo.setEnabledLayerCount(validationLayers.size());
        createInfo.setPpEnabledLayerNames(validationLayers.data());
    } else {
        createInfo.setEnabledLayerCount(0);
    }

    device = physicalDevice.createDevice(createInfo);
    device.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
    device.getQueue(indices.presentFamily.value(), 0, &presentQueue);
}

void Render::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, (VkSurfaceKHR*)(&surface)) != VK_SUCCESS) {
        throw std::runtime_error("failed_to_create_window_surface");
    }
}

bool Render::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

Render::SwapChainSupportDetails Render::querySwapChainSupport(vk::PhysicalDevice device) {
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);
    return details;
}

vk::SurfaceFormatKHR Render::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear};
    }
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR Render::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
    auto bestMode = vk::PresentModeKHR::eFifo;
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
        else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
            bestMode = availablePresentMode;
        }
    }
    return bestMode;
}

vk::Extent2D Render::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        vk::Extent2D actualExtent(width, height);
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}


void Render::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo({}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
                                          extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
    createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    createInfo.setPresentMode(presentMode);
    createInfo.setClipped(true);
    createInfo.setOldSwapchain(nullptr);

    if (device.createSwapchainKHR(&createInfo, {}, &swapChain) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create swap chain!");
    }
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages = device.getSwapchainImagesKHR(swapChain);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Render::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);
    }
}

void Render::createGraphicsPipeline() {

    const std::filesystem::path vertShaderPath = resourcesPath / "shaders/vert.spv";
    const std::filesystem::path fragShaderPath = resourcesPath / "shaders/frag.spv";
    assert(std::filesystem::exists(vertShaderPath));
    assert(std::filesystem::exists(fragShaderPath));
    auto vertShaderCode = readFile(vertShaderPath);
    auto fragShaderCode = readFile(fragShaderPath);
    vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    auto bindDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescription();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindDescription,attributeDescription);

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, vk::False);

    vk::Viewport viewport(0.0f, 0.0f, swapChainExtent.width, swapChainExtent.height, 0.0f, 1.0f);

    vk::Rect2D scissor({0, 0}, swapChainExtent);

    vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizer({}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
                                                        vk::False, {}, {}, {}, 1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling({}, msaaCount, vk::True, 0.2f, {}, {}, {});

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setBlendEnable(vk::False);
    colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo colorBlending({}, false, vk::LogicOp::eCopy, 1, &colorBlendAttachment,
                                                        {0.0f, 0.0f, 0.0f, 0.0f});
    std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 1, &descriptorSetLayout, 0, nullptr);

    if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_pipeline_layout");
    }

    vk::PipelineDepthStencilStateCreateInfo depthStencil({},
                                                         vk::True, vk::True, vk::CompareOp::eLess,
                                                         vk::False, vk::False,
                                                         {}, {}, {}, {});


    vk::GraphicsPipelineCreateInfo pipelineInfo({}, 2, shaderStages, &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer,
                                                &multisampling, &depthStencil, &colorBlending, &dynamicState, pipelineLayout, renderPass, 0, nullptr,0);


    if (device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_graphics_pipeline");
    }

    device.destroy(vertShaderModule, nullptr);
    device.destroy(fragShaderModule, nullptr);
}

//desperate
std::vector<uint32_t> Render::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed_to_open_file");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<uint32_t> buffer(fileSize);
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();
    return buffer;
}

std::vector<uint32_t> Render::readFile(const std::filesystem::path &filePath) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed_to_open_file");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<uint32_t> buffer(fileSize);
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();
    return buffer;
}

vk::ShaderModule Render::createShaderModule(const std::vector<uint32_t>& code) {
    vk::ShaderModuleCreateInfo createInfo({}, code.size(), code.data());
    vk::ShaderModule shaderModule;
    if (device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create shader module");
    }
    return shaderModule;
}

void Render::createRenderPass() {
    vk::AttachmentDescription colorAttachment({}, swapChainImageFormat, msaaCount, vk::AttachmentLoadOp::eClear,
                                              vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentDescription depthAttachment({}, findDepthFormat(), msaaCount, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                                              vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDependency dependency(vk::SubpassExternal, 0,
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                     vk::AccessFlagBits::eNone,
                                     vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    vk::AttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentRef, &colorAttachmentResolveRef, &depthAttachmentRef);



    std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};




    vk::RenderPassCreateInfo renderPassInfo({}, attachments, subpass, dependency);




    if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_render_pass!");
    }
}

//void Render::setupDebugMessenger() {
//
////    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
////    debugInfo.messageSeverity =
////            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
////            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
////            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
////
////    debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
////                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
////                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
////    debugInfo.pfnUserCallback = &debugCallback;
////    vk::DebugUtilsMessengerEXT debugReport = instance.createDebugUtilsMessengerEXT(debugInfo, nullptr, dldy);
//
////    if (instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger) != vk::Result::eSuccess) {
////        throw std::runtime_error("failed_to_set_up_debug_messenger!");
////    }
//}

void Render::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 3> attachments = {colorImageView, depthImageView, swapChainImageViews[i]};
        vk::FramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.setRenderPass(renderPass)
                .setAttachments(attachments)
                .setWidth(swapChainExtent.width)
                .setHeight(swapChainExtent.height)
                .setLayers(1);
        if (device.createFramebuffer(&framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("failed_to_create_framebuffer");
        }
    }

}

void Render::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndices.graphicsFamily.value());

    if (device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_command_pool");
    }
}

void Render::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, commandBuffers.size());
    commandBuffers = device.allocateCommandBuffers(allocateInfo);
}

void Render::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to begin to record command!");
    };
    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].setColor({0.0f, 0.0f, 0.0f, 0.0f});
    clearValues[1].setDepthStencil({1.0f, 0});

    vk::RenderPassBeginInfo renderPassBeginInfo{renderPass, swapChainFramebuffers[imageIndex], {{0, 0}, swapChainExtent}, clearValues.size(), clearValues.data()};

    commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({0, 0}, swapChainExtent);
    commandBuffer.setScissor(0, 1, &scissor);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    vk::DeviceSize  offset{0};
    commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void Render::drawFrame() {
    if (device.waitForFences(inFlightFences[currentFrame], vk::True, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait for fences");
    }
    auto [Result, imageIndex] = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], {});
    if (Result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    } else if (Result != vk::Result::eSuccess && Result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to require swap chain image!");
    }

    if (device.resetFences(1, &inFlightFences[currentFrame]) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to reset fences");
    }

    commandBuffers[currentFrame].reset();
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    updateUniformBuffer(currentFrame);

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1, &commandBuffers[currentFrame], 1, signalSemaphores);

    if (graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    vk::SwapchainKHR swapChains[] = {swapChain};

    vk::PresentInfoKHR presentInfo(1, signalSemaphores, 1, swapChains, &imageIndex);

    Result = presentQueue.presentKHR(&presentInfo);
    if (Result == vk::Result::eErrorOutOfDateKHR || Result == vk::Result::eSuboptimalKHR || frameBufferResized) {
        frameBufferResized = false;
        recreateSwapChain();
    } else if (Result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Render::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
    for(size_t i = 0;i < MAX_FRAMES_IN_FLIGHT; i ++) {
        imageAvailableSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
        renderFinishedSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
        inFlightFences[i] = device.createFence(fenceCreateInfo);
    }
}

void Render::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while(width == 0 || height == 0) {
        std::cout << width << ' ' << height << std::endl;
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    device.waitIdle();
    cleanupSwapChain();
    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
    createFramebuffers();
}

void Render::cleanupSwapChain() {
    device.destroy(depthImage);
    device.destroy(depthImageView);
    device.free(depthImageMemory);
    for (size_t i = 0;i < swapChainFramebuffers.size();i ++) {
        device.destroy(swapChainFramebuffers[i]);
    }
    for (size_t i = 0;i < swapChainImageViews.size();i ++) {
        device.destroy(swapChainImageViews[i]);
    }
    device.destroy(swapChain);
}

void Render::setFrameBufferResized(bool frameBufferResized_) {
    frameBufferResized = frameBufferResized_;
}

void Render::createVertexBuffers() {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data =  device.mapMemory(stagingBufferMemory, 0, bufferSize, {});
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    device.unmapMemory(stagingBufferMemory);

    std::tie(vertexBuffer, vertexBufferMemory) = createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                                              vk::MemoryPropertyFlagBits::eDeviceLocal);
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    device.destroy(stagingBuffer);
    device.free(stagingBufferMemory);
}

uint32_t Render::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0;i < memoryProperties.memoryTypeCount;i ++) {
        if (typeFilter & (1 << i) && memoryProperties.memoryTypes[i].propertyFlags & properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

std::tuple<vk::Buffer, vk::DeviceMemory> Render::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
    buffer = device.createBuffer(bufferInfo);
    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);
    vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, properties));
    bufferMemory = device.allocateMemory(allocateInfo);
    device.bindBufferMemory(buffer, bufferMemory, 0);
    return std::make_tuple(buffer, bufferMemory);
}

void Render::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    vk::BufferCopy bufferCopy(0, 0, size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, bufferCopy);


    commandBuffer.end();
    vk::SubmitInfo submitInfo({}, {}, commandBuffer);
    graphicsQueue.submit(submitInfo);
    graphicsQueue.waitIdle();
    device.freeCommandBuffers(commandPool, commandBuffer);

}

void Render::createIndexBuffer() {
    vk::DeviceSize size = sizeof(indices[0]) * indices.size();
    auto [stagingBuffer, stagingBufferMemory] = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                                                             vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
    void *data = device.mapMemory(stagingBufferMemory, 0, size, {});
    memcpy(data, indices.data(), size);
    device.unmapMemory(stagingBufferMemory);

    std::tie(indexBuffer, indexBufferMemory) = createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                                                            vk::MemoryPropertyFlagBits::eDeviceLocal);
    copyBuffer(stagingBuffer, indexBuffer, size);
    device.destroy(stagingBuffer);
    device.free(stagingBufferMemory);
}

void Render::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                                    vk::ShaderStageFlagBits::eVertex);
    vk::DescriptorSetLayoutBinding samplerBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerBinding};
    vk::DescriptorSetLayoutCreateInfo createInfo({}, bindings);
    descriptorSetLayout = device.createDescriptorSetLayout(createInfo);

}

void Render::createUniformBuffers() {
    vk::DeviceSize size = sizeof(UniformBufferObject);
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0;i < MAX_FRAMES_IN_FLIGHT; i ++) {
        std::tie(uniformBuffers[i], uniformBuffersMemory[i]) = createBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer,
                                                                            vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
        uniformBuffersMapped[i] = device.mapMemory(uniformBuffersMemory[i], 0, size);
    }

}

void Render::updateUniformBuffer(uint32_t currentFrame) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    //TODO::rotate
//    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

}

void Render::createDescriptorPool() {
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0] = {vk::DescriptorType::eUniformBuffer, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)};
    poolSizes[1] = {vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)};
    vk::DescriptorPoolCreateInfo createInfo({}, MAX_FRAMES_IN_FLIGHT, poolSizes);
    descriptorPool = device.createDescriptorPool(createInfo);
}

void Render::createDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, MAX_FRAMES_IN_FLIGHT, layouts.data());
    descriptorSets = device.allocateDescriptorSets(allocInfo);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo(uniformBuffers[i], 0, sizeof(UniformBufferObject));
        vk::DescriptorImageInfo imageInfo(textureSampler, textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0] = {descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo, nullptr};
        descriptorWrites[1] = {descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, imageInfo, {}, {}};
        device.updateDescriptorSets(descriptorWrites, 0);
    }
}

void Render::createTextureImage() {
    const vk::Format format = vk::Format::eR8G8B8A8Srgb;
    const std::filesystem::path texturePath = resourcesPath / "models/viking_room.png";
    assert(std::filesystem::exists(texturePath));
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture");
    }
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    vk::DeviceSize size = texWidth * texHeight * 4;
    auto [buffer, bufferMemory] = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void* data = device.mapMemory(bufferMemory, 0, size);
    memcpy(data, pixels, size);
    device.unmapMemory(bufferMemory);
    stbi_image_free(pixels);

    std::tie(textureImage, textureImageMemory) =
            createImage(texWidth, texHeight, mipLevels, vk::SampleCountFlagBits::e1, format, vk::ImageTiling::eOptimal,
                        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                        vk::MemoryPropertyFlagBits::eDeviceLocal);
    transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
    copyBufferToImage(buffer, textureImage, texWidth, texHeight);
    generateMipmaps(textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, mipLevels);
    device.destroy(buffer);
    device.free(bufferMemory);
}

std::tuple<vk::Image, vk::DeviceMemory>
Render::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling imageTiling,
                        vk::ImageUsageFlags usage, vk::MemoryPropertyFlags memoryProperty) {
    vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, format,
                                   {width, height, 1}, mipLevels, 1, sampleCount,
                                   imageTiling, usage,
                                   vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined);
    vk::Image image = device.createImage(createInfo);
    vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(image);
    vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, memoryProperty));
    vk::DeviceMemory memory = device.allocateMemory(allocateInfo);
    device.bindImageMemory(image, memory, 0);
    return std::make_tuple(image, memory);
}

void Render::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout,
                                       vk::ImageLayout newLayout, uint32_t mipLevels) {
    vk::CommandBuffer commandBuffer = beginCommands();

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    vk::AccessFlags srcAccessMask{}, dstAccessMask{};
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        srcAccessMask = vk::AccessFlagBits::eNone;
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    vk::ImageMemoryBarrier barrier(srcAccessMask, dstAccessMask, oldLayout, newLayout,
                                   vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
                                   {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);

    endCommands(commandBuffer);
}

vk::CommandBuffer Render::beginCommands() {
    vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);
    return commandBuffer;
}

void Render::endCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();
    vk::SubmitInfo submitInfo({}, {}, commandBuffer);
    graphicsQueue.submit(submitInfo);
    graphicsQueue.waitIdle();
    device.freeCommandBuffers(commandPool, commandBuffer);
}

void Render::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
    vk::CommandBuffer commandBuffer = beginCommands();
    vk::BufferImageCopy region(0, 0, 0,
                               {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                               0, {width, height, 1});
    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    endCommands(commandBuffer);
}

void Render::createTextureImageView() {
    textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);

}

vk::ImageView Render::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask, uint32_t mipLevels) {
    vk::ImageViewCreateInfo createInfo({}, image, vk::ImageViewType::e2D, format, {}, {aspectMask, 0, mipLevels, 0, 1});;
    vk::ImageView imageView = device.createImageView(createInfo);
    return imageView;
}

void Render::createTextureSampler() {
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    vk::SamplerCreateInfo createInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                                     vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
                                     0.0f, vk::True, properties.limits.maxSamplerAnisotropy, vk::False, vk::CompareOp::eAlways,
                                     0, static_cast<float>(mipLevels),vk::BorderColor::eIntOpaqueBlack, vk::False);
    textureSampler = device.createSampler(createInfo);
}

vk::Format Render::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
                                           vk::FormatFeatureFlags features) {
    for(vk::Format format : candidates) {
        vk::FormatProperties properties = physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
        throw std::runtime_error("failed to find supported format!");
    }
}

vk::Format Render::findDepthFormat() {
    return findSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

bool Render::hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void Render::createDepthResources() {
    vk::Format depthFormat = findDepthFormat();
    std::tie(depthImage, depthImageMemory) = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaCount, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);

}

void Render::loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {
                    1.0f, 1.0f, 1.0f
            };

            if (uniqueVertex.count(vertex) == 0) {
                uniqueVertex[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertex[vertex]);
        }
    }
}


void Render::generateMipmaps(vk::Image image, vk::Format format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }
    vk::CommandBuffer commandBuffer = beginCommands();
    vk::ImageMemoryBarrier barrier({}, {}, {}, {}, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
                                   {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    int mipWidth = texWidth;
    int mipHeight = texHeight;
    for (uint32_t i = 1;i < mipLevels;i ++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

        std::array<vk::Offset3D, 2> srcOffsets = {vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth, mipHeight, 1)};
        std::array<vk::Offset3D, 2> dstOffsets = {vk::Offset3D(0, 0, 0), vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1)};
        vk::ImageBlit blit({vk::ImageAspectFlagBits::eColor, i - 1, 0, 1}, srcOffsets,
                           {vk::ImageAspectFlagBits::eColor, i, 0, 1}, dstOffsets);
        commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
    endCommands(commandBuffer);
}

vk::SampleCountFlagBits Render::getMaxUsableSampleCount() {
    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();
    vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
    if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
    if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
    if (counts &vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
    if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
    if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }
    return vk::SampleCountFlagBits::e1;
}

void Render::createColorResources() {
    vk::Format colorFormat = swapChainImageFormat;
    std::tie(colorImage, colorImageMemory) = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaCount, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
}