#include <windows.h>
#include <strsafe.h>
#include "main.h"

//Problemy jakie napotkalem
//1. Wybor interfejsu, graficzny SDL, graficzny GDI czy moze konsola tektowa
//2. Migitanie (flickering) animowanych elementow graficznych
//3. Opoznienie powtarzania wcisnietego klawisza co utrudnia sterwanie gra
//4. Pozycjonowanie wyswietlanego wyniku, trzeba pobrac wymiary renderowanego tekstu

//zmienna petli glownej okna
int gameRunning = 1; 

int WINAPI WinMain(HINSTANCE currInstance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
	//nazwa klasy okna
	LPCWSTR CLASS_NAME = L"MyGameWindowClass";

	//definicja pustej klasy okna
	WNDCLASS klasa_okna = { 0 };

	/*
		typedef struct tagWNDCLASSA {
		  UINT      style;
		  WNDPROC   lpfnWndProc;
		  int       cbClsExtra;
		  int       cbWndExtra;
		  HINSTANCE hInstance;
		  HICON     hIcon;
		  HCURSOR   hCursor;
		  HBRUSH    hbrBackground;
		  LPCSTR    lpszMenuName;
		  LPCSTR    lpszClassName;
		}
	*/

	//Ustawienie parametrow klasy okna
	klasa_okna.lpfnWndProc = WindowProc;
	klasa_okna.hInstance = currInstance;
	klasa_okna.lpszClassName = CLASS_NAME;
	klasa_okna.hCursor = LoadCursor(0, IDC_ARROW);
	klasa_okna.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	//Rejestrowanie klasy okna
	RegisterClass(&klasa_okna);

	/*
		HWND CreateWindowExW(
		  DWORD     dwExStyle,
		  LPCWSTR   lpClassName,
		  LPCWSTR   lpWindowName,
		  DWORD     dwStyle,
		  int       X,
		  int       Y,
		  int       nWidth,
		  int       nHeight,
		  HWND      hWndParent,
		  HMENU     hMenu,
		  HINSTANCE hInstance,
		  LPVOID    lpParam
		);
	*/

	//Tworzenie okna
	HWND window = CreateWindowEx(0,
		CLASS_NAME,
		L"Ping Pong - wynik 0:0", //L oznacza string WideChar
		//stala wielkosc okna, bez mozliwosci zmiany wielkosci, resize disabled
		(WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_VISIBLE) - (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME),
		CW_USEDEFAULT, //domyslne polozenie okna przy tworzeniu, mozna ustawic na strodku: (GetSystemMetrics(SM_CXSCREEN) / 2) - (WINDOW_WIDTH / 2),
		CW_USEDEFAULT, //domyslne polozenie okna przy tworzeniu, mozna ustawic na strodku: (GetSystemMetrics(SM_CYSCREEN) / 2) - (WINDOW_HEIGHT / 2),
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		0,
		0,
		currInstance,
		0
		);

	//jesli okno sie nie stworzylo wyjdz z funkcji
	if (!window) return 0;

	//inicjalizacja generatora liczb losowych
	//bedzie potrzebny do wyznaczanie losowego kierunku lotu pilki przy starcie gry
	//DWORD GetTickCount() The return value is the number of milliseconds that have elapsed since the system was started.
	srand(GetTickCount());

	//wystartowanie timera ktory bedzie wyznaczal czestotliwosc odswierzania gry
	/*
		UINT_PTR SetTimer(
		  HWND      hWnd,
		  UINT_PTR  nIDEvent,
		  UINT      uElapse,
		  TIMERPROC lpTimerFunc
		);
	*/
	//ustawiony na 10 ms
	//lpTimerFunc jest null poniewaz bedziemy obslugiwac to zdarzenie w WM_TIMER
	SetTimer(window, 1, 5, NULL);

	//
	whitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	blackBrush = CreateSolidBrush(RGB(0, 0, 0));

	
	

	//petla glowna okna, pobieranie wiadomosci z kolejki i rzekazywanie ich do procedury okna: WindowProc
	/*
		BOOL PeekMessage(
		  LPMSG lpMsg,
		  HWND  hWnd,
		  UINT  wMsgFilterMin,
		  UINT  wMsgFilterMax,
		  UINT  wRemoveMsg
		);
	*/
	while (gameRunning)
	{
		MSG message;
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) //PM_REMOVE usuwa pobrana wiadomosc z kolejki
		{
			TranslateMessage(&message); //Translates virtual-key messages into character messages
			DispatchMessage(&message); //wywoluje WindowProc przekazujac message

		}

		InvalidateRect(window, NULL, 1);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------

/*
	LRESULT Wndproc(
	  HWND unnamedParam1,
	  UINT unnamedParam2,
	  WPARAM unnamedParam3,
	  LPARAM unnamedParam4
	)
*/



LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	HDC hDC;
	PAINTSTRUCT ps;
	HDC         hdcMem;
	HBITMAP     hbmMem;
	HANDLE      hOld;

	switch (message)
	{
		case WM_CLOSE:
		{
			gameRunning = 0;
		} break;

		case WM_TIMER:
		{
			CHAR tmp_string[256];

			swprintf_s(tmp_string, 128, L"Ping Pong - wynik %d:%d", gracz1wynik, gracz2wynik);
			SetWindowText(windowHandle, tmp_string);

			przesun_pilke();

			//Ponizsza metoda parsowania klawiszy eliminuje problem opoznien odczytywania klawiszy przez system
			if (GetAsyncKeyState(VK_UP))
			{
				if (slupek_gracz2_y - 45 > 0) slupek_gracz2_y -= PREDKOSC_PRZESUWANIA_PADOW;
			}

			if (GetAsyncKeyState(VK_DOWN))
			{
				if (slupek_gracz2_y + 65 < WINDOW_HEIGHT - WYSOKOSC_PASKA_NAGLOWKA_OKNA) slupek_gracz2_y += PREDKOSC_PRZESUWANIA_PADOW;
			}

			if (GetAsyncKeyState('A'))
			{
				if (slupek_gracz1_y - 45 > 0) slupek_gracz1_y -= PREDKOSC_PRZESUWANIA_PADOW;
			}

			if (GetAsyncKeyState('Z'))
			{
				if (slupek_gracz1_y + 65 < WINDOW_HEIGHT - WYSOKOSC_PASKA_NAGLOWKA_OKNA) slupek_gracz1_y += PREDKOSC_PRZESUWANIA_PADOW;
			}

		} break;

		
		case WM_PAINT:
		{
			//Problem migotania zostal rozwiazany uzywaja metody opisanej na tej stronie
			//http://www.catch22.net/tuts/win32/flicker-free-drawing#

			hDC = BeginPaint(windowHandle, &ps);

			//pobranie wymiarow okna
			//RECT wymiaryOkna;
			//GetClientRect(windowHandle, &wymiaryOkna);

			//Create an off-screen DC for double-buffering
			hdcMem = CreateCompatibleDC(hDC);
			hbmMem = CreateCompatibleBitmap(hDC, WINDOW_WIDTH, WINDOW_HEIGHT);

			hOld = SelectObject(hdcMem, hbmMem);

			/*
			BOOL GetTextExtentPoint32A(
				  HDC    hdc,
				  LPCSTR lpString,
				  int    c,
				  LPSIZE psizl
				);
			*/
			//rysowanie wyniku
			HFONT hFont = CreateFont(80, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"SYSTEM_FIXED_FONT");
			HFONT hTmp = (HFONT)SelectObject(hdcMem, hFont);
			CHAR tmp_string[256];
			SIZE fontSize;
			swprintf_s(tmp_string, 128, L"%d", gracz1wynik);
			//pobieranie wymiaru tekstu w pikselach w celu poprawnego pozycjonowania
			GetTextExtentPoint(hdcMem, tmp_string, wcslen(tmp_string), &fontSize);
			TextOut(hdcMem, WINDOW_WIDTH / 2 - fontSize.cx - 3, 10, tmp_string, wcslen(tmp_string));

			swprintf_s(tmp_string, 128, L"%d", gracz2wynik);
			//GetTextExtentPoint(hdcMem, tmp_string, wcslen(tmp_string), &fontSize);
			TextOut(hdcMem, WINDOW_WIDTH / 2 + 3, 10, tmp_string, wcslen(tmp_string));

			DeleteObject(SelectObject(hdcMem, hTmp));

			//rysowanie padow graczy
			Rectangle(hdcMem, slupek_gracz1_x - 5, slupek_gracz1_y - 40, slupek_gracz1_x + 5, slupek_gracz1_y + 40);
			Rectangle(hdcMem, slupek_gracz2_x - 5, slupek_gracz2_y - 40, slupek_gracz2_x + 5, slupek_gracz2_y + 40);

			//rysowanie pilki
			Ellipse(hdcMem, pilkaX - PILKA_PROMIEN, pilkaY - PILKA_PROMIEN, pilkaX + PILKA_PROMIEN, pilkaY + PILKA_PROMIEN);

			//kopiowanie bufora do pamieci ekranu
			BitBlt(hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdcMem, 0, 0, SRCCOPY);

			//SelectObject(hDC, whitePen);
			//SelectObject(hDC, blackBrush);
			//Ellipse(hDC, pilkaX - PILKA_PROMIEN, pilkaY - PILKA_PROMIEN, pilkaX + PILKA_PROMIEN, pilkaY + PILKA_PROMIEN);

			DeleteObject(hbmMem);
			DeleteDC(hdcMem);
			EndPaint(windowHandle, &ps);
		} break;



		case WM_KEYDOWN:
		{
			//wcisnieta spacaja, wystartuj gre
			if (wparam == VK_SPACE)
			{
				//jesli predkosc pilki jest 0 to znaczy ze pilka jest na srodku, nadaj jej ruch
				if ((pilkaPredkoscX == 0) & (pilkaPredkoscY == 0))
				{
					if ((rand() & 0x01) == 0)
						pilkaPredkoscX = -pilkaWartoscPredkosci;
					else
						pilkaPredkoscX = pilkaWartoscPredkosci;

					if ((rand() & 0x01) == 0)
						pilkaPredkoscY = -pilkaWartoscPredkosci;
					else
						pilkaPredkoscY = pilkaWartoscPredkosci;
				}

			}

			/*
			//Ponizsza metoda parsowania klawiszy nie jest efektywna, duze opoznienie w odczycie stanu klawisza i jego powtorzeniach
			if (wparam == VK_UP)
			{
				if (slupek_gracz2_y - 45 > 0) slupek_gracz2_y -= PREDKOSC_PRZESUWANIA_PADOW;
			}

			if (wparam == VK_DOWN)
			{
				if (slupek_gracz2_y + 65 < WINDOW_HEIGHT - WYSOKOSC_PASKA_NAGLOWKA_OKNA) slupek_gracz2_y += PREDKOSC_PRZESUWANIA_PADOW;
			}

			if (wparam == 'A')
			{
				if (slupek_gracz1_y - 45 > 0) slupek_gracz1_y -= PREDKOSC_PRZESUWANIA_PADOW;
			}

			if (wparam == 'Z')
			{
				if (slupek_gracz1_y + 65 < WINDOW_HEIGHT - WYSOKOSC_PASKA_NAGLOWKA_OKNA) slupek_gracz1_y += PREDKOSC_PRZESUWANIA_PADOW;
			}
			*/
		} break;

		//patrz efekt migotania
		case WM_ERASEBKGND:
			return 1;

		default:
		{
			//wywolanie domyslnej funkcji obslugi wiadomosci
			result = DefWindowProc(windowHandle, message, wparam, lparam);
		} break;
	}

	return result;
}


void przesun_pilke()
{
	if (pilkaX + pilkaPredkoscX >= WINDOW_WIDTH)
	{
		gracz1wynik++;
		pilkaX = PILKA_POLOZENIE_STARTOWE_X;
		pilkaY = PILKA_POLOZENIE_STARTOWE_Y;
		pilkaPredkoscX = 0;
		pilkaPredkoscY = 0;
	}

	if (pilkaX + pilkaPredkoscX <= 0)
	{
		gracz2wynik++;
		pilkaX = PILKA_POLOZENIE_STARTOWE_X;
		pilkaY = PILKA_POLOZENIE_STARTOWE_Y;
		pilkaPredkoscX = 0;
		pilkaPredkoscY = 0;
	}

	if (pilkaX + pilkaPredkoscX >= slupek_gracz2_x - 6 && (pilkaY <= slupek_gracz2_y + 40 && pilkaY >= slupek_gracz2_y - 40))
		pilkaPredkoscX = -pilkaPredkoscX;

	if (pilkaX + pilkaPredkoscX <= slupek_gracz1_x + 6 && (pilkaY <= slupek_gracz1_y + 40 && pilkaY >= slupek_gracz1_y - 40))
		pilkaPredkoscX = -pilkaPredkoscX;

	pilkaX += pilkaPredkoscX;

	if (pilkaY + pilkaPredkoscY < WINDOW_HEIGHT - 20 && pilkaY + pilkaPredkoscY > 0)
		pilkaY += pilkaPredkoscY;
	else
	{
		pilkaPredkoscY = -pilkaPredkoscY;
		pilkaY += pilkaPredkoscY;
	}
}