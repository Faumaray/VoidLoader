#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <jvmti.h>

enum class ModType {
    JAR,      // Java/bytecode mod
    NATIVE    // Native C++ mod
};

struct ModInfo {
    std::string name;
    std::string version;
    std::string file_path;
    ModType type;
    bool enabled;
    
    // Mod metadata (from fabric.mod.json or similar)
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;
};

class ModManager {
public:
    ModManager();
    ~ModManager();
    
    // Discover and load mods from directory
    void discoverMods(const std::string& mods_dir);
    
    // Load a single mod
    void loadMod(const std::filesystem::path& mod_path);
    
    // Apply transformations from all mods to a class
    void applyTransformations(
        const std::string& class_name,
        const unsigned char* class_data,
        jint class_data_len
    );
    
    // Get list of loaded mods
    const std::vector<ModInfo>& getLoadedMods() const;
    
    // Enable/disable a mod
    void setModEnabled(const std::string& mod_name, bool enabled);
    
private:
    std::vector<ModInfo> loaded_mods;
    
    // Parse mod metadata
    void parseJarMod(const std::filesystem::path& jar_path, ModInfo& mod);
    void parseNativeMod(const std::filesystem::path& native_path, ModInfo& mod);
};