//
// Created by amemiya7 on 2024/4/30.
//

#include "Render.h"
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
#include <cassert>
#include <filesystem>

static void frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    app->setFrameBufferResized(true);
}

Render::Render(GLFWkeyfun key_callback) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    SCREEN_WIDTH = 1600;
    SCREEN_HEIGHT = 900;
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tetris", nullptr, nullptr);
    glfwSetWindowSizeCallback(window, frameBufferResizedCallback);
    glfwSetKeyCallback(window, key_callback);
    initVulkan();
}
void Render::resize(const unsigned int Screen_Width, const unsigned int Screen_Height) {
    SCREEN_WIDTH = Screen_Width;
    SCREEN_HEIGHT = Screen_Height;
    glfwSetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    recreateSwapChain();
}
Render::Render(const unsigned int Screen_Width, const unsigned int Screen_Height,  GLFWkeyfun key_callback) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//    glfwGetPrimaryMonitor()
    SCREEN_WIDTH = Screen_Width;
    SCREEN_HEIGHT  = Screen_Height;
    window = glfwCreateWindow(Screen_Width, Screen_Height, "Tetris", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, frameBufferResizedCallback);
    glfwSetKeyCallback(window, key_callback);

    initVulkan();
}

Render::~Render() {
    cleanup();
}

void Render::render(std::vector<Vertex> vertices_, std::vector<uint32_t> indices_){
    vertices = vertices_;
    indices = indices_;
    createVertexBuffers();
    createIndexBuffer();
    glfwPollEvents();
    drawFrame();
}

void Render::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

void Render::cleanup() {
    device.waitIdle();
    cleanupSwapChain();
    device.destroy(indexBuffer);
    device.free(indexBufferMemory);
    device.destroy(vertexBuffer);
    device.free(vertexBufferMemory);
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
    instance.destroy(surface);
    instance.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Render::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation_layers_requested_but_not_available");
    }
    vk::ApplicationInfo appInfo("Tetris", {}, "No_Engine", {}, vk::ApiVersion13);

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
    if (enableValidationLayers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
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
    vk::DeviceCreateInfo createInfo({}, queueCreateInfos, {}, deviceExtensions);

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
        return {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eR8G8B8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
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
        vk::ImageViewCreateInfo createInfo({}, swapChainImages[i], vk::ImageViewType::e2D, swapChainImageFormat,
                                           {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity},
                                           {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        if (device.createImageView(&createInfo, nullptr, &swapChainImageViews[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("failed_to_create_image_views");
        }
    }
}

void Render::createGraphicsPipeline() {
    const std::filesystem::path shaderPath = resourcesPath / "shaders";
    auto vertShaderCode = readFile(shaderPath / "vert.spv");
    auto fragShaderCode = readFile(shaderPath / "frag.spv");
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

    vk::PipelineRasterizationStateCreateInfo rasterizer({}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
                                                        vk::False, {}, {}, {}, 1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1, false, {}, {}, {}, {});

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

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 0, nullptr, 0, nullptr);

    if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_pipeline_layout");
    }


    vk::GraphicsPipelineCreateInfo pipelineInfo({}, 2, shaderStages, &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer,
                                                &multisampling, nullptr, &colorBlending, &dynamicState, pipelineLayout, renderPass, 0, nullptr,0);


    if (device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_graphics_pipeline");
    }

    device.destroy(vertShaderModule, nullptr);
    device.destroy(fragShaderModule, nullptr);
}

std::vector<uint32_t> Render::readFile(const std::filesystem::path &filename) {
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

vk::ShaderModule Render::createShaderModule(const std::vector<uint32_t>& code) {
//    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    vk::ShaderModuleCreateInfo createInfo({}, code.size(), code.data());
    vk::ShaderModule shaderModule;
    if (device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create shader module");
    }
    return shaderModule;
}

void Render::createRenderPass() {
    vk::AttachmentDescription colorAttachment({}, swapChainImageFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
                                              vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &colorAttachmentRef);
    vk::SubpassDependency dependency(vk::SubpassExternal, 0,
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                     vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);
    vk::RenderPassCreateInfo renderPassInfo({}, 1, &colorAttachment, 1, &subpass,
                                            1, &dependency);


    if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
        throw std::runtime_error("failed_to_create_render_pass!");
    }
}

void Render::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vk::ImageView attachments[] = {swapChainImageViews[i]};
        vk::FramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.setRenderPass(renderPass)
                .setAttachmentCount(1)
                .setPAttachments(attachments)
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
    vk::RenderPassBeginInfo renderPassInfo{renderPass, swapChainFramebuffers[imageIndex], {{0, 0}, swapChainExtent}};
    vk::ClearValue clearColor({0.0f, 0.0f, 0.0f, 1.0f});
    renderPassInfo.setClearValueCount(1)
            .setPClearValues(&clearColor);

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({0, 0}, swapChainExtent);
    commandBuffer.setScissor(0, 1, &scissor);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    vk::DeviceSize  offset{0};
    commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer, &offset);
    commandBuffer.bindIndexBuffer(indexBuffer, offset, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void Render::drawFrame() {
    auto res = device.waitForFences(1, &inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
    assert(res == vk::Result::eSuccess);
    auto [Result, imageIndex] = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], {});
    if (Result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    } else if (Result != vk::Result::eSuccess && Result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to require swap chain image!");
    }


    res = device.resetFences(1, &inFlightFences[currentFrame]);
    assert(res == vk::Result::eSuccess);
    commandBuffers[currentFrame].reset();
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

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
        auto res = device.createSemaphore(&semaphoreCreateInfo, {}, &imageAvailableSemaphores[i]);
        assert(res == vk::Result::eSuccess);
        res = device.createSemaphore(&semaphoreCreateInfo, {}, &renderFinishedSemaphores[i]);
        assert(res == vk::Result::eSuccess);
        res = device.createFence(&fenceCreateInfo, {}, &inFlightFences[i]);
        assert(res == vk::Result::eSuccess);
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
    createFramebuffers();
}

void Render::cleanupSwapChain() {
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
    static vk::DeviceSize size;
    if (!firstDrawVertex) {
        firstDrawVertex = true;
        vk::BufferCreateInfo createInfo({}, sizeof(vertices[0]) * vertices.size(),
                                        vk::BufferUsageFlagBits::eVertexBuffer,
                                        vk::SharingMode::eExclusive);
        size = createInfo.size;
        vertexBuffer = device.createBuffer(createInfo);
        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(vertexBuffer);

        vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size,
                                            findMemoryType(memoryRequirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                                           vk::MemoryPropertyFlagBits::eHostCoherent));
        vertexBufferMemory = device.allocateMemory(allocateInfo);
        device.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);
    }
    void *data =  device.mapMemory(vertexBufferMemory, 0, size, {});
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    device.unmapMemory(vertexBufferMemory);

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

bool Render::shouldStop() {
    return glfwWindowShouldClose(window);
}

glm::vec2 Render::worldToScreen(glm::vec2 world) {
    return glm::vec2((1.0f * world.x / SCREEN_WIDTH - 0.5) * 2, ((1.0f - 1.0f * world.y / SCREEN_HEIGHT) - 0.5)  * 2);
}



void Render::inputCube(VertexVector& vertices, IndexVector& indices, glm::vec2 leftCorner_, float width, float height) {
    //clockwise
    glm::vec2 leftBottom = worldToScreen(leftCorner_);
    glm::vec2 leftTop = worldToScreen({leftCorner_.x, leftCorner_.y + height});
    glm::vec2 rightBottom = worldToScreen({leftCorner_.x + width, leftCorner_.y});
    glm::vec2 rightTop = worldToScreen({leftCorner_.x + width, leftCorner_.y + height});
    auto output = [&](glm::vec2 pos) {
        //
#ifndef NDEBUG
        std::cout << pos.x << ' ' << pos.y << std::endl;
#endif
    };
    output(leftBottom);
    output(leftTop);
    output(rightBottom);
    output(rightTop);

    //-4
    vertices.push_back({leftBottom, glm::vec3(1.0f, 1.0f, 1.0f)});
    //-3
    vertices.push_back({leftTop, glm::vec3(1.0f, 1.0f, 1.0f)});
    //-2
    vertices.push_back({rightBottom, glm::vec3(1.0f, 1.0f, 1.0f)});

    //-2
//    vertices.push_back({rightBottom, glm::vec3(1.0f, 1.0f, 1.0f)});
    //-3
//    vertices.push_back({leftTop, glm::vec3(1.0f, 1.0f, 1.0f)});
    //-1
    vertices.push_back({rightTop, glm::vec3(1.0f, 1.0f, 1.0f)});

    indices.push_back(vertices.index - 4);
    indices.push_back(vertices.index - 3);
    indices.push_back(vertices.index - 2);
    indices.push_back(vertices.index - 2);
    indices.push_back(vertices.index - 3);
    indices.push_back(vertices.index - 1);

}


void Render::createIndexBuffer() {
    static vk::DeviceSize size;
    if (!firstDrawIndex) {
        firstDrawIndex = true;
//        std::cout << "indices size" << indices.size() << std::endl;
        vk::BufferCreateInfo createInfo({}, sizeof(indices[0]) * indices.size(),
                                        vk::BufferUsageFlagBits::eIndexBuffer,
                                        vk::SharingMode::eExclusive);
        size = createInfo.size;
        indexBuffer = device.createBuffer(createInfo);
        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(indexBuffer);
        vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size,
                                            findMemoryType(memoryRequirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                                           vk::MemoryPropertyFlagBits::eHostCoherent));
        indexBufferMemory = device.allocateMemory(allocateInfo);
        device.bindBufferMemory(indexBuffer, indexBufferMemory, 0);
    }
//    std::cout << size << std::endl;
    void *data = device.mapMemory(indexBufferMemory, 0, size, {});
    memcpy(data, indices.data(), size);
    device.unmapMemory(indexBufferMemory);
}
