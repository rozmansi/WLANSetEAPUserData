/*
    Copyright 2016 Simon Rozman

    This file is part of WLANSetEAPUserData.

    WLANSetEAPUserData is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WLANSetEAPUserData is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WLANSetEAPUserData. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <Windows.h>


///
/// Deleter for unique_ptr using LocalFree
///
template <class _Ty>
struct LocalFree_delete
{
    ///
    /// Frees memory
    ///
    /// \sa [LocalFree function](https://msdn.microsoft.com/en-us/library/windows/desktop/aa366730.aspx)
    ///
    inline void operator()(_Ty *_Ptr) const
    {
        LocalFree(_Ptr);
    }
};


///
/// Deleter for unique_ptr to array of unknown size using LocalFree
///
template <class _Ty>
struct LocalFree_delete<_Ty[]>
{
    ///
    /// Frees memory
    ///
    inline void operator()(_Ty *_Ptr) const
    {
        LocalFree(_Ptr);
    }

    ///
    /// Frees memory
    ///
    /// \sa [LocalFree function](https://msdn.microsoft.com/en-us/library/windows/desktop/aa366730.aspx)
    ///
    template<class _Other>
    inline void operator()(_Other *) const
    {
        LocalFree(_Ptr);
    }
};


#include <ObjBase.h>

///
/// Context scope automatic COM (un)initialization
///
class com_initializer
{
public:
    ///
    /// Initializes the COM library on the current thread and identifies the concurrency model as single-thread apartment (STA).
    ///
    /// \sa [CoInitialize function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms678543.aspx)
    ///
    inline com_initializer(_In_opt_ LPVOID pvReserved)
    {
        m_result = CoInitialize(pvReserved);
    }


    ///
    /// Initializes the COM library for use by the calling thread, sets the thread's concurrency model, and creates a new apartment for the thread if one is required.
    ///
    /// \sa [CoInitializeEx function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms695279.aspx)
    ///
    inline com_initializer(_In_opt_ LPVOID pvReserved, _In_ DWORD dwCoInit)
    {
        m_result = CoInitializeEx(pvReserved, dwCoInit);
    }


    ///
    /// Uninitializes COM.
    ///
    /// \sa [CoUninitialize function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms688715.aspx)
    ///
    virtual ~com_initializer()
    {
        if (SUCCEEDED(m_result))
            CoUninitialize();
    }


    ///
    /// Return result of `CoInitialize()` call.
    ///
    /// \sa [CoInitialize function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms678543.aspx)
    ///
    inline HRESULT status() const
    {
        return m_result;
    }

protected:
    HRESULT m_result;   ///< Result of CoInitialize call
};


#include <wlanapi.h>

///
/// Deleter for unique_ptr using WlanCloseHandle
///
struct WlanCloseHandle_delete
{
    ///
    /// Closes the WLAN handle
    ///
    /// \sa [WlanCloseHandle function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms706610.aspx)
    ///
    inline void operator()(void *_Ptr) const
    {
        WlanCloseHandle(_Ptr, NULL);
    }
};


///
/// Deleter for unique_ptr using WlanFreeMemory
///
template <class _Ty>
struct WlanFreeMemory_delete
{
    ///
    /// Frees memory
    ///
    /// \sa [WlanFreeMemory function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms706722.aspx)
    ///
    void operator()(_Ty *_Ptr) const
    {
        WlanFreeMemory(_Ptr);
    }
};
