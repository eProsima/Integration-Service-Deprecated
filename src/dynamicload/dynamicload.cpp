#include <iostream>
#include "dynamicload.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

void* eProsimaLoadLibrary(const char *filename){
    void *libraryHandle = nullptr;

    if(filename != nullptr){
        #ifdef _WIN32
        libraryHandle = LoadLibrary(filename);
        #else
        libraryHandle = dlopen(filename, RTLD_LAZY);
        #endif
    }
    if(libraryHandle == nullptr){
        std::cout << "Load failed: " << dlerror() << std::endl;
    }

    return libraryHandle;
}

void* eProsimaGetProcAddress(void *libraryHandle, const char *functionName){
    void *functionPointer = nullptr;

    if(libraryHandle != nullptr && functionName != nullptr){
        #ifdef _WIN32
        functionPointer = GetProcAddress((HMODULE)libraryHandle, functionName);
        #else
        functionPointer = dlsym(libraryHandle, functionName);
        #endif
    }
    else{
        std::cout << "Bad parameters" << std::endl;
    }

    return functionPointer;
}

void eProsimaCloseLibrary(void *libraryHandle){
    #ifdef _WIN32
    //ToDo
    #else
    dlclose(libraryHandle);
    #endif
}
