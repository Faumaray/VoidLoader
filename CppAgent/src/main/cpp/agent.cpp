#include <jvmti.h>
#include <jni.h>
#include <iostream>
#include <vector>
#include <string>
#include "jvmti_handler.h"
#include "class_transformer.h"
#include "mod_manager.h"

// Global variables defined ONLY here
namespace {
    jvmtiEnv* g_jvmti = nullptr;
    JavaVM* g_jvm = nullptr;
    bool g_agent_initialized = false;
    ModManager g_modManager;
}

// Implementation of JvmtiHandler accessors
jvmtiEnv* JvmtiHandler::getJvmti() {
    return g_jvmti;
}

JavaVM* JvmtiHandler::getJavaVM() {
    return g_jvm;
}

bool JvmtiHandler::isAgentInitialized() {
    return g_agent_initialized;
}

void JvmtiHandler::setJvmti(jvmtiEnv* jvmti) {
    g_jvmti = jvmti;
}

void JvmtiHandler::setJavaVM(JavaVM* jvm) {
    g_jvm = jvm;
}

void JvmtiHandler::setAgentInitialized(bool initialized) {
    g_agent_initialized = initialized;
}

// JVMTI Agent Entry Point
extern "C" JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved) {
    std::cout << "[FabricPP] C++ Agent_OnLoad called!" << std::endl;
    
    JvmtiHandler::setJavaVM(vm);
    
    // Get JVMTI environment
    jint result = vm->GetEnv(reinterpret_cast<void**>(&g_jvmti), JVMTI_VERSION_1_2);
    if (result != JNI_OK || g_jvmti == nullptr) {
        std::cerr << "[FabricPP] ERROR: Unable to access JVMTI!" << std::endl;
        return JNI_ERR;
    }
    
    JvmtiHandler::setJvmti(g_jvmti);
    
    // Set JVMTI capabilities
    jvmtiCapabilities capabilities = {0};
    capabilities.can_retransform_classes = 1;
    capabilities.can_retransform_any_class = 1;
    capabilities.can_generate_all_class_hook_events = 1;
    capabilities.can_get_bytecodes = 1;
    
    jvmtiError error = g_jvmti->AddCapabilities(&capabilities);
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[FabricPP] ERROR: Unable to set JVMTI capabilities!" << std::endl;
        return JNI_ERR;
    }
    
    // Set event callbacks
    jvmtiEventCallbacks callbacks = {0};
    callbacks.ClassFileLoadHook = &ClassTransformer::classFileLoadHook;
    callbacks.VMInit = &JvmtiHandler::vmInit;
    callbacks.VMDeath = &JvmtiHandler::vmDeath;
    
    error = g_jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    if (error != JVMTI_ERROR_NONE) {
        std::cerr << "[FabricPP] ERROR: Unable to set JVMTI callbacks!" << std::endl;
        return JNI_ERR;
    }
    
    // Enable events
    g_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    g_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, nullptr);
    g_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, nullptr);
    
    std::cout << "[FabricPP] C++ Agent loaded successfully!" << std::endl;
    return JNI_OK;
}

// Called from Java agent - NON-BLOCKING
extern "C" JNIEXPORT void JNICALL Java_org_faumaray_FabricPPAgent_nativeAgentStart(JNIEnv* env, jclass clazz) {
    std::cout << "[FabricPP] Native agent started from Java" << std::endl;
    // Don't do any heavy work here - just initialization
}

extern "C" JNIEXPORT void JNICALL Java_org_faumaray_FabricPPAgent_nativeAgentInit(JNIEnv* env, jclass clazz, jstring args, jobject instrumentation) {
    std::cout << "[FabricPP] Native agent initialized with Instrumentation" << std::endl;
    // Initialize mod manager - but don't block
    g_modManager.discoverMods("./mods");
    JavaVM* vm;
    auto error = env->GetJavaVM(&vm);
    if (error != JNI_OK) {
        std::cerr << "[FabricPP] FATAL: Unable to get JavaVM from JNIEnv!" << std::endl;
        return;
    }
    Agent_OnLoad(vm, nullptr, nullptr);
}

// Cleanup native resources
extern "C" JNIEXPORT void JNICALL Java_org_faumaray_FabricPPAgent_nativeCleanup(JNIEnv* env, jclass clazz) {
    std::cout << "[FabricPP] Native cleanup called" << std::endl;
    // Cleanup any native resources if needed
}