#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>

//gerer tout les platforme possible mac , linux(wayland,x11),mac 
#include <vulkan/vulkan.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)


#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>

#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>







const uint32_t l = 800;
const uint32_t h = 600;
//attive les verification  si je lace en mode debug
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif
class HelloTriangleApplication {

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    /*
    create debug est une fonction d'une extension de vulkan et elle permet de faire reference a
    la fonction si elle est pas present sa envoie une erreur
    
    */
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    /*
    detruire le message aprés
    evite les fuite de memoirre
    
    */
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

public:
   
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    };

    
private:
    VkInstance instance;
    GLFWwindow* window;

    void initVulkan() {

            createInstance();
            uint32_t extensionCount = 0;
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
            std::cout << "Extensions disponibles :\n";
            for (const auto& extension : extensions) {
                std::cout << '\t' << extension.extensionName << '\n';
            }

    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    void initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//precice de ne pas creer de contex opengl
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//c'est plus complexe donc pas on doit le faire nous meme
        window = glfwCreateWindow(l, h, "Vulkan", nullptr, nullptr);

    }
    void createInstance() {
        /*
        permet initialiser vulkan
        et lui donne des info utile pour dagnostiquer 
        ou optimiser son compotement

        */
       if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};// purement descriptif mais obligatoir
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};//obligatoire et donne des infoo utile pour la compilation
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;



        uint32_t glfwExtensionCount = 0; //nombre extention nécessaire
        const char** glfwExtensions; //liste des extention

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        //ajoute a info
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        createInfo.enabledLayerCount = 0; //qi tu veux attraper les erreur vulkan et la c'est 0 donc c'est désative
        // cree l'instance finale avec tout les info
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);



        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Echec de la création de l'instance!");
}
        }



    bool checkValidationLayerSupport() {
        /*
        chek les layer disponible 
        les layer est une couche intermediaire permettznt attraper lmes erreur
        */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);//ENuMERE les layer dispo

        std::vector<VkLayerProperties> availableLayers(layerCount);//sert a recupere les info des layer present
        vkEnumerateInstanceLayerProperties(//
            &layerCount, 
            availableLayers.data()
        );


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
};




int main() {
    HelloTriangleApplication app;
    

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}