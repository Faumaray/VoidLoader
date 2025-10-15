#include "mod_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>

ModManager::ModManager() {
    std::cout << "[VoidLoader] ModManager initialized" << std::endl;
}

ModManager::~ModManager() {
    std::cout << "[VoidLoader] ModManager destroyed" << std::endl;
}

// Discover mods in the mods directory
void ModManager::discoverMods(const std::string& mods_dir) {
    namespace fs = std::filesystem;
    
    std::cout << "[VoidLoader] Scanning for mods in: " << mods_dir << std::endl;
    
    try {
        // Create mods directory if it doesn't exist
        if (!fs::exists(mods_dir)) {
            fs::create_directories(mods_dir);
            std::cout << "[VoidLoader] Created mods directory: " << mods_dir << std::endl;
            
            // Create a README
            std::ofstream readme(mods_dir + "/README.txt");
            readme << "VoidLoader Mods Directory\n";
            readme << "=======================\n\n";
            readme << "Place your .jar mod files here.\n";
            readme << "The mod loader will automatically detect and load them.\n";
            readme.close();
            
            return;
        }
        
        // Scan for mod files
        int mod_count = 0;
        for (const auto& entry : fs::directory_iterator(mods_dir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                
                // Support .jar mods (Java mods) and .dll/.so (native mods)
                if (ext == ".jar" || ext == ".dll" || ext == ".so") {
                    loadMod(entry.path());
                    mod_count++;
                }
            }
        }
        
        std::cout << "[VoidLoader] Discovered " << mod_count << " mod file(s)" << std::endl;
        std::cout << "[VoidLoader] Loaded " << loaded_mods.size() << " mod(s) successfully" << std::endl;
        
        // Print loaded mods
        if (!loaded_mods.empty()) {
            std::cout << "\n[VoidLoader] Loaded Mods:" << std::endl;
            for (const auto& mod : loaded_mods) {
                std::cout << "  - " << mod.name << " v" << mod.version 
                          << " (" << mod.file_path << ")" << std::endl;
            }
            std::cout << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[VoidLoader] Error discovering mods: " << e.what() << std::endl;
    }
}

// Load a single mod
void ModManager::loadMod(const std::filesystem::path& mod_path) {
    std::string filename = mod_path.filename().string();
    std::cout << "[VoidLoader] Loading mod: " << filename << std::endl;
    
    try {
        ModInfo mod;
        mod.file_path = mod_path.string();
        mod.name = filename;
        mod.version = "unknown";
        mod.enabled = true;
        
        // Parse mod metadata based on file type
        if (mod_path.extension() == ".jar") {
            parseJarMod(mod_path, mod);
        } else {
            // Native mod (.dll/.so)
            parseNativeMod(mod_path, mod);
        }
        
        loaded_mods.push_back(mod);
        std::cout << "[VoidLoader] Loaded: " << mod.name << " v" << mod.version << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[VoidLoader] Failed to load mod " << filename << ": " << e.what() << std::endl;
    }
}

// Parse JAR mod metadata (fabric.mod.json or similar)
void ModManager::parseJarMod(const std::filesystem::path& jar_path, ModInfo& mod) {
    // TODO: Implement JAR parsing
    // This would:
    // 1. Open the JAR file as a ZIP
    // 2. Extract fabric.mod.json or mod.json
    // 3. Parse JSON for mod metadata
    // 4. Extract transformation rules/mixins
    
    // For now, extract basic info from filename
    std::string filename = jar_path.stem().string();
    
    // Try to parse version from filename (e.g., "modname-1.0.0.jar")
    size_t dash_pos = filename.find_last_of('-');
    if (dash_pos != std::string::npos) {
        mod.name = filename.substr(0, dash_pos);
        mod.version = filename.substr(dash_pos + 1);
    } else {
        mod.name = filename;
    }
    
    mod.type = ModType::JAR;
    
    std::cout << "[VoidLoader]   Parsed JAR mod: " << mod.name << " v" << mod.version << std::endl;
}

// Parse native mod metadata
void ModManager::parseNativeMod(const std::filesystem::path& native_path, ModInfo& mod) {
    mod.name = native_path.stem().string();
    mod.type = ModType::NATIVE;
    
    // TODO: Load the native library and query for metadata
    // This would dlopen/LoadLibrary the .dll/.so and call an exported function
    // like "getModInfo()" to retrieve metadata
    
    std::cout << "[VoidLoader]   Parsed native mod: " << mod.name << std::endl;
}

// Apply transformations from mods to a class
void ModManager::applyTransformations(
    const std::string& class_name,
    const unsigned char* class_data,
    jint class_data_len) {
    
    // Check each mod for transformations targeting this class
    for (const auto& mod : loaded_mods) {
        if (!mod.enabled) continue;
        
        // TODO: Check if mod has transformations for this class
        // This would:
        // 1. Look up transformation rules from mod metadata
        // 2. Check if any mixins target this class
        // 3. Queue transformations to be applied
        
        // For now, just log
        if (class_name.find("net/minecraft/client/Minecraft") == 0) {
            std::cout << "[VoidLoader]   Mod '" << mod.name << "' checking transformations for: " 
                      << class_name << std::endl;
        }
    }
}

// Get list of loaded mods
const std::vector<ModInfo>& ModManager::getLoadedMods() const {
    return loaded_mods;
}

// Enable/disable a mod
void ModManager::setModEnabled(const std::string& mod_name, bool enabled) {
    for (auto& mod : loaded_mods) {
        if (mod.name == mod_name) {
            mod.enabled = enabled;
            std::cout << "[VoidLoader] Mod '" << mod_name << "' " 
                      << (enabled ? "enabled" : "disabled") << std::endl;
            return;
        }
    }
}