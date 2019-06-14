# WLANSetEAPUserData

Windows helper utility to set user data for a given WLAN profile

Binary downloads and change-logs can be found on the [project's release page](https://github.com/rozmansi/WLANSetEAPUserData/releases).


## Usage

```
WLANSetEAPUserData <profile> <flags> <user data URI> [/i]
```

| Parameter       | Explanation |
| --------------- | ----------- |
| `profile`       | The name of the network profile (typically same as SSID) |
| `flags`         | Flags to pass to `WlanSetProfileEapXmlUserData()` function call (decimal number: 0=Current User, 1=All Users) |
| `user data URI` | User data XML URI. Can be a path to an XML file, web URL where user data XML can be loaded from, etc. The XML schema varies according to the configured EAP method provider used by `profile`. For example: Microsoft's EAP-TLS requires the schema described in [EAP-TLS User Properties](https://msdn.microsoft.com/en-us/library/windows/desktop/bb204662.aspx). |
| /i              | Interactive: Pop-up a message dialog in case of error |


## Return codes

| Value | Meaning                                                                              |
| -----:| ------------------------------------------------------------------------------------ |
|     0 | Success (on at least one WLAN interface)                                             |
|   100 | `CommandLineToArgvW()` failed                                                        |
|   101 | Not enough arguments                                                                 |
|   200 | `CoInitialize()` failed                                                              |
|   300 | `CoCreateInstance(CLSID_DOMDocument2)` failed                                        |
|   301 | `IXMLDOMDocument::load()` failed                                                     |
|   302 | `IXMLDOMDocument::load()` reported an error in the XML document                      |
|   304 | `IXMLDOMDocument::get_xml()` failed                                                  |
|   400 | `WlanOpenHandle()` failed                                                            |
|   401 | `WlanEnumInterfaces()` failed                                                        |
|   402 | `WlanSetProfileEapXmlUserData()` failed on all WLAN interfaces for the given profile |
|   403 | No ready WLAN interfaces found                                                       |
