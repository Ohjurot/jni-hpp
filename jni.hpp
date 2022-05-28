/*
*   jni.hpp - C++ Helpers for amazing java embedding
*   Requires jni.hpp.jar to be present in the java classpath!
* 
*	MIT License
*
*	Copyright (c) 2021 Ludwig Fuechsl
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/
#pragma once

#include <jni.h>

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <array>
#include <filesystem>

#ifndef JNIHPP_OPTIONS_MAX 
#define JNIHPP_OPTIONS_MAX 32
#endif

#define __JNIHPP_JNI_LOAD_AND_CHECK(val, expr) {val = expr; if(!val) return false; }
#define __JNIHPP_JNI_CHECK_RESULT(expr) {if(expr != JNI_OK) { return false; }}

// Put all in the jni namespace
namespace jni
{
    // VM Startup arguments
    class VMArguments
    {
        public:
            // Default setting
            VMArguments()
            {
                JvmArgs.version = 0;
                JvmArgs.options = &Options[0];
                JvmArgs.nOptions = 0;
                JvmArgs.ignoreUnrecognized = false;
            }

            // Set the JNI version
            void SetJNIVersion(jint version) noexcept
            {
                JvmArgs.version = version;
            }
            // Set the IgnoreUnrecognized flag
            void IgnoreUnrecognized(bool ignore) noexcept
            {
                JvmArgs.ignoreUnrecognized = ignore;
            }

            // Searches a folder for all jar files and addes them to the classpath
            void AddClasspathJarFolder(std::filesystem::path PathToFolder)
            {
                // For all files
                for (
                    auto it = std::filesystem::directory_iterator(PathToFolder);
                    it != std::filesystem::directory_iterator();
                    ++it
                    )
                {
                    // Check extension
                    if (it->path().extension() == std::string(".jar"))
                    {
                        AddClasspathJar(it->path());
                    }
                }
            }

            // Adds a SINGLE jar to the classpath
            void AddClasspathJar(std::filesystem::path PathToJar)
            {
                JarFiles.push_back(PathToJar.generic_string());
            }

            // Add option helpers
            bool AddOption(const std::string& option)
            {
                return AddOption(option.c_str(), nullptr);
            }
            bool AddOption(const std::string& option, std::string& extraInfo)
            {
                return AddOption(option.c_str(), extraInfo.c_str());
            }

            // Add option
            bool AddOption(const char* option, const char* extraInfo = nullptr)
            {
                // Check if adding is possible
                bool CanAdd = AvailibleOptions;
                if (CanAdd)
                {
                    // Store string
                    if (option)
                    {
                        StringBuffer.push_back(option);
                        Options[JNIHPP_OPTIONS_MAX - AvailibleOptions].optionString = &StringBuffer[StringBuffer.size() - 1][0];
                    }
                    else
                    {
                        Options[JNIHPP_OPTIONS_MAX - AvailibleOptions].optionString = nullptr;
                    }
                    if (extraInfo)
                    {
                        StringBuffer.push_back(extraInfo);
                        Options[JNIHPP_OPTIONS_MAX - AvailibleOptions].extraInfo = &StringBuffer[StringBuffer.size() - 1][0];
                    }
                    else
                    {
                        Options[JNIHPP_OPTIONS_MAX - AvailibleOptions].extraInfo = nullptr;
                    }
                    // One options less
                    AvailibleOptions--;
                }

                return CanAdd;
            }

            // Retrive reference to args
            const JavaVMInitArgs& Get() noexcept
            {
                // Adopt class path
                std::string ClassPath = "-Djava.class.path=";
                for (auto it = JarFiles.begin(); it != JarFiles.end(); it++)
                {
                    ClassPath += *it + ";";
                }
                AddOption(ClassPath);

                // Evaluate options count
                JvmArgs.nOptions = JNIHPP_OPTIONS_MAX - AvailibleOptions;

                // Return args
                return JvmArgs;
            }
        private:
            // Java VM Args
            JavaVMInitArgs JvmArgs;

            // Classpath
            std::vector<std::string> JarFiles;

            // Buffering vector
            std::vector<std::string> StringBuffer;

            // Java options
            std::array<JavaVMOption, JNIHPP_OPTIONS_MAX> Options;
            size_t AvailibleOptions = JNIHPP_OPTIONS_MAX;
    };

    // Relevant java class, function, field, ... table
    struct JAVA_Table
    {
        jclass JAVA_Cls_NativClassLoadResult = nullptr;
        jfieldID JAVA_Fld_NativClassLoadResult_Loaded = nullptr;
        jfieldID JAVA_Fld_NativClassLoadResult_Data = nullptr;

        jclass JAVA_Cls_ClassLoader = nullptr;
        jfieldID JAVA_Fld_ClassLoader_InstanceId = nullptr;

        jclass JAVA_Cls_Setup = nullptr;
        jmethodID JAVA_Mth_Setup_RunSetup = nullptr;

        jclass JAVA_Cls_NativePrintStream = nullptr;

        jclass JAVA_Cls_NativeInputStream = nullptr;
    };

    // Class that interfaces with JNI (override this class)
    class JavaVM
    {
        public:
            // Construct
            JavaVM() noexcept = default;
            JavaVM(const JavaVM&) noexcept = delete;
            JavaVM(JavaVM&& Other) noexcept
            {
                // Do a assign operation
                *this = std::move(Other);
            }

            // Assign
            JavaVM& operator=(const JavaVM&) noexcept = delete;
            JavaVM& operator=(JavaVM&& Other) noexcept
            {
                // Prevent self move
                if (&Other != this)
                {
                    // Move and unset critical
                    JVM = Other.JVM;
                    JNIEnv = Other.JNIEnv;
                    Other.JVM = nullptr;
                    Other.JNIEnv = nullptr;

                    // Copy jvt
                    jvt = Other.jvt;
                }
            }

            // Virtual destruct
            virtual ~JavaVM()
            {
                StopVM();
            }

            ::JNIEnv* operator->() noexcept
            {
                return JNIEnv;
            }

        public:
            // Starts the Java VM (Returns true on success)
            bool StartVM()
            {
                bool JvmOk = false;
                
                if (!JVM)
                {
                    // Create a new JVM Argument object and run custom setup
                    VMArguments VmArgs;
                    SetupVM_Internal(VmArgs);
                    SetupVM(VmArgs);

                    // Create a java vm
                    if (JNI_CreateJavaVM(&JVM, (void**)&JNIEnv, (void*)&VmArgs.Get()) == JNI_OK)
                    {
                        // Load refs and setup natives
                        if (LoadAllJavaRefs() && RegisterNatives() && SetupClassStates())
                        {
                            JvmOk = true;
                        }
                        else
                        {
                            // Direct stop
                            StopVM();
                        }
                    }
                }

                return JvmOk;
            }

            // Stops the Java VM
            void StopVM()
            {
                // Destroy and unset if possible
                if (JVM)
                {
                    JVM->DestroyJavaVM();
                    JVM = nullptr;
                    JNIEnv = nullptr;
                }
            }

            // Get the current JVM state
            bool IsRunning()
            {
                return JVM != nullptr;
            }

        protected:
            // Virtual function called on vm start
            virtual void SetupVM(jni::VMArguments& Args) = 0;
            // Virtual function that is called when the class cannot be loaded by java
            // If ClassName is loaded by c++ code: Allocate memory for pointer "*ClassContent" and return size of allocated buffer (aka class content length)
            virtual jsize LoadClass(const std::string& ClassName, char** ClassContent)
            { 
                return 0; 
            };
            // All java System.out will be redirected to this callback
            virtual void SystemOut(const char* Message)
            {
                std::cout << Message;
            }
            // All java System.err will be redirected to this callback
            virtual void SystemErr(const char* Message)
            {
                std::cerr << Message;
            }
            // Read single char (blocking)
            virtual bool SystemIn(char* c)
            {
                // Check if can read and then set
                int CharValue = getchar();
                bool IsEOF = CharValue == EOF;
                if (!IsEOF)
                {
                    *c = CharValue;
                }

                return !IsEOF;
            }
            // Checks if reading is possible
            virtual bool SystemInCanRead()
            {
                return std::cin.rdbuf()->in_avail() > 0;
            }

        private:
            // Internally required setup
            void SetupVM_Internal(jni::VMArguments& Args)
            {
                // Magically fit in our class loader
                Args.AddOption("-Djava.system.class.loader=com.fuechsl.jnihpp.ClassLoader");
            }

            // Function that will load all java ref objects
            bool LoadAllJavaRefs()
            {
                // === com.fuechsl.jnihpp.ClassLoadResult ===
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Cls_NativClassLoadResult, JNIEnv->FindClass("com/fuechsl/jnihpp/ClassLoadResult"));
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Fld_NativClassLoadResult_Loaded, JNIEnv->GetFieldID(jvt.JAVA_Cls_NativClassLoadResult, "loaded", "Z"));
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Fld_NativClassLoadResult_Data, JNIEnv->GetFieldID(jvt.JAVA_Cls_NativClassLoadResult, "data", "[B"));
                
                // === com.fuechsl.jnihpp.ClassLoader ===
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Cls_ClassLoader, JNIEnv->FindClass("com/fuechsl/jnihpp/ClassLoader"));
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Fld_ClassLoader_InstanceId, JNIEnv->GetStaticFieldID(jvt.JAVA_Cls_ClassLoader, "nativeInstanceId", "J"));

                // === com.fuechsl.jnihpp.Setup ===
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Cls_Setup, JNIEnv->FindClass("com/fuechsl/jnihpp/Setup"));
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Mth_Setup_RunSetup, JNIEnv->GetStaticMethodID(jvt.JAVA_Cls_Setup, "RunSetup", "(J)V"));

                // === com.fuechsl.jnihpp.NativePrintStream ===
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Cls_NativePrintStream, JNIEnv->FindClass("com/fuechsl/jnihpp/NativePrintStream"));

                // === com.fuechsl.jnihpp.NativeInputStream ===
                __JNIHPP_JNI_LOAD_AND_CHECK(jvt.JAVA_Cls_NativeInputStream, JNIEnv->FindClass("com/fuechsl/jnihpp/NativeInputStream"));

                // OK
                return true;
            }

            // Registers all the native functions
            bool RegisterNatives()
            {
                // Array
                std::array<JNINativeMethod, 8> NativeFunctions;
                
                // === com.fuechsl.jnihpp.ClassLoader ===
                // Functions for class loading by C++
                NativeFunctions[0].name = (char*)"NLoadClassData";
                NativeFunctions[0].signature = (char*)"(JLjava/lang/String;)Lcom/fuechsl/jnihpp/ClassLoadResult;";
                NativeFunctions[0].fnPtr = &JAVA_NLoadClassData;
                // Register natives (call)
                __JNIHPP_JNI_CHECK_RESULT(JNIEnv->RegisterNatives(jvt.JAVA_Cls_ClassLoader, &NativeFunctions[0], 1));

                // === com.fuechs.jnihpp.NativePrintStream ===
                // System.out.* callback
                NativeFunctions[0].name = (char*)"NStdOut";
                NativeFunctions[0].signature = (char*)"(JLjava/lang/String;)V";
                NativeFunctions[0].fnPtr = &JAVA_NStdOut;
                // System.err.* callback
                NativeFunctions[1].name = (char*)"NStdErr";
                NativeFunctions[1].signature = (char*)"(JLjava/lang/String;)V";
                NativeFunctions[1].fnPtr = &JAVA_NStdErr;
                // Register natives (call)
                __JNIHPP_JNI_CHECK_RESULT(JNIEnv->RegisterNatives(jvt.JAVA_Cls_NativePrintStream, &NativeFunctions[0], 2));

                // === com.fuechs.jnihpp.NativeInputStream ===
                // System.in.* callback READ
                NativeFunctions[0].name = (char*)"NReadByte";
                NativeFunctions[0].signature = (char*)"(J)I";
                NativeFunctions[0].fnPtr = &JAVA_NReadByte;
                // System.in.* callback CANREAD
                NativeFunctions[1].name = (char*)"NCanRead";
                NativeFunctions[1].signature = (char*)"(J)Z";
                NativeFunctions[1].fnPtr = &JAVA_NCanRead;
                // Register natives (call)
                __JNIHPP_JNI_CHECK_RESULT(JNIEnv->RegisterNatives(jvt.JAVA_Cls_NativeInputStream, &NativeFunctions[0], 2));

                // OK
                return true;
            }

            // Sets up all required java class states
            bool SetupClassStates()
            {
                // Set native instance id
                JNIEnv->SetStaticLongField(jvt.JAVA_Cls_ClassLoader, jvt.JAVA_Fld_ClassLoader_InstanceId, (jlong)this);

                // Run setup function
                JNIEnv->CallStaticVoidMethod(jvt.JAVA_Cls_Setup, jvt.JAVA_Mth_Setup_RunSetup, (jlong)this);

                return true;
            }

            // Callback for java --> native class loading
            static jobject JAVA_NLoadClassData(JNIEnv* JAVA_Env, jobject JAVA_This, jlong JAVA_InstanceRef, jstring JAVA_ClassName)
            {
                // Convert to instance pointer
                JavaVM* Interface = (JavaVM*)JAVA_InstanceRef;

                // Create result object
                jobject JAVA_LoadResult = JAVA_Env->AllocObject(Interface->jvt.JAVA_Cls_NativClassLoadResult);;
                if (JAVA_LoadResult)
                {
                    JAVA_Env->SetBooleanField(JAVA_LoadResult, Interface->jvt.JAVA_Fld_NativClassLoadResult_Loaded, false);
                }

                // Get string content
                const char* ClassName = JAVA_Env->GetStringUTFChars(JAVA_ClassName, nullptr);
                if (ClassName && JAVA_LoadResult)
                {
                    // Convert to c++ string
                    std::string ClassNameStd = ClassName;

                    // Call load class function
                    char* ClassContent = nullptr;
                    jsize ClassContentSize = Interface->LoadClass(ClassName, &ClassContent);
                    if (ClassContentSize)
                    {
                        // Allocate a new byte array
                        jbyteArray JAVA_ClassBytes = JAVA_Env->NewByteArray(ClassContentSize);
                        if (JAVA_ClassBytes)
                        {
                            // Copy buffer to java
                            JAVA_Env->SetByteArrayRegion(JAVA_ClassBytes, 0, ClassContentSize, (const jbyte*)ClassContent);

                            // Setup Object properties
                            JAVA_Env->SetBooleanField(JAVA_LoadResult, Interface->jvt.JAVA_Fld_NativClassLoadResult_Loaded, true);
                            JAVA_Env->SetObjectField(JAVA_LoadResult, Interface->jvt.JAVA_Fld_NativClassLoadResult_Data, JAVA_ClassBytes);
                        }

                        // Delete local ref
                        if(JAVA_ClassBytes)
                            JAVA_Env->DeleteLocalRef(JAVA_ClassBytes);
                    }
                 
                    // Free array using delete
                    if (ClassContent)
                        delete ClassContent;
                }

                // Delete ref
                if (JAVA_LoadResult)
                    JAVA_Env->DeleteLocalRef(JAVA_LoadResult);

                // Return result object (or null)
                return JAVA_LoadResult;
            }
            // Callback for std::cout
            static void JAVA_NStdOut(JNIEnv* JAVA_Env, jobject JAVA_This, jlong JAVA_InstanceRef, jstring JAVA_Message)
            {
                // Convert to instance pointer
                JavaVM* Interface = (JavaVM*)JAVA_InstanceRef;

                // Get message
                const char* Message = JAVA_Env->GetStringUTFChars(JAVA_Message, nullptr);
                if (Message)
                {
                    // Send to callback
                    Interface->SystemOut(Message);

                    // Release message
                    JAVA_Env->ReleaseStringUTFChars(JAVA_Message, Message);
                }
            }
            // Callback for std::cerr
            static void JAVA_NStdErr(JNIEnv* JAVA_Env, jobject JAVA_This, jlong JAVA_InstanceRef, jstring JAVA_Message)
            {
                // Convert to instance pointer
                JavaVM* Interface = (JavaVM*)JAVA_InstanceRef;

                // Get message
                const char* Message = JAVA_Env->GetStringUTFChars(JAVA_Message, nullptr);
                if (Message)
                {
                    // Send to callback
                    Interface->SystemErr(Message);

                    // Release message
                    JAVA_Env->ReleaseStringUTFChars(JAVA_Message, Message);
                }
            }
            // Callback for std::cin
            static jint JAVA_NReadByte(JNIEnv* JAVA_Env, jobject JAVA_This, jlong JAVA_InstanceRef)
            {
                // Convert to instance pointer
                JavaVM* Interface = (JavaVM*)JAVA_InstanceRef;

                // Call virtual function to get a char
                jint JavaReturn = -1;
                unsigned char Value = 0x00;
                if (Interface->SystemIn((char*)&Value))
                {
                    JavaReturn = Value;
                }

                return JavaReturn;
            }
            // Callback for checking std::cin
            static jboolean JAVA_NCanRead(JNIEnv* JAVA_Env, jobject JAVA_This, jlong JAVA_InstanceRef)
            {
                // Convert to instance pointer
                JavaVM* Interface = (JavaVM*)JAVA_InstanceRef;

                // Call virtual function with nullptr to get if reading is possible
                return Interface->SystemInCanRead();
            }

        private:
            // Java JNI and VM interface
            ::JavaVM* JVM = nullptr;
            ::JNIEnv* JNIEnv = nullptr;

            // Java refs
            JAVA_Table jvt;
    };
}
