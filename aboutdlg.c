#include "defs.h"

void ShowAboutDialog (void)
{
    WCHAR szFilename[MAX_PATH];
    WCHAR szProductName[MAX_STRING];
    WCHAR szProductVersion[MAX_STRING];
    WCHAR szLegalCopyright[MAX_STRING];
    WCHAR szApp[MAX_STRING];
    DWORD dwSize, dwHandle;
    LPVOID lpData, lpBuffer;
    UINT uLen;

    dwSize = GetModuleFileNameW (NULL, szFilename, MAX_PATH);
    if (dwSize != 0)
    {
        dwSize = GetFileVersionInfoSizeW (szFilename, &dwHandle);
        if (dwSize != 0)
        {
            lpData = HeapAlloc (GetProcessHeap (), 0, (SIZE_T) dwSize);
            if (lpData != NULL)
            {
                if (GetFileVersionInfoW (szFilename, 0, dwSize, lpData))
                {
                    SecureZeroMemory (szProductName, MAX_STRING * sizeof (WCHAR));
                    SecureZeroMemory (szProductVersion, MAX_STRING * sizeof (WCHAR));
                    SecureZeroMemory (szLegalCopyright, MAX_STRING * sizeof (WCHAR));

                    // 0409-FDE9 is the lang-codepage found in the resource file.

                    if (VerQueryValueW (lpData, L"\\StringFileInfo\\0409FDE9\\ProductName", &lpBuffer, &uLen))
                    {
                        CopyMemory (szProductName, lpBuffer, sizeof (WCHAR) * min(uLen, MAX_STRING));
                        szProductName[MAX_STRING - 1] = L'\0';  // for safety reasons
                    }

                    if (VerQueryValueW (lpData, L"\\StringFileInfo\\0409FDE9\\ProductVersion", &lpBuffer, &uLen))
                    {
                        CopyMemory (szProductVersion, lpBuffer, sizeof (WCHAR) * min(uLen, MAX_STRING));
                        szProductVersion[MAX_STRING - 1] = L'\0';  // for safety reasons
                    }

                    if (VerQueryValueW (lpData, L"\\StringFileInfo\\0409FDE9\\LegalCopyright", &lpBuffer, &uLen))
                    {
                        CopyMemory (szLegalCopyright, lpBuffer, sizeof (WCHAR) * min(uLen, MAX_STRING));
                        szLegalCopyright[MAX_STRING - 1] = L'\0';  // for safety reasons
                    }

                    if (szProductName[0] == L'\0' || szProductVersion[0] == L'\0' || szLegalCopyright[0] == L'\0')
                    {
                        ShellAboutW (g_hwndMain, L"", NULL, NULL);
                    }
                    else
                    {
                        StringCchPrintfW (szApp, MAX_STRING, L"%s#%s %s ― %s", szProductName, szProductName, szProductVersion, szLegalCopyright);

                        // Not explicitly specifying the width and height of the icon results in an ugly icon.

                        ShellAboutW (g_hwndMain, szApp, NULL, LoadImage (g_hInstance, MAKEINTRESOURCEW (IDI_ICON), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR|LR_SHARED));
                    }
                }
                HeapFree (GetProcessHeap (), 0, lpData);
            }
        }
    }
}


