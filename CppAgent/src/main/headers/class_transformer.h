#pragma once
#include <jvmti.h>
#include <string>

class ClassTransformer {
public:
    static void JNICALL classFileLoadHook(jvmtiEnv* jvmti, JNIEnv* jni,
                                         jclass class_being_redefined,
                                         jobject loader, const char* name,
                                         jobject protection_domain,
                                         jint class_data_len,
                                         const unsigned char* class_data,
                                         jint* new_class_data_len,
                                         unsigned char** new_class_data);
    
private:
    static bool shouldTransform(const std::string& class_name);
    static bool transformClass(jvmtiEnv* jvmti, 
                              const unsigned char* original_data,
                              jint original_len,
                              unsigned char** new_data,
                              jint* new_len);
};