#include "jvmti_handler.h"

void JNICALL JvmtiHandler::vmInit(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread) {
    std::cout << "[FabricPP] VM Initialized - Minecraft starting soon" << std::endl;
    
    JvmtiHandler::setJvmti(jvmti);
    JvmtiHandler::setAgentInitialized(true);
    
    // The main application is now starting - our agent is ready
    std::cout << "[FabricPP] Agent ready for class transformation" << std::endl;
}

void JNICALL JvmtiHandler::vmDeath(jvmtiEnv* jvmti, JNIEnv* jni) {
    std::cout << "[FabricPP] VM Shutting down" << std::endl;
    JvmtiHandler::setAgentInitialized(false);
}