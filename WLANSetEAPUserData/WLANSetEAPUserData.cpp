/*
    Copyright 2016-2019 Simon Rozman

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

#include "WLANSetEAPUserData.h"
#include <comip.h>
#include <comutil.h>
#include <msxml6.h>
#include <tchar.h>
#include <memory>

#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#endif
#pragma comment(lib, "msxml6.lib")
#pragma comment(lib, "Wlanapi.lib")

using namespace std;


///
/// Main program body
///
int CALLBACK WinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPSTR     lpCmdLine,
    _In_     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance    );
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine    );
    UNREFERENCED_PARAMETER(nCmdShow     );

    // Get command line arguments. (As Unicode, please.)
    int nArgs;
    unique_ptr<LPWSTR[], LocalFree_delete<LPWSTR[]> > pwcArglist(CommandLineToArgvW(GetCommandLineW(), &nArgs));
    if (!pwcArglist) {
        //_ftprintf(stderr, _T("CommandLineToArgvW() failed (error %u).\n"), GetLastError());
        return 100;
    }
    if (nArgs <= 3) {
        //_ftprintf(stderr, _T("Not enough arguments.\n"));
        return 101;
    }

    // Initialize COM.
    com_initializer com_init(NULL);
    if (FAILED(com_init.status())) {
        //_ftprintf(stderr, _T("CoInitialize() failed (error 0x%08x).\n"), com_init.status());
        return 200;
    }

    // Load user data XML into memory.
    // Use MSXML6 IXMLDOMDocument to load XML to offload charset detection.
    _bstr_t user_data;
    {
        // Create XML document.
        _com_ptr_t<_com_IIID<IXMLDOMDocument, &__uuidof(IXMLDOMDocument)> > doc;
        HRESULT hr = doc.CreateInstance(CLSID_DOMDocument60, NULL, CLSCTX_ALL);
        if (FAILED(hr)) {
            //_ftprintf(stderr, _T("CoCreateInstance(CLSID_DOMDocument2) failed (error 0x%08x).\n"), hr);
            return 300;
        }
        doc->put_async(VARIANT_FALSE);
        doc->put_validateOnParse(VARIANT_FALSE);

        // Load XML from user data file.
        VARIANT_BOOL succeeded = VARIANT_FALSE;
        hr = doc->load(_variant_t(pwcArglist[3]), &succeeded);
        if (FAILED(hr)) {
            //_ftprintf(stderr, _T("IXMLDOMDocument::load(%ls) failed (error 0x%08x).\n"), pwcArglist[3], hr);
            return 301;
        } else if (!succeeded) {
            //_ftprintf(stderr, _T("IXMLDOMDocument::load(%ls) failed.\n"), pwcArglist[3]);
            return 302;
        }

        // Get document XML.
        BSTR bstr;
        hr = doc->get_xml(&bstr);
        if (FAILED(hr)) {
            //_ftprintf(stderr, _T("IXMLDOMDocument::get_xml() failed (error 0x%08x).\n"), hr);
            return 304;
        }
        user_data.Attach(bstr);
    }

    // Open WLAN handle.
    DWORD dwNegotiatedVersion;
    unique_ptr<void, WlanCloseHandle_delete> wlan;
    {
        HANDLE hWlan;
        DWORD dwResult = WlanOpenHandle(WLAN_API_MAKE_VERSION(2, 0), NULL, &dwNegotiatedVersion, &hWlan);
        if (dwResult != ERROR_SUCCESS) {
            //_ftprintf(stderr, _T("WlanOpenHandle() failed (error %u).\n"), dwResult);
            return 400;
        }
        wlan.reset(hWlan);
    }

    // Get a list of WLAN interfaces.
    unique_ptr<WLAN_INTERFACE_INFO_LIST, WlanFreeMemory_delete<WLAN_INTERFACE_INFO_LIST> > interfaces;
    {
        WLAN_INTERFACE_INFO_LIST *pInterfaceList;
        DWORD dwResult = WlanEnumInterfaces(wlan.get(), NULL, &pInterfaceList);
        if (dwResult != ERROR_SUCCESS) {
            //_ftprintf(stderr, _T("WlanEnumInterfaces() failed (error %u).\n"), dwResult);
            return 401;
        }
        interfaces.reset(pInterfaceList);
    }

    // Iterate over all WLAN interfaces.
    bool success = false;
    for (DWORD i = 0; i < interfaces->dwNumberOfItems; i++) {
        if (interfaces->InterfaceInfo[i].isState == wlan_interface_state_not_ready) {
            // This interface is not ready.
            continue;
        }

        // Set user data.
        DWORD dwResult = WlanSetProfileEapXmlUserData(
            wlan.get(),
            &(interfaces->InterfaceInfo[i].InterfaceGuid),
            pwcArglist[1],
            wcstoul(pwcArglist[2], NULL, 10),
            user_data,
            NULL);
        if (dwResult == ERROR_SUCCESS) {
            // At least one interface/profile succeeded.
            success = true;
        } else {
            //_ftprintf(stderr, _T("WlanSetProfileEapXmlUserData() failed (error %u).\n"), dwResult);
        }
    }

    return success ? 0 : interfaces->dwNumberOfItems ? 402 : 403;
}
