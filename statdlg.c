#include "defs.h"

LOCAL INT_PTR CALLBACK StatisticsDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   	WCHAR szBuffer[MAX_STRING];

    switch (uMsg)
    {
        case WM_INITDIALOG:

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKL]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKL, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKI]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKI, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKT]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKT, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKS]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKS, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKZ]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKZ, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKO]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKO, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d", g_nBlockStatistics[BLOCKJ]);
            SetDlgItemText (hDlg, IDC_STATISTICSBLOCKJ, szBuffer);

            StringCchPrintfW (szBuffer, MAX_STRING, L"%d",
                g_nBlockStatistics[BLOCKL] +
                g_nBlockStatistics[BLOCKI] +
                g_nBlockStatistics[BLOCKT] +
                g_nBlockStatistics[BLOCKS] +
                g_nBlockStatistics[BLOCKZ] +
                g_nBlockStatistics[BLOCKO] +
                g_nBlockStatistics[BLOCKJ]
                );

            SetDlgItemText (hDlg, IDC_STATISTICSTOTAL, szBuffer);

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

void ShowStatisticsDialog (void)
{
    DialogBox (NULL, MAKEINTRESOURCE (IDD_STATISTICS), g_hwndMain, StatisticsDlgProc);
}



