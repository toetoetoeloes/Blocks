#include "defs.h"

LOCAL INT_PTR CALLBACK NewHighScoreDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:

            SendMessage (GetDlgItem (hDlg, IDC_NEWHIGHSCORENAME), EM_SETLIMITTEXT, 40, 0);

            SecureZeroMemory (g_szNewHighScoreName, sizeof (WCHAR) * MAX_STRING);

            return TRUE;

        case WM_COMMAND:

            switch (LOWORD (wParam))
            {
                case IDC_NEWHIGHSCORENAME:

                    if (HIWORD (wParam) == EN_CHANGE)
                    {
                        EnableWindow (GetDlgItem (hDlg, IDOK), GetWindowTextLengthW (GetDlgItem (hDlg, IDC_NEWHIGHSCORENAME)) > 0);
                    }

                    return TRUE;

                case IDOK:

                    GetDlgItemTextW(hDlg, IDC_NEWHIGHSCORENAME, g_szNewHighScoreName, MAX_STRING);

                case IDCANCEL:

                    EndDialog (hDlg, wParam);

                    return TRUE;
            }

    }

    return FALSE;
}

void ShowNewHighScoreDialog (void)
{
    DialogBoxW (NULL, MAKEINTRESOURCE (IDD_NEWHIGHSCORE), g_hwndMain, NewHighScoreDlgProc);
}



