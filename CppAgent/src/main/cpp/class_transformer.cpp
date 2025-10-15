#include "class_transformer.h"
#include "jvmti_handler.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <set>

// Only target specific Minecraft classes, exclude everything else
static const std::set<std::string> TARGET_CLASSES = {
    "net/minecraft/client/Minecraft",
    "net/minecraft/client/main/Main", 
    "net/minecraft/server/MinecraftServer"
};

// Exclude all launcher and system classes
static const std::set<std::string> EXCLUDED_PREFIXES = {
    "java/", "javax/", "sun/", "jdk/", "com/sun/",
    "org/prismlauncher/", "org/gtm/", "com/google/", 
    "org/apache/", "org/slf4j/", "org/lwjgl/", "org/objectweb/",
    "io/", "okhttp3/", "okio/", "kotlin/", "scala/",
    "org/json/", "com/mojang/", "org/spongepowered/",
    "org/reflections/", "oshi/", "net/minecrell/", "org/checkerframework/"
};

void JNICALL ClassTransformer::classFileLoadHook(jvmtiEnv* jvmti, JNIEnv* jni,
                                                jclass class_being_redefined,
                                                jobject loader, const char* name,
                                                jobject protection_domain,
                                                jint class_data_len,
                                                const unsigned char* class_data,
                                                jint* new_class_data_len,
                                                unsigned char** new_class_data) {
    
    // Early return for null class name
    if (name == nullptr) {
        return;
    }
    
    std::string class_name(name);
    
    // Don't transform system classes or launcher classes
    for (const auto& excluded_prefix : EXCLUDED_PREFIXES) {
        if (class_name.find(excluded_prefix) == 0) {
            return;
        }
    }
    
    // Only transform specific Minecraft classes
    if (TARGET_CLASSES.find(class_name) != TARGET_CLASSES.end()) {
        std::cout << "[VoidLoader] Transforming class: " << class_name << std::endl;
        transformClass(jvmti, class_data, class_data_len, new_class_data, new_class_data_len);
    }
}

bool ClassTransformer::shouldTransform(const std::string& class_name) {
    return TARGET_CLASSES.find(class_name) != TARGET_CLASSES.end();
}

bool ClassTransformer::transformClass(jvmtiEnv* jvmti,
                                     const unsigned char* original_data,
                                     jint original_len,
                                     unsigned char** new_data,
                                     jint* new_len) {
    if (jvmti == nullptr || original_data == nullptr || original_len == 0) {
        return false;
    }
    
    // For now, just copy without modification to avoid issues
    unsigned char* copy = nullptr;
    jvmtiError error = jvmti->Allocate(original_len, &copy);
    
    if (error == JVMTI_ERROR_NONE && copy != nullptr) {
        std::memcpy(copy, original_data, original_len);
        *new_data = copy;
        *new_len = original_len;
        
        std::cout << "[VoidLoader] Successfully copied class (no transformation applied)" << std::endl;
        return true;
    }
    
    return false;
}