// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"

// include notifications constants file here

#include <NotificationsReunionEndpoint_h.h>

// Temporarily disable C4324 because WRL generates a false (well, irrelevant) warning
//   'Microsoft::WRL::Details::StaticStorage<Microsoft::WRL::Details::OutOfProcModuleBase<ModuleT>::GenericReleaseNotifier<T>,Microsoft::WRL::Details::StorageInstance::OutOfProcCallbackBuffer1,ModuleT>': structure was padded due to alignment specifier
#pragma warning(push)
#pragma warning(disable:4324)
#include <wrl.h>
#pragma warning(pop)

using namespace Microsoft::WRL;

struct __declspec(uuid("330EC755-31F2-40A7-977D-B0ABB1E1E52E")) NotificationsReunionEndpointImpl WrlFinal : RuntimeClass<RuntimeClassFlags<ClassicCom>, INotificationsReunionEndpoint>
{
    STDMETHODIMP GetStringFromLRP(/*[out, retval]*/ PWSTR* packageFullName)
    {
        *packageFullName = nullptr;

        // working too hard I guess
        size_t length = 0;
        StringCchLength(L"Hi from the Long Running Process UwU", STRSAFE_MAX_CCH, &length);

        PWSTR copy = reinterpret_cast<PWSTR>(CoTaskMemAlloc(sizeof(WCHAR) * (length + 1)));
        RETURN_IF_NULL_ALLOC(copy);

        RETURN_IF_FAILED(StringCchCopy(copy, length + 1, L"Hi from the Long Running Process UwU"));

        *packageFullName = copy;
        return S_OK;
    }
};
CoCreatableClass(NotificationsReunionEndpointImpl);

wil::unique_event g_endOfTheLine;

void EndOfTheLine()
{
    g_endOfTheLine.SetEvent();
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    RETURN_IF_FAILED(::CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

    wil::unique_event endOfTheLine(::CreateEventW(nullptr, TRUE, FALSE, nullptr));
    RETURN_LAST_ERROR_IF_NULL(endOfTheLine);
    g_endOfTheLine = std::move(endOfTheLine);

    auto& module = Module<OutOfProc>::Create(EndOfTheLine);
    RETURN_IF_FAILED(module.RegisterObjects());

    g_endOfTheLine.wait();

    (void)LOG_IF_FAILED(module.UnregisterObjects());
    module.Terminate();

    ::CoUninitialize();

    return 0;
}
