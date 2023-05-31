#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <tchar.h>
#include <gdiplus.h>
#include <assert.h>


#define SIZE_OF_MAP 9 // 地图大小
int mineCount  = 10; // 地雷个数
// 地图结构体
typedef struct
{
	unsigned __int8 isMine;					// 是否是地雷
	unsigned __int8 count;					// 周围地雷数目
	unsigned __int8 isSweeped;				// 是否被扫过
	unsigned __int8 isSigned;
} MAP;
// 坐标结构体
typedef struct
{
	unsigned __int8 x;
	unsigned __int8 y;
} COORDINATE;


MAP map[SIZE_OF_MAP][SIZE_OF_MAP];			// 定义地图数组
__int16 rest;                       //剩余的安全位置，为0则游戏胜利
__int16 signRest;
HDC hdc, hMemdc;
HBITMAP bg;
char str[2][5];
HFONT hFont, hFont1;
int ifFirst = 1;
// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

COORDINATE input = {0, 0};

//声明函数
void GotoXY(COORDINATE coord);
void GameInit(HWND hWnd);
void SetMine(COORDINATE coordinate);
void MapInit(COORDINATE coordinate);
void ShowMap();
COORDINATE InputCoord();
int Sweep(COORDINATE coord);
void fail(COORDINATE coord, HWND hWnd);
void Win(HWND hWnd);
void CountMine();
void SetSweeped(COORDINATE coord);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Int2Char(int num, char* str);
COORDINATE Getclick(int pixelX, int pixelY, HWND hWnd);
void SignMine(COORDINATE coordinate);
void ShowInfo(HWND hWnd);

int WINAPI WinMain(
   _In_ HINSTANCE hInstance,
   _In_opt_ HINSTANCE hPrevInstance,
   _In_ LPSTR     lpCmdLine,
   _In_ int       nCmdShow
)
{

   WNDCLASSEX wcex;

   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style          = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc    = WndProc;
   wcex.cbClsExtra     = 0;
   wcex.cbWndExtra     = 0;
   wcex.hInstance      = hInstance;
   wcex.hIcon          = LoadIcon(wcex.hInstance, IDI_APPLICATION);
   wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
   wcex.lpszMenuName   = NULL;
   wcex.lpszClassName  = szWindowClass;
   wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);

   if (!RegisterClassEx(&wcex))
   {
      MessageBox(NULL,
         _T("Call to RegisterClassEx failed!"),
         _T("Windows Desktop Guided Tour"),
         0);

      return 1;
   }

   // Store instance handle in our global variable
   hInst = hInstance;

   // The parameters to CreateWindowEx explained:
   // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
   // szWindowClass: the name of the application
   // szTitle: the text that appears in the title bar
   // WS_OVERLAPPEDWINDOW: the type of window to create
   // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
   // 500, 100: initial size (width, length)
   // NULL: the parent of this window
   // NULL: this application does not have a menu bar
   // hInstance: the first parameter from WinMain
   // NULL: not used in this application
   HWND hWnd = CreateWindowEx(
      WS_EX_OVERLAPPEDWINDOW,
      szWindowClass,
      szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      28 * SIZE_OF_MAP + 148, 28 * SIZE_OF_MAP + 80,
      NULL,
      NULL,
      hInstance,
      NULL
   );

   if (!hWnd)
   {
      MessageBox(NULL,
         _T("Call to CreateWindow failed!"),
         _T("Windows Desktop Guided Tour"),
         0);

      return 1;
   }

   // The parameters to ShowWindow explained:
   // hWnd: the value returned from CreateWindow
   // nCmdShow: the fourth parameter from WinMain
   ShowWindow(hWnd,
      nCmdShow);
   UpdateWindow(hWnd);

   // Main message loop:
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  
	switch (message)
	{
	case WM_CREATE:
		
		GameInit(hWnd);
		hFont= CreateFont(24, 12, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
							CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
		hFont1= CreateFont(48, 24, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
							CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
		
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);

			// Here your application is laid out.
			// For this introduction, we just print out "Hello, Windows desktop!"
			// in the top left corner.
			bg = (HBITMAP)LoadImage(NULL, _T("1.bmp"), IMAGE_BITMAP, 0,0, LR_LOADFROMFILE);
			hMemdc = CreateCompatibleDC(hdc);
			SelectObject(hMemdc, bg);
			BitBlt(hdc, 0, 0, 500, 500, hMemdc, 0, 0, SRCCOPY);
			SetBkMode(hdc, TRANSPARENT);			
			ShowMap();
			ShowInfo(hWnd);
			//FillRect(hdc, &ps.rcPaint, CreateHatchBrush(5, RGB(255, 0, 0)));
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		// printf("move");
		//OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), hWnd);
		if(ifFirst)
		{
			input = Getclick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), hWnd);
			MapInit(input);
			Sweep(input);
			InvalidateRect(hWnd, NULL, TRUE);
			if (rest == 0) Win(hWnd);
			else ifFirst = 0;
		}else 
		{
			input = Getclick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), hWnd);
			if (Sweep(input)) fail(input, hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
			if (rest == 0) Win(hWnd);
		}
		
		break;
	case WM_RBUTTONDOWN:
		SignMine(Getclick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), hWnd));
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

// int main()
// {
// 	COORDINATE input;
// 	GameInit();
// 	ShowMap();
// 	input = InputCoord();
// 	MapInit(input);
// 	Sweep(input);
// 	ShowMap();
// 	while (rest)
// 	{
// 		input = InputCoord();
// 		if (Sweep(input))
// 			fail(input);		// 判断是否触雷
// 		ShowMap();
// 	}
// 	Win();
// 	return 0;
// }

// GotoXY()将光标移动到(x, y)
void GotoXY(COORDINATE coord)
{
	COORD pos;
	pos.Y = coord.x;			// 横坐标赋值
	pos.X = coord.y;			// 纵坐标赋值
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
	// 设置光标位置
}

// 初始化游戏函数
void GameInit(HWND hWnd)
{
    // 计算安全区域
	rest = SIZE_OF_MAP * SIZE_OF_MAP - mineCount;
	if (rest < 1)
	{
		mineCount = SIZE_OF_MAP * SIZE_OF_MAP - 1;
		rest = 1;
	}
	signRest = mineCount;
	// 地图初始化
	for (int i = 0; i < SIZE_OF_MAP; i++)
	{
		for (int j = 0; j < SIZE_OF_MAP; j++)
		{
			map[i][j] = (MAP)
			{
			0, 0, 0, 0};
		}

	}

}

// 布雷函数
void SetMine(COORDINATE coordinate)
{
	int mineX, mineY;
	int count = mineCount;
	srand((unsigned int)time(NULL)); //随机数种子
	while (count)
	{
		mineX = rand() % SIZE_OF_MAP + 0;
		mineY = rand() % SIZE_OF_MAP + 0;	// 随机布雷
		if (map[mineX][mineY].isMine == 0 && !(mineX == coordinate.x && mineY == coordinate.y))	// 第一个选中的区域一定不是雷
		{
			map[mineX][mineY].isMine = 1;	// 布置成功就是1
			count--;
		}
	}

}

//初始化地图
void MapInit(COORDINATE coordinate)
{
	SetMine(coordinate);
    CountMine();
}

// 打印地图
void ShowMap()
{
   char str[3];
   POINT point[3];
   HBRUSH hbrushWhite = CreateSolidBrush(RGB(255,255,255)); // 创建一个白色实心画刷
   HBRUSH hbrushBlack = CreateSolidBrush(RGB(0,0,0));
   HBRUSH hbrushRed = CreateSolidBrush(RGB(255,0,0));
   HBRUSH hbrushYellow = CreateSolidBrush(RGB(255,255,0));
   HBRUSH oldhbrush = SelectObject(hdc,hbrushWhite); // 选入设备环境，并保留先前画刷
   SetTextColor(hdc, RGB(0, 0, 0));
   // Rectangle(hdc,0,0,x,y); // 绘制一个矩形
         
    // 将光标移动到(0, 0)
	// GotoXY((COORDINATE) 
	// 	   {
	// 	   0, 0});
	// printf("  ");
   SelectObject(hdc, hFont);
	for (int i = 0; i <= SIZE_OF_MAP; i++)	// 在第一行输出Y坐标轴
	{
      Int2Char(i, str);
		TextOut(hdc, 28*i, 0, str, strlen(str));
      TextOut(hdc, 0, 28*i, str, strlen(str));
	}
	// printf(" ->y\n");
	for (int i = 0; i < SIZE_OF_MAP; i++)
	{
		// printf("%d ", i);		// X坐标轴输出
		for (int j = 0; j < SIZE_OF_MAP; j++)
		{
			if (!map[i][j].isSweeped)
			{
					// printf("* "); //未翻开的区域打印‘* ’
				SelectObject(hdc, hbrushBlack);
				Rectangle(hdc, 28 * (j + 1), 28 * (i + 1), 28 * (j + 1) + 24, 28 * (i + 1) + 24);
			}
			else if (map[i][j].isMine)
					// printf("@ "); //地雷打印‘@ ’
			{
				SelectObject(hdc, hbrushRed);
				Ellipse(hdc, 28 * (j + 1), 28 * (i + 1), 28 * (j + 1) + 24, 28 * (i + 1) + 24);
			}
			else
			{
					// printf("%d ", map[i][j].count); //其他已经翻开的区域显示周围的雷的数目
				SelectObject(hdc, hbrushWhite);
				Rectangle(hdc, 28 * (j + 1), 28 * (i + 1), 28 * (j + 1) + 24, 28 * (i + 1) + 24);
				if (map[i][j].count)
				{
				SelectObject(hdc, hFont);
				Int2Char(map[i][j].count, str);
				TextOut(hdc, 28 * (j + 1) + 6, 28 * (i + 1), str, strlen(str));
				}   
			}
			if (map[i][j].isSigned)
			{
				SelectObject(hdc, hbrushYellow);
				point[0] = (POINT){28 * (j + 1) + 12, 28 * (i + 1) + 2};
				point[1] = (POINT){28 * (j + 1) + 4, 28 * (i + 1) + 22};
				point[2] = (POINT){28 * (j + 1) + 20, 28 * (i + 1) + 22};
				Polygon(hdc, point, 3);
			}
		}
		// printf("\n");
	}
	// printf("|\nx\n");
   SelectObject(hdc,oldhbrush); // 恢复画刷
   // DeleteObject(hbrushWhite); // 删除创建画刷 
}

// 输入坐标
COORDINATE InputCoord()
{
	COORDINATE coord;
    GotoXY((COORDINATE)
				   {
				   4 + SIZE_OF_MAP, 0});
    printf("                                                                              "); //清行
	printf("\rPlease input the coordinate \"x,y\" you want to sweep --> ");
	int n;
	n = scanf("%d,%d", &coord.x, &coord.y);
	while (n != 2)				// 判断格式是否输入正确
	{
		while (getchar() != '\n');	// 清楚缓冲区
		GotoXY((COORDINATE)
			   {
			   3 + SIZE_OF_MAP, 0}
		);
		printf("\n                                                                   ");
		GotoXY((COORDINATE)
			   {
			   3 + SIZE_OF_MAP, 0}
		);
		printf("\nPlease input the coordinate \"x,y\" again --> ");
		n = scanf("%d,%d", &coord.x, &coord.y);
	}
	while (coord.x >= SIZE_OF_MAP || coord.y >= SIZE_OF_MAP || coord.x < 0 || coord.y < 0) //判断输入的坐标是否在地图里面
	{
		GotoXY((COORDINATE)
			   {
			   4 + SIZE_OF_MAP, 0}
		);
        printf("                                                                              ");
		printf("\rThe coordinate is out of map. Please input the coordinate \"x,y\" again --> ");
		n = scanf("%d,%d", &coord.x, &coord.y);
		while (n != 2)			// 判断是否输入正确
		{
			while (getchar() != '\n');	// 清楚缓冲区
			GotoXY((COORDINATE)
				   {
				   4 + SIZE_OF_MAP, 0}
			);
            printf("                                                                              ");
			printf("\rPlease input the coordinate \"x,y\" again --> ");
			n = scanf("%d,%d", &coord.x, &coord.y);
		}
	}

	return coord;
}

// 扫雷函数
int Sweep(COORDINATE coord)
{
	if (coord.x < 0 || coord.y < 0 || coord.x >= SIZE_OF_MAP || coord.y >= SIZE_OF_MAP || map[coord.x][coord.y].isSigned) return 0;
	
    COORDINATE coord1;
	if (map[coord.x][coord.y].isMine)	// 如果是雷直接返回1
		return 1;
    else if (map[coord.x][coord.y].isSweeped) return 0; // 已经翻开的区域不做处理
    // 如果是0的话，会继续自动的翻开上下左右四片区域
	else if (!(map[coord.x][coord.y].count))
	{
        SetSweeped(coord);
		for (int i = -1; i < 2; i++)
        {
            for (int j = -1; j < 2; j++)
            {
                //统计周围的雷的数目
                if (!(i ==0 && j == 0))
                {
                    coord1 = coord;
                    coord1.x += i;
                    coord1.y += j;
                    if (coord1.x >= 0 && coord1.y >= 0 && coord1.x < SIZE_OF_MAP && coord1.y < SIZE_OF_MAP)
                    {
                        Sweep(coord1);
                    }
        
                }
                
            }
		}
        // if (coord.x + 1 < SIZE_OF_MAP) Sweep((COORDINATE){coord.x + 1, coord.y});
        // if (coord.x - 1 >= 0) Sweep((COORDINATE){coord.x - 1, coord.y});
        // if (coord.y + 1 < SIZE_OF_MAP) Sweep((COORDINATE){coord.x, coord.y + 1});
        // if (coord.y - 1 >= 0) Sweep((COORDINATE){coord.x, coord.y - 1});
        return 0;
	}
    else // 如果非0，则不继续处理
    {
        SetSweeped(coord);
        return 0;
    }
}

// 打印失败的显示
void fail(COORDINATE coord, HWND hWnd)
{
	for (int i = 0; i < SIZE_OF_MAP; i++)
	{
		for (int j = 0; j < SIZE_OF_MAP; j++)
		{
			if (!map[i][j].isSweeped)
				map[i][j].isSweeped = map[i][j].isMine ? 1 : 0;
		}
		
	}
	InvalidateRect(hWnd, NULL, TRUE);
	// ShowMap();
	// MessageBox(NULL,
    //      _T("The mine exploded, you lost!"),
    //      _T("The mine exploded, you lost!"),
    //      0);
	// printf("\n\nThe mine exploded, you lost!\n");
	// system("pause");
	// exit(0);
	if (MessageBox(hWnd,
			(LPCSTR)"The mine exploded, you lost!\n\tPress retry to play again.\n\tPress no to exit.", 
			(LPCSTR)"You lost!",
			MB_RETRYCANCEL) == IDRETRY)
		{
			GameInit(hWnd);
			ifFirst = 1;
			InvalidateRect(hWnd, NULL, TRUE);
		}
	else DestroyWindow(hWnd);
}

// 打印胜利的显示
void Win(HWND hWnd)
{
	// MessageBox(NULL,
    //      _T("You are the Winner!!"),
    //      _T("You are the Winner!!"),
    //      0);
	// printf("\n\nYou are the Winner!!\n");
	// system("pause");
	if (MessageBox(hWnd,
			(LPCSTR)"You are the Winner!\n\tPress retry to play again.\n\tPress no to exit.", 
			(LPCSTR)"You WIN!",
			MB_RETRYCANCEL) == IDRETRY)
		{
			GameInit(hWnd);
			ifFirst = 1;
			InvalidateRect(hWnd, NULL, TRUE);
		}
	else DestroyWindow(hWnd);
}

// 计算每一个格子周围8个格子里面的地雷总数
void CountMine(){
    int count = 0;
    COORDINATE coord1;
    for (int i = 0; i < SIZE_OF_MAP; i++)
    {
        for (int j = 0; j < SIZE_OF_MAP; j++)
        {                                   //遍历每一个格子
            if (!map[i][j].isMine)
            {
                for (int l = -1; l < 2; l++)
                {
                    for (int k = -1; k < 2; k++)
                    {
                        // 统计周围的雷的数目
                        if (!(l == 0 && k == 0))
                        {
                            coord1 = (COORDINATE){i, j};
                            coord1.x += l;
                            coord1.y += k;
                            if (coord1.x >= 0 && coord1.y >= 0 && coord1.x < SIZE_OF_MAP && coord1.y < SIZE_OF_MAP) // 边缘
                            {                            
                                count += map[coord1.x][coord1.y].isMine;       
                            }

                        }

                    }

                }
                map[i][j].count = count;
                count = 0; // 重置计数器
            }
        }
        
    }
}

// 标记已经翻开的区域
void SetSweeped(COORDINATE coord){
    map[coord.x][coord.y].isSweeped = 1;
    rest--;
}

void Int2Char(int num, char* str){
   int i = 0;
   do
   {
      for (int j = i + 1; j > 0; j--)
      {
         str[j] = str[j - 1];
      }
      
      str[0] = (char)(num%10) + 48;
      i++;
   } while ((num /= 10) != 0);
   str[i] = '\0';
}

COORDINATE Getclick(int pixelX, int pixelY, HWND hWnd){
	int x = 0, y = 0;
	if (pixelX % 28 < 1 || pixelX % 28 > 23 || pixelY % 28 < 1 || pixelX % 28 > 23)
	{
		return (COORDINATE){-1 ,-1};
	}
	
	x = pixelY / 28 - 1;
	y = pixelX / 28 - 1;
	return (COORDINATE){x ,y};
}

void SignMine(COORDINATE coordinate){
	if (coordinate.x < 0 || coordinate.y < 0 || coordinate.x >= SIZE_OF_MAP || coordinate.y >= SIZE_OF_MAP) return;
	if (map[coordinate.x][coordinate.y].isSweeped) return;
	
	if (map[coordinate.x][coordinate.y].isSigned)
	{
		map[coordinate.x][coordinate.y].isSigned = 0;
		signRest++;
	}else
	{
		if (signRest <= 0) return;
		map[coordinate.x][coordinate.y].isSigned = 1;
		signRest--;
	}
	
	
	// map[coordinate.x][coordinate.y].isSigned ^= (unsigned __int8) 1;
}

void ShowInfo(HWND hWnd)
{
	char str[4];
	SelectObject(hdc, hFont1);
	SetTextColor(hdc, RGB(0, 255, 0));
	Int2Char(signRest, str);
	TextOut(hdc, 28 * mineCount  + 4, 56, "REST", 4);
	TextOut(hdc, 28 * mineCount + 28 , 100, str, strlen(str));
}
