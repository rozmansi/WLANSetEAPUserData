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

#include <WinStd\Com.h>
#include <WinStd\Common.h>
#include <WinStd\WLAN.h>
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
using namespace winstd;


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

    int result = -1;
    bool interactive = false;

    try {
        // Get command line arguments. (As Unicode, please.)
        int nArgs;
        unique_ptr<LPWSTR[], LocalFree_delete<LPWSTR[]> > pwcArglist(CommandLineToArgvW(GetCommandLineW(), &nArgs));
        if (!pwcArglist) {
            result = 100;
            throw win_runtime_error("CommandLineToArgvW() failed");
        }
        if (nArgs < 4) {
            result = 101;
            throw invalid_argument("Not enough arguments.");
        }

        for (int i = 4; i < nArgs; i++)
            if (_wcsicmp(pwcArglist[i], L"/I") == 0) interactive = true;

        // Initialize COM.
        com_initializer com_init(NULL);
        if (FAILED(com_init.status())) {
            result = 200;
            throw com_runtime_error(com_init.status(), "CoInitialize() failed");
        }

        // Load user data XML into memory.
        // Use MSXML6 IXMLDOMDocument to load XML to offload charset detection.
        _bstr_t user_data;
        {
            // Create XML document.
            _com_ptr_t<_com_IIID<IXMLDOMDocument, &__uuidof(IXMLDOMDocument)> > doc;
            HRESULT hr = doc.CreateInstance(CLSID_DOMDocument60, NULL, CLSCTX_ALL);
            if (FAILED(hr)) {
                result = 300;
                throw com_runtime_error(hr, "CoCreateInstance(CLSID_DOMDocument2) failed");
            }
            doc->put_async(VARIANT_FALSE);
            doc->put_validateOnParse(VARIANT_FALSE);

            // Load XML from user data file.
            VARIANT_BOOL succeeded = VARIANT_FALSE;
            hr = doc->load(_variant_t(pwcArglist[3]), &succeeded);
            if (FAILED(hr)) {
                result = 301;
                throw com_runtime_error(hr, string_printf("IXMLDOMDocument::load(%ls) failed", pwcArglist[3]));
            } else if (!succeeded) {
                result = 302;
                throw runtime_error(string_printf("IXMLDOMDocument::load(%ls) failed", pwcArglist[3]));
            }

            // Get document XML.
            BSTR bstr;
            hr = doc->get_xml(&bstr);
            if (FAILED(hr)) {
                result = 304;
                throw com_runtime_error(hr, "IXMLDOMDocument::get_xml() failed");
            }
            user_data.Attach(bstr);
        }

        // Open WLAN handle.
        DWORD dwNegotiatedVersion;
        wlan_handle wlan;
        if (!wlan.open(WLAN_API_MAKE_VERSION(2, 0), &dwNegotiatedVersion)) {
            result = 400;
            throw win_runtime_error("WlanOpenHandle() failed");
        }

        // Get a list of WLAN interfaces.
        unique_ptr<WLAN_INTERFACE_INFO_LIST, WlanFreeMemory_delete<WLAN_INTERFACE_INFO_LIST> > interfaces;
        {
            WLAN_INTERFACE_INFO_LIST *pInterfaceList;
            DWORD dwResult = WlanEnumInterfaces(wlan, NULL, &pInterfaceList);
            if (dwResult != ERROR_SUCCESS) {
                result = 401;
                throw win_runtime_error(dwResult, "WlanEnumInterfaces() failed");
            }
            interfaces.reset(pInterfaceList);
        }
        if (!interfaces->dwNumberOfItems) {
            result = 403;
            throw runtime_error("No ready WLAN interfaces found");
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
                wlan,
                &(interfaces->InterfaceInfo[i].InterfaceGuid),
                pwcArglist[1],
                wcstoul(pwcArglist[2], NULL, 10),
                user_data,
                NULL);
            if (dwResult == ERROR_SUCCESS) {
                // At least one interface/profile succeeded.
                success = true;
            } else if (interactive)
                MessageBox(NULL, tstring_printf(_T("WlanSetProfileEapXmlUserData() failed: %s"), win_runtime_error(dwResult).msg().c_str()).c_str(), NULL, MB_ICONWARNING | MB_OK);
        }

        return success ? 0 : 402;
    } catch (com_runtime_error err) {
        if (interactive)
            MessageBox(NULL, tstring_printf(_T("%hs: 0x%08x"), err.what(), err.number()).c_str(), NULL, MB_ICONERROR | MB_OK);
    } catch (win_runtime_error err) {
        if (interactive)
            MessageBox(NULL, tstring_printf(_T("%hs: %s"), err.what(), err.msg().c_str()).c_str(), NULL, MB_ICONERROR | MB_OK);
    } catch (exception err) {
        if (interactive)
            MessageBoxA(NULL, err.what(), NULL, MB_ICONERROR | MB_OK);
    }

    return result;
}
