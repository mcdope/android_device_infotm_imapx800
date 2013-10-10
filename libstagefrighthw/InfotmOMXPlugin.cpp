/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANInfotmES OR CONDIInfotmONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "InfotmOMXPlugin.h"

#include <dlfcn.h>

#include <media/hardware/HardwareAPI.h>
#include <media/stagefright/foundation/ADebug.h>

namespace android {

extern "C" OMXPluginBase *createOMXPlugin() {
    return new InfotmOMXPlugin;
}

extern "C" void destroyOMXPlugin(OMXPluginBase *plugin) {
    delete plugin;
}

#define LIBOMX "libIM_OMX_Core.so"

InfotmOMXPlugin::InfotmOMXPlugin()
    : mLibHandle(dlopen(LIBOMX, RTLD_NOW)),
      mInit(NULL),
      mDeinit(NULL),
      mComponentNameEnum(NULL),
      mGetHandle(NULL),
      mFreeHandle(NULL),
      mGetRolesOfComponentHandle(NULL) {
    if (mLibHandle != NULL) {
        mInit = (InitFunc)dlsym(mLibHandle, "IM_OMX_Init");
        mDeinit = (DeinitFunc)dlsym(mLibHandle, "IM_OMX_Deinit");
        mComponentNameEnum =
            (ComponentNameEnumFunc)dlsym(mLibHandle, "IM_OMX_ComponentNameEnum");
        mGetHandle = (GetHandleFunc)dlsym(mLibHandle, "IM_OMX_GetHandle");
        mFreeHandle = (FreeHandleFunc)dlsym(mLibHandle, "IM_OMX_FreeHandle");
        mGetRolesOfComponentHandle =
            (GetRolesOfComponentFunc)dlsym(
                    mLibHandle, "IM_OMX_GetRolesOfComponent");
        if(mInit == NULL || mDeinit == NULL ||
                mComponentNameEnum == NULL || mGetHandle == NULL || 
                mFreeHandle == NULL || mGetRolesOfComponentHandle == NULL){
            ALOGE("can not find all the interface from libIM_OMX_Core.so!");
            dlclose(mLibHandle);
            mLibHandle = NULL;
            return;
        }
        (*mInit)();
    }
    else
        ALOGE("%s: failed to load %s", __func__, LIBOMX);
}

InfotmOMXPlugin::~InfotmOMXPlugin() {
    if (mLibHandle != NULL) {
        (*mDeinit)();

        dlclose(mLibHandle);
        mLibHandle = NULL;
    }
}

OMX_ERRORTYPE InfotmOMXPlugin::makeComponentInstance(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component) {
    if (mLibHandle == NULL) {
        return OMX_ErrorUndefined;
    }

    return (*mGetHandle)(
            reinterpret_cast<OMX_HANDLETYPE *>(component),
            const_cast<char *>(name),
            appData, const_cast<OMX_CALLBACKTYPE *>(callbacks));
}

OMX_ERRORTYPE InfotmOMXPlugin::destroyComponentInstance(
        OMX_COMPONENTTYPE *component) {
    if (mLibHandle == NULL) {
        return OMX_ErrorUndefined;
    }

    return (*mFreeHandle)(reinterpret_cast<OMX_HANDLETYPE *>(component));
}

OMX_ERRORTYPE InfotmOMXPlugin::enumerateComponents(
        OMX_STRING name,
        size_t size,
        OMX_U32 index) {
    if (mLibHandle == NULL) {
		ALOGE("mLibHandle is NULL!");
        return OMX_ErrorUndefined;
    }

    return (*mComponentNameEnum)(name, size, index);
}

OMX_ERRORTYPE InfotmOMXPlugin::getRolesOfComponent(
        const char *name,
        Vector<String8> *roles) {
    roles->clear();

    if (mLibHandle == NULL) {
        return OMX_ErrorUndefined;
    }

    OMX_U32 numRoles;
    OMX_ERRORTYPE err = (*mGetRolesOfComponentHandle)(
            const_cast<OMX_STRING>(name), &numRoles, NULL);

    if (err != OMX_ErrorNone) {
        ALOGE("GetRolesOfComponents failed!\n");
        return err;
    }

    if (numRoles > 0) {
        OMX_U8 **array = new OMX_U8 *[numRoles];
        for (OMX_U32 i = 0; i < numRoles; ++i) {
            array[i] = new OMX_U8[OMX_MAX_STRINGNAME_SIZE];
        }

        err = (*mGetRolesOfComponentHandle)(
                const_cast<OMX_STRING>(name), &numRoles, array);

        CHECK_EQ(err, OMX_ErrorNone);

        for (OMX_U32 i = 0; i < numRoles; ++i) {
            String8 s((const char *)array[i]);
            roles->push(s);

            delete[] array[i];
            array[i] = NULL;
        }

        delete[] array;
        array = NULL;
    }

    return OMX_ErrorNone;
}

}  // namespace android
