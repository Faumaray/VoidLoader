#pragma once
#include <jvmti.h>
#include <jni.h>
#include <iostream>

class JvmtiHandler {
public:
    static void JNICALL vmInit(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread);
    static void JNICALL vmDeath(jvmtiEnv* jvmti, JNIEnv* jni);
    
    // Accessor methods for global variables
    static jvmtiEnv* getJvmti();
    static JavaVM* getJavaVM();
    static bool isAgentInitialized();
    
    // Setter methods
    static void setJvmti(jvmtiEnv* jvmti);
    static void setJavaVM(JavaVM* jvm);
    static void setAgentInitialized(bool initialized);

private:
    // No global variable declarations here - they'll be in agent.cpp
};