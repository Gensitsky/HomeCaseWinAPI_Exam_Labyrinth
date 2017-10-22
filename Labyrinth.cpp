#include "stdafx.h"
#include "Labyrinth.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LABYRINTH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABYRINTH));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABYRINTH));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_LABYRINTH);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 600, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

const int SQUARE_SIDE = 50;
const int LABYRINTH_HEIGHT = 8;
const int LABYRINTH_WIDTH = 12;

int LABYRINTH[LABYRINTH_HEIGHT][LABYRINTH_WIDTH] =
{
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1 },
	{ 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1 },
	{ 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1 },
	{ 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1 },
	{ 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1 },
	{ 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1 }
};
int MAN_X = 4, MAN_Y = 4;

enum CellState { CELL_FREE = 0, CELL_BRICK = 1, CELL_FOUND = 2, CELL_PROCESSED = 3};

struct Cell
{
	int x, y;
	CellState state;
};

Cell PATH[LABYRINTH_HEIGHT * LABYRINTH_WIDTH * 4];
int NEXT_PATH_INDEX = 0;

bool FindPath(Cell currentCell)
{
	LABYRINTH[currentCell.y][currentCell.x] = CELL_FOUND;
	currentCell.state = CELL_FOUND;

	PATH[NEXT_PATH_INDEX++] = currentCell;

	if (currentCell.x == 0 || currentCell.x == LABYRINTH_WIDTH - 1 ||
		currentCell.y == 0 || currentCell.y == LABYRINTH_HEIGHT - 1)
	{
		return true;
	}

	static int dx[4] = {0, -1,  0, 1};
	static int dy[4] = {1,  0, -1, 0};

	bool cellIsAddedToPath = false;

	for (int i = 0; i < 4; ++i)
	{
		Cell neightbourCell = { currentCell.x + dx[i], currentCell.y + dy[i] };

		if (neightbourCell.x >= 0 && neightbourCell.x < LABYRINTH_WIDTH &&
			neightbourCell.y >= 0 && neightbourCell.y < LABYRINTH_HEIGHT &&
			LABYRINTH[neightbourCell.y][neightbourCell.x] == CELL_FREE)
		{
			bool exitFound = FindPath(neightbourCell);
			if (exitFound)
				return true;
			PATH[NEXT_PATH_INDEX++] = currentCell;
			cellIsAddedToPath = true;

		}
	}

	LABYRINTH[currentCell.y][currentCell.x] = CELL_PROCESSED;
	currentCell.state = CELL_PROCESSED;
	
	if (cellIsAddedToPath)
		PATH[NEXT_PATH_INDEX - 1] = currentCell;
	else
		PATH[NEXT_PATH_INDEX++] = currentCell;

	return false;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static HBITMAP hbitmapBrick, hbitmapMan;

	switch (message)
	{
	case WM_CREATE:
		{
			hbitmapBrick = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BRICK));
			if (hbitmapBrick == NULL)
				MessageBox(hWnd, "Bitmap with bricks failed to boot ", " Error!", MB_OK | MB_ICONEXCLAMATION);
			hbitmapMan = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MAN));
			if (hbitmapMan == NULL)
				MessageBox(hWnd, "Bitmap with smiley did not boot ", " Error!", MB_OK | MB_ICONEXCLAMATION);
		
			Cell initCell = { MAN_X, MAN_Y, CELL_FREE};
			bool exitFound = FindPath(initCell);

			for (int y = 0; y < LABYRINTH_HEIGHT; ++y)
				for (int x = 0; x < LABYRINTH_WIDTH; ++x)
				{
					if (LABYRINTH[y][x] != CELL_BRICK)
						LABYRINTH[y][x] = CELL_FREE;
				}
			SetTimer(hWnd, 5555, 750, NULL);

		}
		break;
	case WM_TIMER:
		{
			if (wParam == 5555)
			{
				static int stepNumber = 0;
				if (stepNumber < NEXT_PATH_INDEX)
				{
					MAN_X = PATH[stepNumber].x;
					MAN_Y = PATH[stepNumber].y;
					LABYRINTH[MAN_Y][MAN_X] = PATH[stepNumber].state;

					stepNumber++;
					
					InvalidateRect(hWnd, NULL, true);
				}
				else
				{
					KillTimer(hWnd, 5555);
				}
			}
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}	
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);

			HDC hdcBrick = CreateCompatibleDC(hdc);
			HBITMAP hbmOldBrick = (HBITMAP)SelectObject(hdcBrick, hbitmapBrick);
			HDC hdcMan = CreateCompatibleDC(hdc);
			HBITMAP hbmOldMan = (HBITMAP)SelectObject(hdcMan, hbitmapMan);

			HBRUSH foundCellBrush = CreateSolidBrush(RGB(200, 200, 200));
			HBRUSH processedCellBrush = CreateSolidBrush(RGB(50, 50, 50));

			for (int y = 0; y < LABYRINTH_HEIGHT; ++y)
			for (int x = 0; x < LABYRINTH_WIDTH; ++x)
			{
				if (x == MAN_X && y == MAN_Y)
				{
					BitBlt(hdc, (x + 1)*SQUARE_SIDE, (y + 1)*SQUARE_SIDE, SQUARE_SIDE, SQUARE_SIDE, hdcMan, 0, 0, SRCCOPY);
				}
				else if (LABYRINTH[y][x] == CELL_BRICK)
					BitBlt(hdc, (x+1)*SQUARE_SIDE, (y+1)*SQUARE_SIDE, SQUARE_SIDE, SQUARE_SIDE, hdcBrick, 0, 0, SRCCOPY);
				else if (LABYRINTH[y][x] == CELL_FOUND)
				{
					SelectObject(hdc, foundCellBrush);
					Rectangle(hdc, (x + 1)*SQUARE_SIDE, (y + 1)*SQUARE_SIDE, (x + 2)*SQUARE_SIDE, (y + 2)*SQUARE_SIDE);
				}
				else if (LABYRINTH[y][x] == CELL_PROCESSED)
				{
					SelectObject(hdc, processedCellBrush);
					Rectangle(hdc, (x + 1)*SQUARE_SIDE, (y + 1)*SQUARE_SIDE, (x + 2)*SQUARE_SIDE, (y + 2)*SQUARE_SIDE);
				}

			}

			BitBlt(hdc, 200, 200, SQUARE_SIDE, SQUARE_SIDE, hdcBrick, 0, 0, SRCCOPY);

			//Rectangle(hdc, 10, 10, 100, 200);

			SelectObject(hdcBrick, hbmOldBrick);
			SelectObject(hdcMan, hbmOldMan);
			DeleteDC(hdcBrick);
			DeleteDC(hdcMan);
			DeleteObject(foundCellBrush);
			DeleteObject(processedCellBrush);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

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