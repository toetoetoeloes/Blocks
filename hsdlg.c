#include "defs.h"

LOCAL INT_PTR CALLBACK HighScoresDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int i;
    WCHAR szBuffer[MAX_STRING];
    size_t len;

    switch (uMsg)
    {
        case WM_INITDIALOG:

            for (i = 0; i < 10; i++)
            {
                StringCchLengthW (g_hsHighScores[i].szName, MAX_STRING, &len);

                StringCchPrintfW (szBuffer, MAX_STRING, (len == 0) ? L"--" : L"%s", g_hsHighScores[i].szName);
                SetDlgItemText (hDlg, IDC_HIGHSCORESNAME0 + i, szBuffer);

                StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_hsHighScores[i].nLevel);
                SetDlgItemText (hDlg, IDC_HIGHSCORESLEVEL0 + i, szBuffer);

                StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_hsHighScores[i].nScore);
                SetDlgItemText (hDlg, IDC_HIGHSCORESSCORE0 + i, szBuffer);
            }

            return TRUE;

        case WM_COMMAND:

            switch (LOWORD (wParam))
            {
                case IDOK:
                case IDCANCEL:  // Cancel button in upper right corner of dialog window.

                    EndDialog (hDlg, wParam);

                    return TRUE;
            }

    }

    return FALSE;
}

void ShowHighScoresDialog (void)
{
    DialogBox (NULL, MAKEINTRESOURCE (IDD_HIGHSCORES), g_hwndMain, HighScoresDlgProc);
}


