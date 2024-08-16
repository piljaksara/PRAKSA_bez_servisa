#include <jni.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <malloc.h>
#include <sys/resource.h>
#include <android/log.h>
#include "ResourceUsage.cpp"
#include <thread>
#include <atomic>
#define LOG_TAG "MyApp"
extern "C" ResourceUsage* Sara();


// Globalni pokazivač na JavaVM
JavaVM* g_jvm = nullptr;
extern "C" {
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT jstring JNICALL
Java_com_example_prekopiranceokod_MainActivity_callNativeFunction(JNIEnv *env, jobject thiz) {
    ResourceUsage* usage = Sara();
    std::string resultStr = "CPU Usage: " + std::to_string(usage->getCpuUsage()) +
                            "%, Memory Usage: " + std::to_string(usage->getMemoryUsage()) + " KB";
    return env->NewStringUTF(resultStr.c_str());
}

JNIEXPORT void JNICALL
Java_com_example_prekopiranceokod_MainActivity_startMonitoring(JNIEnv *env, jobject thiz) {
    extern void start_monitoring();
    start_monitoring();
}

JNIEXPORT void JNICALL
Java_com_example_prekopiranceokod_MainActivity_stopMonitoring(JNIEnv *env, jobject thiz) {
    extern void stop_monitoring();
    stop_monitoring();
}
}

extern "C" {
jstring Java_com_example_prekopiranceokod_MainActivity_getResourceUsage(JNIEnv* env, jobject thiz);
}


std::atomic<bool> is_monitoring(false);
std::thread monitoring_thread;


// Funkcija za periodičnu proveru resursa
void periodic_resource_usage() {
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    while (is_monitoring) {
        jobject thiz = nullptr;  // Ako je potreban pravi objekat, prosledi ga

        jstring result = Java_com_example_prekopiranceokod_MainActivity_getResourceUsage(env, thiz);
        const char* result_str = env->GetStringUTFChars(result, nullptr);

        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Resource Usage: %s", result_str);

        env->ReleaseStringUTFChars(result, result_str);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    g_jvm->DetachCurrentThread();
}


// Funkcija za startovanje periodičnog ispisivanja
void start_monitoring() {
    if (!is_monitoring) {
        is_monitoring = true;
        monitoring_thread = std::thread(periodic_resource_usage);
    }
}

// Funkcija za zaustavljanje periodičnog ispisivanja
void stop_monitoring() {
    if (is_monitoring) {
        is_monitoring = false;
        if (monitoring_thread.joinable()) {
            monitoring_thread.join();
        }
    }
}
/*
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_prekopiranceokod_MainActivity_callNativeFunction2(JNIEnv *env, jobject thiz) {
    ResourceUsage* usage = Sara();
}*/