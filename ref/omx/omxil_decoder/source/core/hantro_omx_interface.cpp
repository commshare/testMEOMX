/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#define LOG_TAG "omx_interface"
//#include <utils/Log.h>

#include "pvlogger.h"

#include "pv_omxcore.h"
#include "omx_interface.h"

#define OMX_CORE_LIBRARY "libhantro_omx_core.so"

class HantroOMXInterface : public OMXInterface
{
    public:
        // Handle to the OMX core library
        void* ipHandle;

        ~HantroOMXInterface()
        {
            if ((NULL != ipHandle) && (0 != dlclose(ipHandle)))
            {
                // dlclose() returns non-zero value if close failed, check for errors
                const char* pErr = dlerror();
                if (NULL != pErr)
                {
                    //LOGE("HantroOMXInterface: Error closing library: %s\n", pErr);
                }
                else
                {
                    //LOGE("OsclSharedLibrary::Close: Error closing library, no error reported");
                }
            }

            ipHandle = NULL;
        };

        OsclAny* SharedLibraryLookup(const OsclUuid& aInterfaceId)
        {
            // Make sure ipHandle is valid. If ipHandle is NULL, the dlopen
            // call failed.
            if (ipHandle && aInterfaceId == OMX_INTERFACE_ID)
            {
                // the library lookup was successful
                return this;
            }
            // the ID doesn't match
            return NULL;
        };

        static HantroOMXInterface* Instance()
        {
            return OSCL_NEW(HantroOMXInterface, ());
        };

        bool UnloadWhenNotUsed(void)
        {
            // As of 9/22/08, the PV OMX core library can not be
            // safely unloaded and reloaded when the proxy interface
            // is enabled.
            return false;
        };

    private:

        HantroOMXInterface()
        {
            ipHandle = dlopen(OMX_CORE_LIBRARY, RTLD_NOW);

            if (NULL == ipHandle)
            {
                pOMX_Init = NULL;
                pOMX_Deinit = NULL;
                pOMX_ComponentNameEnum = NULL;
                pOMX_GetHandle = NULL;
                pOMX_FreeHandle = NULL;
                pOMX_GetComponentsOfRole = NULL;
                pOMX_GetRolesOfComponent = NULL;
                pOMX_SetupTunnel = NULL;
                pOMX_GetContentPipe = NULL;
                // added extra method to enable config parsing without instantiating the component
                pOMXConfigParser = NULL;
                // check for errors
                const char* pErr = dlerror();
                if (NULL == pErr)
                {
                    // No error reported, but no handle to the library
                    //LOGE("OsclLib::LoadLibrary: Error opening "
                    //   "library (%s) but no error reported\n", OMX_CORE_LIBRARY);
                }
                else
                {
                    // Error reported
                    //LOGE("OsclLib::LoadLibrary: Error opening "
                    //     "library (%s): %s\n", OMX_CORE_LIBRARY, pErr);
                }
            }
            else
            {
                // Lookup all the symbols in the OMX core
                pOMX_Init = (tpOMX_Init)dlsym(ipHandle, "hantro_OMX_Init");
                pOMX_Deinit = (tpOMX_Deinit)dlsym(ipHandle, "hantro_OMX_Deinit");
                pOMX_ComponentNameEnum = (tpOMX_ComponentNameEnum)dlsym(ipHandle, "hantro_OMX_ComponentNameEnum");
                pOMX_GetHandle = (tpOMX_GetHandle)dlsym(ipHandle, "hantro_OMX_GetHandle");
                pOMX_FreeHandle = (tpOMX_FreeHandle)dlsym(ipHandle, "hantro_OMX_FreeHandle");
                pOMX_GetComponentsOfRole = (tpOMX_GetComponentsOfRole)dlsym(ipHandle, "hantro_OMX_GetComponentsOfRole");
                pOMX_GetRolesOfComponent = (tpOMX_GetRolesOfComponent)dlsym(ipHandle, "hantro_OMX_GetRolesOfComponent");
                pOMX_SetupTunnel = (tpOMX_SetupTunnel)dlsym(ipHandle, "hantro_OMX_SetupTunnel");
                pOMXConfigParser = (tpOMXConfigParser)dlsym(ipHandle, "hantro_OMXConfigParserRedirect");
                pOMX_GetContentPipe = NULL; // (tpOMX_GetContentPipe)dlsym(ipHandle, "hantro_OMX_GetContentPipe");
            }
        };

};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return HantroOMXInterface::Instance();
    }

    OSCL_EXPORT_REF void PVReleaseInterface(void* interface)
    {
        HantroOMXInterface* pInterface = (HantroOMXInterface*)interface;
        if (pInterface)
        {
            OSCL_DELETE(pInterface);
        }
    }
}

