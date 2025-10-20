#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>

// --- Constantes globales ---
const uint32_t WIDTH = 800;   // largeur de la fenêtre
const uint32_t HEIGHT = 600;  // hauteur de la fenêtre

// Liste des "validation layers" (couches de validation pour le debug Vulkan)
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Active ou non les layers selon le mode debug
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// ============================================================================
// Fonctions utilitaires pour la gestion du Debug Messenger
// ============================================================================

// Fonction d'extension non chargée par défaut : crée le Debug Messenger
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    // Récupère le pointeur vers la fonction d'extension
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    // Si la fonction existe, on l'appelle
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Fonction inverse : détruit le Debug Messenger
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// ============================================================================
// Structure pour identifier les familles de files (queues) supportées par le GPU
// ============================================================================
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily; // Index de la file supportant les commandes graphiques
    std::optional<uint32_t> presentFamily; //

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};


// ============================================================================
// Classe principale de l'application Vulkan
// ============================================================================
class HelloTriangleApplication {
public:
    void run() {
        initWindow();   // Crée la fenêtre GLFW
        initVulkan();   // Initialise Vulkan (instance, device, etc.)
        mainLoop();     // Boucle principale
        cleanup();      // Nettoyage à la fin
    }

private:
    // --- Variables principales ---
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // GPU physique
    VkDevice device;                                  // Device logique
    VkQueue graphicsQueue;                            // File de commandes graphiques
    VkSurfaceKHR surface;                             // creer

    // =========================================================================
    // 1. Création de la fenêtre avec GLFW
    // =========================================================================
    void initWindow() {
        glfwInit(); // Initialise GLFW
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // On n'utilise pas OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // Fenêtre non redimensionnable
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    // =========================================================================
    // 2. Initialisation de Vulkan
    // =========================================================================
    void initVulkan() {
        createInstance();        // Crée l'instance Vulkan (première étape obligatoire)
        setupDebugMessenger();   // Active la gestion du debug (si mode debug)
        createSurface();
        pickPhysicalDevice();    // Sélectionne un GPU compatible
        createLogicalDevice();   // Crée le device logique (communication avec le GPU)
    }

    // =========================================================================
    // 3. Boucle principale (événements + rendu futur)
    // =========================================================================
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents(); // Traite les événements (clavier, souris, etc.)
        }
    }

    // =========================================================================
    // 4. Nettoyage à la fin
    // =========================================================================
    void cleanup() {
        vkDestroyDevice(device, nullptr); // Détruit le device logique

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr); // Détruit l'instance Vulkan
        glfwDestroyWindow(window);           // Ferme la fenêtre
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwTerminate();                      // Termine GLFW
    }

    // =========================================================================
    // Création de l’instance Vulkan (le cœur de toute appli Vulkan)
    // =========================================================================
    void createInstance() {
        // Vérifie la présence des validation layers
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // Infos sur l’application (optionnelles mais bonnes pratiques)
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";  // Nom de l’app
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";            // Pas de moteur
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;      // Version Vulkan ciblée

        // Infos pour créer l’instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Extensions requises par GLFW
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Configuration du debug (si activé)
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // Création de l’instance Vulkan
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create Vulkan instance!");
        }
    }

    // =========================================================================
    // Configuration du Debug Messenger
    // =========================================================================
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |  // Messages détaillés
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |  // Avertissements
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;     // Erreurs
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback; // Pointeur vers la fonction de callback
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    // =========================================================================
    // Sélection d’un GPU physique compatible avec Vulkan
    // =========================================================================
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("no GPU with Vulkan support found!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // On choisit le premier GPU compatible
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // =========================================================================
    // Création du Device logique (interface entre ton app et le GPU)
    // =========================================================================
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // Configuration de la file graphique
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // Aucune fonctionnalité spéciale demandée
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Création du device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // Création effective du device logique
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // Récupération de la file graphique du GPU
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    }

    // =========================================================================
    // Vérifie si un GPU est compatible
    // =========================================================================
    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        return indices.isComplete();
    }

    // =========================================================================
    // Recherche d'une file graphique disponible sur le GPU
    // =========================================================================
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i; // On note l’indice de la file graphique
                break;
            }
            i++;
        }

        return indices;
    }

    // =========================================================================
    // Extensions nécessaires (GLFW + Debug)
    // =========================================================================
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    // =========================================================================
    // Vérifie la disponibilité des validation layers
    // =========================================================================
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) return false;
        }
        return true;
    }

    // =========================================================================
    // Fonction de callback pour afficher les messages du Debug Layer
    // =========================================================================
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
    void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("échec de la création de la window surface!");
    }
}
};


int main() {
    HelloTriangleApplication app;

    try {
        app.run(); // Lance l’application
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
