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


extern "C" ResourceUsage* Sara();


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_prekopiranceokod_MainActivity_callNativeFunction(JNIEnv *env, jobject thiz) {
    // Pozovi Sara funkciju koja vraća ResourceUsage
    ResourceUsage* usage = Sara();

    // Kreiraj string za ispis rezultata
    std::string resultStr = "CPU Usage: " + std::to_string(usage->getCpuUsage()) +
                            "%, Memory Usage: " + std::to_string(usage->getMemoryUsage()) + " KB";

    // Pretvori std::string u jstring
    return env->NewStringUTF(resultStr.c_str());
}

extern "C" {
jstring Java_com_example_prekopiranceokod_MainActivity_getResourceUsage(JNIEnv* env, jobject thiz);
}

