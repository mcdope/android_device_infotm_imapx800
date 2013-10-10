/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BoardItems"
#include <utils/Log.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>

#include "jni.h"
#include "items.h"


static jint jni_item_exist(JNIEnv *env, jobject thiz,
			jstring key)
{
	const char *_k;
	int ret;

	_k = env->GetStringUTFChars(key, false);
	if(_k == NULL) {
		ALOGE("%s: failed to get key string\n", __func__);
		return -1;
	}

	ret = item_exist(_k);
	env->ReleaseStringUTFChars(key, _k);

	return ret;
}

static jint jni_item_equal(JNIEnv *env, jobject thiz,
			jstring key, jstring value, jint section)
{
	const char *_k, *_v;
	int ret;

	_k = env->GetStringUTFChars(key,   false);
	_v = env->GetStringUTFChars(value, false);

	if(!_k || !_v) {
		ALOGE("%s: failed to get strings %d: %d\n", __func__,
					(jint)_k, (jint)_v);
		return -1;
	}

	ret = item_equal(_k, _v, section);
	env->ReleaseStringUTFChars(key,   _k);
	env->ReleaseStringUTFChars(value, _v);

	return ret;
}

static jint jni_item_integer(JNIEnv *env, jobject thiz,
			jstring key, jint section)
{
	const char *_k;
	int ret;

	_k = env->GetStringUTFChars(key,   false);
	if(!_k) {
		ALOGE("%s: failed to get strings\n", __func__);
		return -1;
	}

	ret = item_integer(_k, section);
	env->ReleaseStringUTFChars(key, _k);

	return ret;
}

static jstring jni_item_string(JNIEnv *env, jobject thiz,
			jstring key, jint section)
{
	const char *_k;
	char _s[ITEM_MAX_LEN] = "Error";
	int ret;                                          

	_k = env->GetStringUTFChars(key,   false);
	if(!_k) {
		ALOGE("%s: failed to get strings\n", __func__);
		goto out;
	}

	item_string(_s, _k, section);

out:
	return env->NewStringUTF(_s);
}

static jstring jni_item_string_item(JNIEnv *env, jobject thiz,
			jstring key, jint section)
{
	const char *_k;
	char _s[ITEM_MAX_LEN] = "Error";
	int ret;                                          

	_k = env->GetStringUTFChars(key,   false);
	if(!_k) {
		ALOGE("%s: failed to get strings\n", __func__);
		goto out;
	}

	item_string_item(_s, _k, section);

out:
	return env->NewStringUTF(_s);
}

static jstring jni_item_file(JNIEnv *env, jobject thiz)
{
	return env->NewStringUTF(ITEMS_NODE);
}

static JNINativeMethod methods[] = {
	{"ItemExist",   "(Ljava/lang/String;)I", (void *)jni_item_exist},
	{"ItemEqual",   "(Ljava/lang/String;Ljava/lang/String;I)I", (void *)jni_item_equal},
	{"ItemInteger", "(Ljava/lang/String;I)I", (void *)jni_item_integer},
	{"ItemString",  "(Ljava/lang/String;I)Ljava/lang/String;", (void *)jni_item_string},
	{"ItemStringItem",  "(Ljava/lang/String;I)Ljava/lang/String;", (void *)jni_item_string_item},
	{"ItemFile",    "()Ljava/lang/String;", (void *)jni_item_file},
};

static const char *classPathName = "com/infotm/android/Items";








/*
 * ---->>>> the belowing code stays the same
 * <<<<-------------------------------------- */



/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName, methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

/*
 * This is called by the VM when the shared library is first loaded.
 */
 
typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    
    ALOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }
    
    result = JNI_VERSION_1_4;
    
bail:
    return result;
}

