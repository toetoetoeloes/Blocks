// There is no sound in this game due to the fact that the PlaySound()
// function slowed down the program after a while and I didn't want to
// resort to other APIs/libraries.

#include "defs.h"

#define ID_STATUSBAR    1

LOCAL LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LOCAL void SetStatusBarSize (void);
LOCAL void UpdateStatusBar (void);
LOCAL void UpdateScore (void);
LOCAL void NewGame (void);
LOCAL void NewBlock (void);
LOCAL void EliminateFullRows (void);
LOCAL void LoadData (void);
LOCAL void SaveData (void);
LOCAL void Pause (BOOL fGamePaused);
LOCAL void SetColorTableEntries (COLORTABLEENTRIES cte);

LOCAL BOOL MoveLeft (void);
LOCAL BOOL MoveRight (void);
LOCAL BOOL Rotate (void);
LOCAL BOOL PutBlock (int x, int y, BLOCKTYPE btType, int iFrame, BYTE bColorIndex, BOOL fTest);
LOCAL BOOL Move (int dx, int dy);
LOCAL BOOL MoveDown (BOOL fDrop);
LOCAL BOOL ShiftRow (int iRow);

LOCAL int GetBlockSpeed (void);
LOCAL int FindFullRow (int iStartRow);


HINSTANCE g_hInstance;                      // Handle to the current running application.

HWND g_hwndMain;

HIGHSCORE g_hsHighScores[10];

int g_nBlockStatistics[7];                  // Use BLOCKTYPE values to index this array.

WCHAR g_szNewHighScoreName[MAX_STRING];     // Used when user has a top 10 score.

LOCAL HWND g_hwndStatusBar;

LOCAL BYTE g_bGrid[ROWS][COLS];             // The grid. 0 = empty cell. 1 = current block. > 2 = color index - 2.
                                            // The grid does NOT have two hidden top rows to spawn blocks. Blocks
											// are spawned in the second and third top row of the grid.
											
LOCAL BLOCKTYPE g_btCurrentBlockType;

LOCAL GAMESTATE g_gsGameState;

LOCAL int g_iCellSize;                      // Cell width and height in pixels.
LOCAL int g_iCellSpacing = 1;               // Number of pixels between cells.
LOCAL int g_iPadding = 10;                  // Grid padding in pixels.
LOCAL int g_nOriginX;                       // X coordinate of current block frame. Can be negative.
LOCAL int g_nOriginY;                       // Y coordinate of current block frame. Cannot be negative.
LOCAL int g_iCurrentFrame;                  // Current block frame index in g_pBlocks.
LOCAL int g_iFullRow = -1;                  // Row index of 'first full row starting from the bottom'.
LOCAL int g_nFullRows;                      // Number of full rows during game.
LOCAL int g_nLevel;                         // Current level [0-9].
LOCAL int g_nScore;                         // Game score.
LOCAL int g_nFreeFallIters;                 // Number of non-user block movements.
LOCAL int g_nAnimationSpeed = 50;           // Full row animation in milliseconds.

LOCAL COLORTABLEENTRIES g_cteColorTableEntries;         // Set by SetColorTableEntries ().

LOCAL COLORREF g_crColorTable[7] =
{
    RGB (255,255,0),
    RGB (0,255,255)
};

LOCAL BOOL g_fGamePaused = FALSE;

LOCAL WCHAR g_szAppName[] = L"Blocks";


int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEXW wcex;
    MSG msg;
    BOOL bRet;
    HACCEL hAccel;
    INITCOMMONCONTROLSEX iccex;

    iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
    iccex.dwICC = ICC_BAR_CLASSES;
    if (!InitCommonControlsEx (&iccex))
    {
        return -1;
    }

    if (BufferedPaintInit () != S_OK)
    {
        return -1;
    }

    SetProcessDPIAware ();

    wcex.cbSize        = sizeof (WNDCLASSEXW);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = (WNDPROC) WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIconW (hInstance, MAKEINTRESOURCEW (IDI_ICON));
    wcex.hCursor       = LoadCursorW (NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName  = L"MainMenu";
    wcex.lpszClassName = g_szAppName;
    wcex.hIconSm       = 0;

    RegisterClassExW (&wcex);

    g_hInstance = hInstance;

    g_hwndMain = CreateWindowExW (
                     0,
                     g_szAppName,
                     g_szAppName,
                     WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     NULL,
                     NULL,
                     hInstance,
                     NULL
                     );

    if (g_hwndMain == NULL)
    {
        BufferedPaintUnInit ();
        return -1;
    }

    // Load settings and high scores.

    LoadData ();

    ShowWindow (g_hwndMain, nCmdShow);
    UpdateWindow (g_hwndMain);

    hAccel = LoadAcceleratorsW (hInstance, L"AcceleratorTable");

    // We do this here because SetTimer () doesn't seem to work in WM_CREATE.

    NewGame ();

    // The second parameter must be NULL, otherwise the application
    // will hang after WM_DESTROY because it cannot receive messages
    // anymore.

    while ( (bRet = GetMessage (&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            // error

            break;
        }
        else
        {
            // Check for accelerator keystrokes.

            if (!TranslateAccelerator (g_hwndMain, hAccel, &msg))
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc, rcStatusBar;
    int i, j, x, y, iClientWidth, iClientHeight;
    PAINTSTRUCT ps;
    HPAINTBUFFER pb;
    HDC hDC, hNewDC;
    HGDIOBJ hOldPenAndBrush;
    COLORREF clrColor;
    BOOL fPaused;

    switch (message)
    {
        case WM_COMMAND:

            switch (LOWORD (wParam))
            {
                case IDM_GAMENEWGAME:

                    if (MessageBoxW (g_hwndMain, L"Are you sure you want to start a new game?", g_szAppName,
                                     MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
                    {
                        NewGame ();
                    }

                    return 0;

                case IDM_GAMEPAUSE:

                    Pause (!g_fGamePaused);

                    return 0;

                case IDM_GAMEHIGHSCORES:

                    fPaused = g_fGamePaused;
                    Pause (TRUE);
                    ShowHighScoresDialog ();
                    Pause (fPaused);

                    return 0;

                case IDM_GAMESTATISTICS:

                    fPaused = g_fGamePaused;
                    Pause (TRUE);
                    ShowStatisticsDialog ();
                    Pause (fPaused);

                    return 0;

                case IDM_GAMECOLORSMONOCHROME:

                    SetColorTableEntries (COLORTABLE_MONOCHROME);

                    return 0;

                case IDM_GAMECOLORSBICOLOR:

                    SetColorTableEntries (COLORTABLE_BICOLOR);

                    return 0;

                case IDM_GAMECOLORSMULTICOLOR:

                    SetColorTableEntries (COLORTABLE_MULTICOLOR);

                    return 0;

                case IDM_GAMEEXIT:

                    SendMessage (hWnd, WM_CLOSE, 0, 0);

                    return 0;

                case IDM_HELPABOUT:

                    fPaused = g_fGamePaused;
                    Pause (TRUE);
                    ShowAboutDialog ();
                    Pause (fPaused);

                    return 0;

            }

            break;

        case WM_CREATE:

            g_hwndStatusBar = CreateWindowEx (
                                0,
                                STATUSCLASSNAME,
                                (PCTSTR) NULL,
                                SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE,
                                0, 0, 0, 0,
                                hWnd,
                                (HMENU) ID_STATUSBAR,
                                g_hInstance,
                                NULL
                                );

            SetColorTableEntries (COLORTABLE_BICOLOR);

            // A call to SetTimer() in WM_CREATE doesn't seem to send WM_TIMER messages.

            return 0;

        case WM_ERASEBKGND:

            return 1;

        case WM_INITMENUPOPUP:

            if (!HIWORD(lParam))    // If not window menu ...
            {
                CheckMenuItem (
                    (HMENU) wParam,
                    IDM_GAMEPAUSE,
                    g_fGamePaused ? MF_CHECKED : MF_UNCHECKED
                );

                CheckMenuRadioItem (
                    (HMENU) wParam,
                    IDM_GAMECOLORSMONOCHROME,
                    IDM_GAMECOLORSMULTICOLOR,
                    IDM_GAMECOLORSMONOCHROME + g_cteColorTableEntries,
                    MF_BYCOMMAND
                );
            }

            return 0;

        case WM_KEYDOWN:

            if (!g_fGamePaused)
            {
                switch (wParam)
                {
                    case VK_LEFT:
                        MoveLeft ();
                        break;
                    case VK_RIGHT:
                        MoveRight ();
                        break;
                    case VK_UP:
                        Rotate ();
                        break;
                    case VK_DOWN:
                        MoveDown (FALSE);
                        break;
                    case VK_SPACE:
                        MoveDown (TRUE);
                        break;
                }
            }

            if (wParam == VK_ESCAPE)
            {
                DestroyWindow (hWnd);
            }

            return 0;

        case WM_PAINT:

            GetClientRect(hWnd, &rc);
            iClientWidth = rc.right - rc.left;
            iClientHeight = rc.bottom - rc.top;

            GetWindowRect (g_hwndStatusBar, &rcStatusBar);
            iClientHeight -= rcStatusBar.bottom - rcStatusBar.top;

            hDC = BeginPaint (hWnd, (LPPAINTSTRUCT) &ps);
            pb = BeginBufferedPaint (hDC, &rc, BPBF_COMPATIBLEBITMAP, NULL, &hNewDC);

            hOldPenAndBrush = SelectObject (hNewDC, GetStockObject (DC_PEN));
            SelectObject (hNewDC, GetStockObject (NULL_PEN));
            SelectObject (hNewDC, GetStockObject (DKGRAY_BRUSH));
            Rectangle (hNewDC, rc.left, rc.top, rc.right, rc.bottom);

            SelectObject (hNewDC, GetStockObject (DC_BRUSH));

            y = g_iPadding + (iClientHeight - (ROWS * (g_iCellSize + g_iCellSpacing) + g_iCellSpacing + g_iPadding * 2)) / 2;
            for (i = 0; i < ROWS; i++)
            {
                x = g_iPadding + (iClientWidth - (COLS * (g_iCellSize + g_iCellSpacing) + g_iCellSpacing + g_iPadding * 2)) / 2;
                for (j = 0; j < COLS; j++)
                {
                    switch (g_bGrid[i][j])
                    {
                        case 0:
                            clrColor = RGB(0,0,0);
                            break;

                        case 1:
                            clrColor = g_crColorTable[g_btCurrentBlockType];
                            break;

                        default:
                            clrColor = g_crColorTable[g_bGrid[i][j] - 2];
                    }

                    SetDCBrushColor (hNewDC, clrColor);

                    // We use a NULL_PEN, so we add 1 to right and bottom because according to
                    // the documentation, Rectangle () draws a rectangle one less in width and
                    // height if a NULL_PEN is used.

                    Rectangle (hNewDC, x, y, x + g_iCellSize + 1, y + g_iCellSize + 1);

                    x += g_iCellSize + g_iCellSpacing;
                }
                y += g_iCellSize + g_iCellSpacing;
            }

            SelectObject (hNewDC, hOldPenAndBrush);

            EndBufferedPaint (pb, TRUE);
            EndPaint(hWnd, &ps);

            return 0;

        case WM_SIZE:

            // Update the size and parts of the status bar.

            SetStatusBarSize ();

            // Get client width and height. The client height is the client height of the main window
            // minus the height of the status bar.

            GetClientRect (hWnd, &rc);
            iClientWidth = rc.right - rc.left;
            iClientHeight = rc.bottom - rc.top;

            GetWindowRect (g_hwndStatusBar, &rcStatusBar);
            iClientHeight -= rcStatusBar.bottom - rcStatusBar.top;

            // Calculate cell size. Doing this in WM_PAINT sometimes gave unexpected results.
            // We first try to fit the grid in ClientHeight and if the grid is too large, we
            // shrink it so that it fits the client width.

            g_iCellSize = (iClientHeight - ((ROWS + 1) * g_iCellSpacing) - g_iPadding * 2) / ROWS;
            if (COLS * (g_iCellSize + g_iCellSpacing) + g_iCellSpacing + g_iPadding * 2 > iClientWidth)
                g_iCellSize = (iClientWidth - ((COLS + 1) * g_iCellSpacing) - g_iPadding * 2) / COLS;

            if (g_iCellSize < 0)
                g_iCellSize = 0;

            return 0;

        case WM_TIMER:

            switch (g_gsGameState)
            {
                case GAMESTATE_PLAY:
                    MoveDown (FALSE);
                    break;

                case GAMESTATE_ELIMINATEFULLROWS:
                    EliminateFullRows ();
                    break;

            }

            return 0;

        case WM_CLOSE:

            SaveData ();

            break;

        case WM_DESTROY:

            PostQuitMessage (0);

            return 0;

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void NewGame (void)
{
    g_nScore = 0;
    g_nLevel = 0;
    g_iFullRow = -1;
    g_nFullRows = 0;
    g_nFreeFallIters = 0;
    g_fGamePaused = FALSE;

    SecureZeroMemory (g_nBlockStatistics, sizeof (int) * 7);
    SecureZeroMemory (g_bGrid, sizeof (BYTE) * ROWS * COLS);

    NewBlock ();

    UpdateStatusBar ();
}

BOOL PutBlock (int x, int y, BLOCKTYPE btType, int iFrame, BYTE bColorIndex, BOOL fTest)
{
    int i, row, col;

    // Decode 16-bit block "btType", frame "iFrame".

    for (i = 0; i < 16; i++)
    {
        if ((g_pBlocks[btType][iFrame] >> (15 - i)) & 0x0001)
        {
            row = y + i / 4;
            col = x + i % 4;
            if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
            {
                if (fTest)
                {
                    // Don't modify grid, just test. Ignore 0 and 1.

                    if (g_bGrid[row][col] != 0 && g_bGrid[row][col] != 1)
                        return FALSE;
                }
                else
                {
                    // - bColorIndex = 0 represents an empty cell
                    // - bColorIndex = 1 represents the current block
                    // - bColorIndex > 1 represents a color index in the color table (g_crColorTable[bColorIndex - 2])

                    g_bGrid[row][col] = bColorIndex;
                }
            }
            else
            {
                // If a 1 is outside the grid, the block cannot be placed at (x,y).

                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL Move (int dx, int dy)
{
    if (PutBlock (g_nOriginX + dx, g_nOriginY + dy, g_btCurrentBlockType, g_iCurrentFrame, 1, TRUE))
    {
        // Clear current block position.
        PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, g_iCurrentFrame, 0, FALSE);

        PutBlock (g_nOriginX + dx, g_nOriginY + dy, g_btCurrentBlockType, g_iCurrentFrame, 1, FALSE);

        return TRUE;
    }

    return FALSE;
}

BOOL MoveDown (BOOL fDrop)
{
    int nOriginYOld = g_nOriginY;

    if (fDrop)
    {
        while (Move (0, +1))
        {
            g_nOriginY++;
        }
    }
    else
    {
        if (Move (0, +1))
        {
            g_nOriginY++;
            g_nFreeFallIters++;
        }
    }

    if (nOriginYOld == g_nOriginY)
    {
        // Block couldn't move anymore, so freeze it.
        PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, g_iCurrentFrame, 2 + g_btCurrentBlockType, FALSE);

        InvalidateRect (g_hwndMain, NULL, FALSE);
        UpdateWindow (g_hwndMain);

        UpdateScore ();
        UpdateStatusBar ();

        EliminateFullRows ();

        return FALSE;
    }

    InvalidateRect (g_hwndMain, NULL, FALSE);
    UpdateWindow (g_hwndMain);

    return TRUE;
}

BOOL MoveLeft (void)
{
    if (!Move (-1, 0))
        return FALSE;

    g_nOriginX--;

    InvalidateRect (g_hwndMain, NULL, FALSE);
    UpdateWindow (g_hwndMain);

    return TRUE;
}

BOOL MoveRight (void)
{
    if (!Move (+1, 0))
        return FALSE;

    g_nOriginX++;

    InvalidateRect (g_hwndMain, NULL, FALSE);
    UpdateWindow (g_hwndMain);

    return TRUE;
}

BOOL Rotate (void)
{
    int iNextFrame;

    // Get the index of the next block frame.

    iNextFrame = g_iCurrentFrame + 1;
    if (iNextFrame >= g_pFrameCounts[g_btCurrentBlockType])
        iNextFrame = 0;

    // If the new block frame cannot be put at (g_nOriginX, g_nOriginY), don't rotate.

    if (!PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, iNextFrame, 1, TRUE))
        return FALSE;

    // Clear current block position.
    PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, g_iCurrentFrame, 0, FALSE);

    // Put the new block frame in the grid.
    PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, iNextFrame, 1, FALSE);

    g_iCurrentFrame = iNextFrame;

    InvalidateRect (g_hwndMain, NULL, FALSE);
    UpdateWindow (g_hwndMain);

    return TRUE;
}

void NewBlock (void)
{
    int i, j, dx;

    g_nOriginX = 0;
    g_nOriginY = 0;

    g_btCurrentBlockType = RandInt (0, 6);

    g_iCurrentFrame = 0;
    g_nFreeFallIters = 0;
    g_gsGameState = GAMESTATE_PLAY;

    dx = 2;
    if (g_btCurrentBlockType == BLOCKO || g_btCurrentBlockType == BLOCKI)
        dx = 3;

    g_nOriginX = dx;
    g_nOriginY = 0;

    if (!PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, g_iCurrentFrame, 1, TRUE))
    {
        // Cannot spawn a new block: end game.

        KillTimer (g_hwndMain, 1);

        // Let the player know the game is over.

        MessageBoxW (g_hwndMain, L"\tGAME OVER !\t", g_szAppName, MB_OK);

        // See if we have a new high score.

        for (i = 0; i < 10; i++)
        {
            if (g_nScore > g_hsHighScores[i].nScore)
            {
                // Yes, we have a new high score, so ask for a name.
                ShowNewHighScoreDialog ();

                // If user pressed cancel button, don't do anything.
                if (g_szNewHighScoreName[0] == L'\0')
                    break;

                // Insert the new high score in g_hsHighScores.

                for (j = 8; j >= i; j--)
                {
                    SecureZeroMemory (&g_hsHighScores[j + 1], sizeof (HIGHSCORE));
                    CopyMemory (&g_hsHighScores[j + 1], &g_hsHighScores[j], sizeof (HIGHSCORE));
                }

                StringCchCopyW (g_hsHighScores[i].szName, MAX_STRING, g_szNewHighScoreName);
                g_hsHighScores[i].nLevel = g_nLevel;
                g_hsHighScores[i].nScore = g_nScore;

                ShowHighScoresDialog ();

                break;
            }
        }

        NewGame ();

    }
    else
    {
        // Spawn a new block. Blocks are spawned in the second and third top rows and not
		// in two hidden rows above the grid.

        PutBlock (g_nOriginX, g_nOriginY, g_btCurrentBlockType, g_iCurrentFrame, 1, FALSE);

        InvalidateRect (g_hwndMain, NULL, FALSE);
        UpdateWindow (g_hwndMain);

        // Update statistics.

        g_nBlockStatistics[g_btCurrentBlockType]++;

        // Start timer.

        SetTimer (g_hwndMain, 1, GetBlockSpeed (), NULL);
    }
}

int FindFullRow (int iStartRow)
{
    int i, j;

    // Returns the row index of the first full row starting from the bottom.

    for (i = iStartRow; i >= 0; i--)
    {
        // If a cell is 0, it not a full row.

        for (j = 0; j < COLS; j++)
        {
            if (g_bGrid[i][j] == 0)
                break;
        }

        // Yes, it's a full row.

        if (j == COLS)
            return i;
    }

    return -1;
}

BOOL ShiftRow (int iRow)
{
    int i;

    // Shift row iRow one to the right by moving all cells one position to the right.
    // The 'new' cell at the left is set to 0.

    if (g_bGrid[iRow][COLS - 2] == 0)
    {
        // If there is only one cell left in row iRow, set the value of that last cell to 0.

        g_bGrid[iRow][COLS - 1] = 0;

        InvalidateRect (g_hwndMain, NULL, FALSE);
        UpdateWindow (g_hwndMain);

        return FALSE;
    }

    // The row has more than one non-zero cell, so shift all cells one to the right.

    for (i = COLS - 1; i > 0; i--)
    {
        // If there is a cell with value 0, we don't have to move the other cells
        // anymore because they are all set to 0. This test could be omitted but
        // then we have to move all cells.

        if (g_bGrid[iRow][i - 1] == 0)
        {
            break;
        }
        else
        {
            g_bGrid[iRow][i] = g_bGrid[iRow][i - 1];
        }
    }

    g_bGrid[iRow][i] = 0;

    InvalidateRect(g_hwndMain, NULL, FALSE);
    UpdateWindow (g_hwndMain);

    return TRUE;
}

void EliminateFullRows (void)
{
    switch (g_gsGameState)
    {
        case GAMESTATE_PLAY:

            g_iFullRow = FindFullRow (ROWS - 1);
            if (g_iFullRow != -1)
            {
                KillTimer (g_hwndMain, 1);
                SetTimer (g_hwndMain, 1, g_nAnimationSpeed, NULL);

                g_gsGameState = GAMESTATE_ELIMINATEFULLROWS;
            }
            else
            {
                NewBlock ();
            }

            break;

        case GAMESTATE_ELIMINATEFULLROWS:

            if (!ShiftRow (g_iFullRow))
            {
                // Instead of using two for() loops to move cells one row lower, we simply
                // call the MoveMemory() function to move the correct memory chunck.

                MoveMemory (&g_bGrid[1], &g_bGrid[0], (sizeof (BYTE) * COLS) * g_iFullRow);
                SecureZeroMemory (&g_bGrid[0], sizeof (BYTE) * COLS);

                InvalidateRect(g_hwndMain, NULL, FALSE);
                UpdateWindow (g_hwndMain);

                g_nFullRows++;
                UpdateStatusBar ();

                g_iFullRow = FindFullRow (g_iFullRow);
                if (g_iFullRow == -1)
                {
                    NewBlock ();
                }
            }

            break;
    }
}

void SetStatusBarSize (void)
{
    HDC hDC;
    RECT rc;
    SIZE size;
    int pStatusBarParts[4];

    // Put the status bar at the bottom of the main window.

    SendMessage (g_hwndStatusBar, WM_SIZE, 0, 0);

    // Calculate the sizes of the status bar parts. All parts
    // have the same size.

    GetClientRect (g_hwndStatusBar, &rc);

    hDC = GetDC (g_hwndStatusBar);

    GetTextExtentPoint32W (hDC, L"   LABEL:   000000", 18, &size);

    // SB_SETPARTS wants the RIGHT coordinate of the EDGE of the parts, NOT the size of the parts.

    pStatusBarParts[3] = -1;
    pStatusBarParts[2] = rc.right - (int) size.cx;
    pStatusBarParts[1] = pStatusBarParts[2] - (int) size.cx;
    pStatusBarParts[0] = pStatusBarParts[1] - (int) size.cx;

    SendMessage (g_hwndStatusBar, SB_SETPARTS, (WPARAM) 4, (LPARAM) pStatusBarParts);

    ReleaseDC (g_hwndStatusBar, hDC);
}

void UpdateStatusBar (void)
{
    WCHAR szBuffer[MAX_STRING];

    StringCchPrintfW (szBuffer, MAX_STRING, L"   Score:   %d", g_nScore);
    SendMessage (g_hwndStatusBar, SB_SETTEXT, (WPARAM) MAKEWORD (3, 0), (LPARAM) szBuffer);

    StringCchPrintfW (szBuffer, MAX_STRING, L"   Level:   %d", g_nLevel);
    SendMessage (g_hwndStatusBar, SB_SETTEXT, (WPARAM) MAKEWORD (2, 0), (LPARAM) szBuffer);

    StringCchPrintfW (szBuffer, MAX_STRING, L"   Full lines:   %d", g_nFullRows);
    SendMessage (g_hwndStatusBar, SB_SETTEXT, (WPARAM) MAKEWORD (1, 0), (LPARAM) szBuffer);
}

int GetBlockSpeed (void)
{
    // Speed in milliseconds.

    return (int) ((10.0 - g_nLevel) / 20.0 * 1000);
}

void UpdateScore (void)
{
    g_nLevel = (int) ((g_nFullRows - 1) / 10.0);

    // Make sure level is in range [0,9].

    g_nLevel = max(0, g_nLevel);
    g_nLevel = min(g_nLevel, 9);

    g_nScore += 24 + (3 * g_nLevel) - g_nFreeFallIters;
}

int RandInt (int iMin, int iMax)
{
    unsigned int i;

    rand_s (&i);

    // Return an integer in the range [iMin, iMax].

    return (int) round (iMin + ((double) i / UINT_MAX * (iMax - iMin)));
}

BOOL GetDataFileName (LPWSTR lpFilename)
{
    DWORD i, dwSize;

    // If the buffer is too small, GetModuleFileNameW () truncates the path,
    // so we end up with an invalid path but we don't care about that: we
    // simply assume there is no file.

    dwSize = GetModuleFileNameW (NULL, lpFilename, MAX_PATH);
    if (dwSize == 0)
    {
        return FALSE;
    }

    // For safety reasons.
    lpFilename[MAX_PATH - 1] = L'\0';

    for (i = dwSize - 1; i > 0; i--)
    {
        if (lpFilename[i] == L'.')
        {
            lpFilename[i + 1] = L'\0';
            break;
        }
    }

    StringCchCat (lpFilename, MAX_PATH, L"dat");

    return TRUE;
}

void LoadData (void)
{
    HANDLE hFile;
    WCHAR szPath[MAX_PATH];
    DWORD i, dwBytesRead;
    BYTE bSettings = 0;

    // If the file cannot be opened or an error occurs, don't do anything.

    if (!GetDataFileName (szPath))
    {
        return;
    }

    hFile = CreateFileW (szPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (ReadFile (hFile, (LPVOID) &bSettings, sizeof (BYTE), &dwBytesRead, NULL))
        {
            // Bit 1 & 2 : color scheme

            g_cteColorTableEntries = (int) (bSettings & 3);
            if (g_cteColorTableEntries > 2)
                g_cteColorTableEntries = 0;

            for (i = 0; i < 10; i++)
            {
                    if (!ReadFile (hFile, (LPVOID) g_hsHighScores[i].szName, MAX_STRING * sizeof (WCHAR), &dwBytesRead, NULL))
                    {
                        break;
                    }

                    if (!ReadFile (hFile, (LPVOID) &g_hsHighScores[i].nLevel, sizeof (int), &dwBytesRead, NULL))
                    {
                        break;
                    }

                    if (!ReadFile (hFile, (LPVOID) &g_hsHighScores[i].nScore, sizeof (int), &dwBytesRead, NULL))
                    {
                        break;
                    }
            }
        }

        CloseHandle (hFile);
    }
}

void SaveData (void)
{
    HANDLE hFile;
    DWORD i, dwBytesWritten;
    WCHAR szPath[MAX_PATH];
    BYTE bSettings;

    // If the file cannot be created or an error occurs, don't do anything.

    if (!GetDataFileName (szPath))
    {
        return;
    }

    hFile = CreateFileW (szPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // Bit 1 & 2 : color scheme

        bSettings = (BYTE) g_cteColorTableEntries;

        if (WriteFile (hFile, (LPVOID) &bSettings, sizeof (BYTE), &dwBytesWritten, NULL))
        {
            for (i = 0; i < 10; i++)
            {
                    if (!WriteFile (hFile, (LPVOID) g_hsHighScores[i].szName, MAX_STRING * sizeof (WCHAR), &dwBytesWritten, NULL))
                    {
                        break;
                    }

                    if (!WriteFile (hFile, (LPVOID) &g_hsHighScores[i].nLevel, sizeof (int), &dwBytesWritten, NULL))
                    {
                        break;
                    }

                    if (!WriteFile (hFile, (LPVOID) &g_hsHighScores[i].nScore, sizeof (int), &dwBytesWritten, NULL))
                    {
                        break;
                    }
            }
        }

        CloseHandle (hFile);
    }
}

LOCAL void Pause (BOOL fGamePaused)
{
    g_fGamePaused = fGamePaused;

    if (g_fGamePaused)
    {
        KillTimer (g_hwndMain, 1);
    }
    else
    {
        switch (g_gsGameState)
        {
            case GAMESTATE_PLAY:

                SetTimer (g_hwndMain, 1, GetBlockSpeed (), NULL);

                break;

            case GAMESTATE_ELIMINATEFULLROWS:

                SetTimer (g_hwndMain, 1, g_nAnimationSpeed, NULL);

                break;
        }
    }
}

LOCAL void SetColorTableEntries (COLORTABLEENTRIES cte)
{
    int i;

    g_cteColorTableEntries = cte;

    switch (cte)
    {
        case COLORTABLE_MONOCHROME:

            for (i = 0; i < 7; i++)
            {
                g_crColorTable[i] = RGB (255,255,0);
            }

            break;

        case COLORTABLE_BICOLOR:

            g_crColorTable[0] = RGB (255,255,0);
            g_crColorTable[1] = RGB (255,255,0);
            g_crColorTable[2] = RGB (255,255,0);
            g_crColorTable[3] = RGB (255,255,0);
            g_crColorTable[4] = RGB (0,255,255);
            g_crColorTable[5] = RGB (0,255,255);
            g_crColorTable[6] = RGB (0,255,255);

            break;

        case COLORTABLE_MULTICOLOR:

            g_crColorTable[0] = RGB (255,0,0);
            g_crColorTable[1] = RGB (0,255,0);
            g_crColorTable[2] = RGB (0,0,255);
            g_crColorTable[3] = RGB (255,255,0);
            g_crColorTable[4] = RGB (0,255,255);
            g_crColorTable[5] = RGB (255,0,255);
            g_crColorTable[6] = RGB (255,255,255);

            break;
    }

    InvalidateRect(g_hwndMain, NULL, FALSE);
    UpdateWindow (g_hwndMain);
}

