// TicTacToe.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TicTacToe.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground = (HBRUSH)(GetStockObject(GRAY_BRUSH));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TICTACTOE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

//Global Variables for our game
const int CELL_SIZE = 100;
HBRUSH hbr1, hbr2;
HICON hIcon1, hIcon2;
int playerTurn = 1;
int gameBoard[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int winner = 0;
int wins[3];

BOOL GetGameBoardRect(HWND hwnd, RECT* pRect) {
    RECT rc;
    if (GetClientRect(hwnd, &rc)) {

        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        pRect->left = (width - CELL_SIZE * 3) / 2;// how to center
        pRect->top = (height - CELL_SIZE * 3) / 2;

        pRect->right = pRect-> left + CELL_SIZE * 3;
        pRect->bottom = pRect-> top + CELL_SIZE * 3;
        return TRUE;
    }

    SetRectEmpty(pRect);
        return FALSE;

}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2) {
     MoveToEx(hdc, x1, y1, NULL);
     LineTo(hdc, x2, y2); //Draws Basic Line in established Positions
}

int GetCellNumberFromPoint(HWND hwnd, int x, int y) {
    RECT rc;
    POINT pt = { x , y } ;

    if (GetGameBoardRect(hwnd, &rc)) {
        if (PtInRect(&rc, pt)) {
            //user clicked inside gameboard
            //Normalize (0 to 3 * CELL_SIZE)
            x = pt.x - rc.left;
            y = pt.y - rc.top;

            int column = x / CELL_SIZE;
            int row = y / CELL_SIZE;

            //convert to index ( 0 to 8)
            return column + row * 3;
        }
    }
    return -1; // outside of game board
}

BOOL GetCellRect(HWND hWnd, int index, RECT * pRect) {
    RECT rcBoard;

    SetRectEmpty(pRect);// checking parameters
    if (index < 0 || index > 8)
        return FALSE;

    if (GetGameBoardRect(hWnd, &rcBoard)) 
    {
        //Convert Index from 0 to 8 into x,y pair
        int y = index / 3; // Row number
        int x = index % 3; // Column number

        pRect->left = rcBoard.left + x * CELL_SIZE + 1;
        pRect->top = rcBoard.top + y * CELL_SIZE + 1;

        pRect->right  = pRect->left + CELL_SIZE - 1;
        pRect->bottom = pRect->top + CELL_SIZE - 1;

        return TRUE;

    }
    return FALSE;
}
// will return the winning 3 cells 
// 0 if no winner
// 1 if player 1 wins
// 2 if player 2 wins
// 3 if a draw
/*  Number layout
    0,1,2
    3,4,5
    6,7,8
*/

int GetWinner(int wins[3])
{
    int cells[] = { 0,1,2, 3,4,5, 6,7,8,  0,3,6, 1,4,7, 2,5,8,  0,4,8, 2,4,6 };

    for (int i = 0; i < ARRAYSIZE(cells); i += 3)
    {
        if ((0 != gameBoard[cells[i]]) && gameBoard[cells[i]] == gameBoard[cells[i + 1]] && gameBoard[cells[i]] == gameBoard[cells[i + 2]]) {
            // we have a winner
            wins[0] = cells[i];
            wins[1] = cells[i + 1];
            wins[2] = cells[i + 2];

            return gameBoard[cells[i]];
        }
    }
    
    // lets check for any empty cells next
    for (int i = 0; i < ARRAYSIZE(gameBoard); ++i)    
        if (0 == gameBoard[i]) 
            return 0;// lets us continue to play
            
    return 3;// it's a draw
}

void ShowTurn(HWND hwnd, HDC hdc) 
{
    const WCHAR szTurn1[] = L"Turn: Player 1";
    const WCHAR szTurn2[] = L"Turn: Player 2";

    const WCHAR* pszTurnText = NULL;

    switch (winner) 
    {
    case 0: //Continue to play
        pszTurnText = (playerTurn == 1 )? szTurn1 : szTurn2;
        break;
    case 1: //Player 1 wins
        pszTurnText = L"Player 1 is the winner!";
        break;
    case 2: //Player 2 wins
        pszTurnText = L"Player 2 is the winner!";
        break;
    case 3: //It's a draw 
        pszTurnText = L"It's a draw!";
        break;
    }
    RECT rc;

    if (NULL != pszTurnText && GetClientRect(hwnd, &rc)) 
    {
        rc.top = rc.bottom - 48;
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, pszTurnText, lstrlen(pszTurnText), &rc, DT_CENTER);
    }
}
void DrawIconCentered(HDC hdc,RECT * pRect, HICON hIcon) 
{
    const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);// gets fixed icon width and height
    const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);
    if (NULL != pRect)
    {
        int left = pRect-> left + ((pRect->right - pRect->left) - 32) / 2;
        int top = pRect -> top + ((pRect->bottom - pRect -> top)- 32) / 2;
        DrawIcon(hdc, left, top, hIcon);
    }
}
void ShowWinner (HWND hWnd, HDC hdc)
{
    RECT rcWin;
    for (int i = 0; i < 3; ++i) {
        if (GetCellRect(hWnd, wins[i], &rcWin))
        {
            FillRect(hdc, &rcWin, (winner == 1) ? hbr1 : hbr2);
            DrawIconCentered(hdc, &rcWin, (winner == 1) ? hIcon1 : hIcon2);
        }
    }
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    switch (message)
    {
    case WM_CREATE:
    {
        hbr1 = CreateSolidBrush(RGB(255, 0, 0));
        hbr2 = CreateSolidBrush(RGB(0, 0, 255));

        //Load player icons
        hIcon1 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER1));
        hIcon2 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PLAYER2));
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_NEWGAME: {// is initialized in TicTacToe.rc
                int ret = MessageBox(hWnd, L"Are you sure you want to start a new game?",
                    L"New Game", MB_YESNO | MB_ICONQUESTION);
                if (IDYES == ret) {
                    // reset and start a new game
                    playerTurn = 1;
                    winner = 0;
                    ZeroMemory(gameBoard, sizeof(gameBoard));
                    // Force a paint message
                    InvalidateRect(hWnd, NULL, TRUE); // Post a WM_PAINT  to our windowProc. It gets queued in our msg queue
                    UpdateWindow(hWnd); // forces immediate handling of WM_Paint
                }
            }
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_LBUTTONDOWN: {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        // Only handle clicks if it is a player turn i.e. 1 or 2
   
        int index = GetCellNumberFromPoint(hWnd, xPos, yPos);

        HDC hdc = GetDC(hWnd);
        if (NULL != hdc) {
            //WCHAR temp[100];

            //Prints the Indexes to the screen 
            //wsprintf(temp, L"Index = %d", index);
            //TextOut(hdc, xPos, yPos, temp, lstrlen(temp));

            //Get cell dimension from it's index
            if (index != -1) 
            {
                RECT rcCell;
                if (0 == gameBoard[index] && GetCellRect(hWnd, index, &rcCell)) 
                {
                    gameBoard[index] = playerTurn;// saves the next players turn

                    //FillRect(hdc, &rcCell, (playerTurn == 1) ? hbr1 : hbr2); the square coloring
                    DrawIconCentered(hdc, &rcCell, (playerTurn == 1) ? hIcon1 : hIcon2);

                    //Check for a winner
                    winner = GetWinner(wins);
                    if (winner == 1 || winner == 2) 
                    {
                        ShowWinner(hWnd, hdc);
                        
                        // we have a winner !
                        MessageBox(hWnd, 
                            (winner == 1) ? L"Player 1 is the winner" : L"Player 2 is the winner!", L"You Win!",
                            MB_OK | MB_ICONINFORMATION);
                        playerTurn = 0;
                    }
                    else if (3 == winner)
                    {
                        // It's a draw!
                        MessageBox(hWnd,
                            L"No one wins this time !",
                            L"It's a draw!",
                            MB_OK | MB_ICONEXCLAMATION);
                        playerTurn = 0;
                    }
                    else if (0 == winner) 
                    {
                        playerTurn = (playerTurn == 1) ? 2 : 1;
                    }
                    //Display Turn
                    ShowTurn(hWnd, hdc);
                }
                
            }
            ReleaseDC(hWnd, hdc);
        }

    }
    case WM_GETMINMAXINFO:// failed once index values were installed
        {
        MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;// This restricts the Minimum
        //pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;//amount the window can be resized
        //pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;

        }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        RECT rc;

        if (GetGameBoardRect(hWnd, &rc)) {
        //Draw Player 1 and Player 2 text

            RECT rcClient;

            // display player text and tuen
            if (GetClientRect(hWnd, &rcClient)) {
                // compiler will auto assign the size via the constant unicode
                const WCHAR szPlayer1 []= L"Player 1";
                const WCHAR szPlayer2 []= L"Player 2";

                //SetBkColor(hdc, RGB(128, 128, 128)); grey
                SetBkMode(hdc, TRANSPARENT);

                // Draw Player 1 and Player 2 text
                // use lstrlen but we can get away with ARRAYSIZE but dont use on charcter pointers
                SetTextColor(hdc, RGB(255, 255, 0));
                TextOut(hdc, 16, 16, szPlayer1, ARRAYSIZE(szPlayer1));
                DrawIcon(hdc, 24, 40, hIcon1);
                
                SetTextColor(hdc, RGB(0, 0, 255));
                TextOut(hdc, rcClient.right - 72, 16, szPlayer2, ARRAYSIZE(szPlayer2));
                DrawIcon(hdc, rcClient.right - 64, 40, hIcon2);

                ShowTurn(hWnd, hdc);
            }
            
            // Draw Game Board
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));// This allows the removal of the black border of rect
            //Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom); // removed because it made a 1 pixel offf extra line

             //Rectangle(hdc, 0, 0, 100, 100);// will display a rectangle in the corner "0,0"

            for (int i = 0; i < 4; ++i) {
                //Draw Vertical Lines
                DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);

                //Draw Horizontal Lines
                DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right, rc.top + CELL_SIZE * i);
            }

            // Draw all occupied cells
            RECT rcCell;
            for (int i = 0; i < ARRAYSIZE(gameBoard); ++i) {              
                if (0 != gameBoard[i] && GetCellRect(hWnd, i, &rcCell))
                {
                    //FillRect(hdc, &rcCell, (gameBoard[i] == 2) ? hbr2 : hbr1);                    
                    DrawIconCentered(hdc, &rcCell, (gameBoard[i] == 1) ? hIcon1 : hIcon2);
                }
            }
            //Show winner
            if(winner == 1 || winner == 2)
                ShowWinner(hWnd, hdc);
        }
            EndPaint(hWnd, &ps);
        }

        break;
    case WM_DESTROY:
        DeleteObject(hbr1);
        DeleteObject(hbr2);
        //dispose of incon images
        DestroyIcon(hIcon1);
        DestroyIcon(hIcon2);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/*
Things to check again

1. Window minimum

Possible upgrades,
make A.I. using randomizer, center square choice & three of a row
can make a button indication or player 2 being a cpu option

get rid of the "About Tic Tac Toe"

*/