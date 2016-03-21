// Copyright (c) 2012, Sowicm
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// * Neither the name of the Sowicm nor the names of its contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "ClassFactory.h"
#include "MyContextMenu.h"
#include <new>
#include <Shlwapi.h>

extern long g_cDllRef;

ClassFactory::ClassFactory() : m_cRef(1)
{
    InterlockedIncrement(&g_cDllRef);
}

ClassFactory::~ClassFactory()
{
    InterlockedDecrement(&g_cDllRef);
}

IFACEMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(ClassFactory, IClassFactory),
        {0}
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return cRef;
}

IFACEMETHODIMP ClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;

    // pUnkOuter is used for aggregation. We do not support it in the sample.
    if (pUnkOuter == NULL)
    {
        hr = E_OUTOFMEMORY;

        // Create the COM component.
        MyContextMenu *pExt = new (std::nothrow) MyContextMenu();
        if (pExt)
        {
            // Query the specified interface.
            hr = pExt->QueryInterface(riid, ppv);
            pExt->Release();
        }
    }

    return hr;
}

IFACEMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_cDllRef);
    }
    else
    {
        InterlockedDecrement(&g_cDllRef);
    }
    return S_OK;
}
