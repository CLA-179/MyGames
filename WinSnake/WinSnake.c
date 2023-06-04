#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <tchar.h>
#include <wchar.h>
#include <gdiplus.h>
#include <assert.h>

// #define UP 1
// #define DOWN 2
// #define LEFT 3
// #define RIGHT 4

enum direction
{
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
} dire;

static TCHAR szWindowClass[] = _T("DesktopApp");

// 标题
static TCHAR szTitle[] = _T("SNAKE");
// 句柄
HINSTANCE hInst;
// 字体笔刷
HFONT hFont1;
// 窗口句柄
HDC hdc, hMemdc;
// 背景
HBITMAP bg;

// 初始配置文件
struct
{
    unsigned __int8 mapSize;   // 地图大小
    unsigned __int8 blockSize; // 蛇身大小
    unsigned __int8 initLen;   // 初始长度
    unsigned __int16 speed;    // 速度
} option;

// 坐标结构体
typedef struct
{
    unsigned __int8 x;
    unsigned __int8 y;
} COORDINATE;

// 蛇的节点的结构体
typedef struct snake_node
{
    COORDINATE coord;
    struct snake_node *next;
} SNAKE_NODE;

// 蛇头蛇尾
SNAKE_NODE *head, *tail;
// 食物
COORDINATE food;

// 游戏状态结构体
struct
{
    unsigned __int8 direction;  // 方向
    COLORREF foodColor; //随机颜色食物
    unsigned int points;        // 得分
    unsigned __int8 ifGetPoint; // 是否得分
    unsigned __int8 pause;      // 是否暂停
    unsigned __int8 moveable;   // 是否可以转向
} state;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // win窗口相应函数
void Stealth();//隐藏控制台窗口
void ShowMap(HWND);                                   // 打印地图及背景
void ShowFood(HWND);                                  // 显示食物
void CreateSnake();                                   // 蛇初始化
void ShowSnake(HWND);                                 // 打印蛇
void MoveSnake();                                     // 蛇的移动
void GameInit(HWND);                                  // 初始化游戏
void KeyProc(char, HWND);                             // 键盘相应函数
int IsSafe();                                         // 是否死亡函数
void Fail(HWND);                                      // 失败处理函数
void SetFood();                                       // 生成食物函数
void IfGetPoint();                                    // 是否得分函数
void ShowPoint(HWND);                                 // 显示分数函数
void Int2Char(int, char *);                           // 数字转字符串函数
void Pause(HWND);                                     // 暂停函数
void ShowPause(HWND);                                 // 暂停显示函数
unsigned __int8 LoadOpt(char *);                      // 加载设置文件函数

// winmain函数是Win32里的主函数，相当于int main();
// 定义这个函数之后不能定义main()
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{

    WNDCLASSEX wcex;
    // 窗口的一些参数
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_HAND);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL; // 设置背景的笔刷为NULL防止频闪
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    // 判断是否能生成窗口
    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
                   _T("Call to RegisterClassEx failed!"),
                   _T("Windows Desktop Guided Tour"),
                   0);

        return 1;
    }

    // 加载配置文件
    while (!LoadOpt("option.txt"))
    {
        if (MessageBox(NULL,
                       (LPCSTR) "Can not read the option file.\n\tRetry or EXIT?",
                       (LPCSTR) "ERROR",
                       MB_RETRYCANCEL) != IDRETRY)
            exit(0);
    }

    hInst = hInstance;

    // 创建窗口
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        12 * option.mapSize + 148, 12 * option.mapSize + 120, // 窗口大小
        NULL,
        NULL,
        hInstance,
        NULL);
    if (!hWnd)
    {
        MessageBox(NULL,
                   _T("Call to CreateWindow failed!"),
                   _T("Windows Desktop Guided Tour"),
                   0);

        return 1;
    }
    
    // 显示窗口
    ShowWindow(hWnd,
               nCmdShow);
    UpdateWindow(hWnd);

    // 获取消息并循环处理消息（windows系统API）:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        // 在这之后会调用WndProc函数对消息做出回应
    }

    return (int)msg.wParam;
}

// 消息回应函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: // 窗口建立
        Stealth();
        // 定义字体
        hFont1 = CreateFont(option.blockSize * 2, option.blockSize, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,
                            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
        // 初始化游戏
        GameInit(hWnd);
        return 0;
    case WM_PAINT: // 窗口更新
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hWnd, &ps); // 打印句柄

        bg = (HBITMAP)LoadImage(NULL, _T("1.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); // 加载背景
        hMemdc = CreateCompatibleDC(hdc);
        SelectObject(hMemdc, bg);
        BitBlt(hdc, 0, 0, 12 * option.mapSize + 148, 12 * option.mapSize + 120, hMemdc, 0, 0, SRCCOPY);
        SetBkMode(hdc, TRANSPARENT); // 双缓冲加载背景

        ShowMap(hWnd);           // 地图
        ShowFood(hWnd); //食物
        ShowSnake(hWnd); // 蛇
        ShowPoint(hWnd); // 积分
        ShowPause(hWnd); // 暂停提示

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY: // 摧毁窗口
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN: // 键盘按下
        KeyProc(wParam, hWnd);
        break;
    case WM_TIMER:                        // 计时器相应
        MoveSnake();                      // 移动蛇
        IfGetPoint();                     // 判断是否得分
        InvalidateRect(hWnd, NULL, TRUE); // 刷新窗口
        if (!IsSafe())                    // 判断是否死亡
            Fail(hWnd);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

//隐藏控制台窗口
void Stealth()
{
 HWND Stealth;
 AllocConsole();
 Stealth = GetConsoleWindow();
 ShowWindow(Stealth,0);
}
// 打印地图
void ShowMap(HWND hWnd)
{
    
    HBRUSH hbrushBlack = CreateSolidBrush(RGB(0, 0, 0)); // 创建一个黑色实心画刷
    SelectObject(hdc, hbrushBlack);                      // 选择画刷

    // 画地图周围一圈的边框
    Rectangle(hdc, 2 * option.blockSize, 2 * option.blockSize, 3 * option.blockSize, (2 + option.mapSize + 2) * option.blockSize);
    Rectangle(hdc, 2 * option.blockSize, 2 * option.blockSize, (2 + option.mapSize + 2) * option.blockSize, 3 * option.blockSize);
    Rectangle(hdc, (3 + option.mapSize) * option.blockSize, 2 * option.blockSize, (2 + option.mapSize + 2) * option.blockSize, (2 + option.mapSize + 2) * option.blockSize);
    Rectangle(hdc, 2 * option.blockSize, (3 + option.mapSize) * option.blockSize, (2 + option.mapSize + 2) * option.blockSize, (2 + option.mapSize + 2) * option.blockSize);
    DeleteObject(hbrushBlack); // 删除黑色画刷
}

// 画食物
void ShowFood(HWND hWnd)
{
    HBRUSH hbrushRand = CreateSolidBrush(state.foodColor); // 创建一个随机颜色实心画刷
    SelectObject(hdc, hbrushRand);                        // 选择
    Ellipse(hdc, option.blockSize * (3 + food.x), option.blockSize * (3 + food.y),
            option.blockSize * (4 + food.x), option.blockSize * (4 + food.y));
    DeleteObject(hbrushRand); // 删除
}

// 蛇初始化
void CreateSnake()
{
    SNAKE_NODE *p;
    p = (SNAKE_NODE *)malloc(sizeof(SNAKE_NODE));
    head = p;
    // 创建一个链表，表头就是蛇头，初始的蛇头指向右，位置在中间
    for (int i = 0; i < option.initLen - 1; i++)
    {
        // 蛇身的坐标依次左移
        p->coord = (COORDINATE){(int)(option.mapSize) / 2 - i, (int)(option.mapSize) / 2};
        p->next = (SNAKE_NODE *)malloc(sizeof(SNAKE_NODE));
        p = p->next;
    }
    // 蛇尾
    p->coord = (COORDINATE){(int)(option.mapSize) / 2 - option.initLen + 1, (int)(option.mapSize) / 2};
    p->next = NULL;
    tail = p;
}

// 展示蛇
void ShowSnake(HWND hWnd)
{
    HBRUSH hbrushYellow = CreateSolidBrush(RGB(255, 255, 0)); // 黄色笔刷
    HBRUSH hbrushRed = CreateSolidBrush(RGB(255, 0, 0));      // 红色笔刷
    SelectObject(hdc, hbrushRed);

    // 蛇头为红色矩形
    Rectangle(hdc, option.blockSize * (3 + head->coord.x), option.blockSize * (3 + head->coord.y),
              option.blockSize * (4 + head->coord.x), option.blockSize * (4 + head->coord.y));
    SelectObject(hdc, hbrushYellow);
    // 蛇身为黄色矩形
    SNAKE_NODE *p = head->next;
    for (; p != NULL; p = p->next) // 遍历打印蛇身
    {
        Rectangle(hdc, option.blockSize * (3 + p->coord.x), option.blockSize * (3 + p->coord.y),
                  option.blockSize * (4 + p->coord.x), option.blockSize * (4 + p->coord.y));
    }
    DeleteObject(hbrushYellow);
    DeleteObject(hbrushRed);
}

void MoveSnake()
{
    COORDINATE move;
    SNAKE_NODE *p = (SNAKE_NODE *)malloc(sizeof(SNAKE_NODE));
    // 根据方向来判断蛇的移动
    switch (state.direction)
    {
    case LEFT:
        move = (COORDINATE){-1, 0};
        break;
    case RIGHT:
        move = (COORDINATE){1, 0};
        break;
    case UP:
        move = (COORDINATE){0, -1};
        break;
    case DOWN:
        move = (COORDINATE){0, 1};
        break;
    default:
        return;
    }
    // 在移动到的位置上创建新的节点，并使其为新的蛇头
    p->coord.x = head->coord.x + move.x;
    p->coord.y = head->coord.y + move.y;
    p->next = head;
    head = p;

    // 如果没吃到食物，就删除蛇尾最后一个节点，等效于蛇的移动
    if (state.ifGetPoint)
    {
        state.ifGetPoint = 0;
        return;
    }

    while (p->next != tail)
        p = p->next;
    free(p->next);
    tail = p;
    p->next = NULL;

    state.moveable = 1;
}

void GameInit(HWND hWnd)
{
    mciSendString("play background.mp3 repeat", NULL, 0, NULL); // 播放背景音乐

    // 初始化游戏状态
    state.direction = RIGHT;
    state.points = 0;
    state.ifGetPoint = 0;
    state.pause = 0;
    state.moveable = 1;
    // 初始化蛇
    CreateSnake();
    //提示说明
    if(MessageBox(hWnd,
                       (LPCSTR) "Use \"WASD\" or \"Direction key\" to move the snake\nUse \"P\" to pause\nPlease swtich to English input mode or enable uppercase input",
                       (LPCSTR) "Readme",
                       MB_OKCANCEL) == IDOK)
    // 设置计时器，计时器每speed毫秒发出一条WM_TIMER消息
    SetTimer(hWnd, 1, option.speed, NULL);
    // 放置第一个食物
    SetFood();
}

// 键盘相应
void KeyProc(char newDire, HWND hWnd)
{
    // 按下按键后在蛇头转向之前不会再转向
    if (!state.moveable)
        return;
    switch (newDire)
    {
    case 'W':
    case '&':
        state.direction = (state.direction == DOWN) ? DOWN : UP; // 防止向后走，下同
        state.moveable = 0;
        break;
    case 'S':
    case '(':
        state.direction = (state.direction == UP) ? UP : DOWN;
        state.moveable = 0;
        break;
    case 'A':
    case '%':
        state.direction = (state.direction == RIGHT) ? RIGHT : LEFT;
        state.moveable = 0;
        break;
    case 'D':
    case '\'':
        state.direction = (state.direction == LEFT) ? LEFT : RIGHT;
        state.moveable = 0;
        break;
    case 'P':
        Pause(hWnd); // 暂停
    default:
        return;
    }
}

int IsSafe()
{
    SNAKE_NODE *p = head->next;
    // 查找蛇头是否与蛇身相撞
    for (; p != NULL; p = p->next)
        if (p->coord.x == head->coord.x && p->coord.y == head->coord.y)
            return 0;

    // 是否撞墙
    if (head->coord.x >= option.mapSize || head->coord.x < 0 || head->coord.y >= option.mapSize || head->coord.y < 0)
        return 0;

    return 1;
}

// 失败
void Fail(HWND hWnd)
{
    mciSendString("stop background.mp3", NULL, 0, NULL); // 停止背景音乐
    mciSendString("play fail.mp3 wait", NULL, 0, NULL);  // 播放失败音效
    KillTimer(hWnd, 1);                                  // 停止计时器

    // 弹出失败的提示框，并询问是否重新开始游戏
    if (MessageBox(hWnd,
                   (LPCSTR) "you lost!\n\tPress retry to play again.\n\tPress no to exit.",
                   (LPCSTR) "You lost!",
                   MB_RETRYCANCEL) == IDRETRY)
    {
        GameInit(hWnd);
        InvalidateRect(hWnd, NULL, TRUE);
    }
    else
        DestroyWindow(hWnd);
}

// 判断是否得分
void IfGetPoint()
{
    // 如果蛇头坐标与食物的坐标相同，就认定为吃到
    if (!(head->coord.x == food.x && head->coord.y == food.y))
        return;
    // 播放音效
    PlaySound(_T("point.wav"), NULL, SND_FILENAME | SND_ASYNC);
    state.points++; // 计分
    // 重新放置食物
    SetFood();
    // 标记状态，使蛇身加长
    state.ifGetPoint = 1;
}

// 放置食物
void SetFood()
{
    //随机食物颜色
    unsigned __int8 r, g, b;
    srand((unsigned int)time(NULL));
    r = rand() % 255 + 0;
    g = rand() % 255 + 0;
    b = rand() % 255 + 0;
    state.foodColor = RGB(r, g, b);

    //随机食物位置
    unsigned __int8 foodX, foodY;
    unsigned __int8 sign = 0;
    SNAKE_NODE *p = head;
    srand((unsigned int)time(NULL)); // 随机数种子

    // 食物不会出现再蛇的身体里面
    do
    {
        foodX = rand() % option.mapSize + 0;
        foodY = rand() % option.mapSize + 0;
        for (; p != NULL; p = p->next)
        {
            if (foodX == p->coord.x && foodY == p->coord.y)
            {
                sign = 1;
                break;
            }
            sign = 0;
        }

    } while (sign);

    food.x = foodX;
    food.y = foodY;
}

// 打印得分
void ShowPoint(HWND hWnd)
{
    char str[2][4];
    // 使用字体并设定颜色
    SelectObject(hdc, hFont1);
    SetTextColor(hdc, RGB(0, 0, 0));
    // 将积分由数子转为字符串
    Int2Char(state.points, str[0]);
    Int2Char(state.points + option.initLen, str[1]);

    // 再相应位置打印积分以及蛇长
    TextOut(hdc, option.blockSize * (option.mapSize + 5), option.blockSize * 3, "POINTS", 6);
    TextOut(hdc, option.blockSize * (option.mapSize + 6), option.blockSize * 5, str[0], strlen(str[0]));
    TextOut(hdc, option.blockSize * (option.mapSize + 5), option.blockSize * 8, "LENTH", 5);
    TextOut(hdc, option.blockSize * (option.mapSize + 6), option.blockSize * 10, str[1], strlen(str[1]));
}

// 暂停，继续函数
void Pause(HWND hWnd)
{
    if (state.pause == 0)
    {
        // 暂停之后停止计时器，并刷新屏幕
        KillTimer(hWnd, 1);
        state.pause = 1;
        InvalidateRect(hWnd, NULL, TRUE);
    }
    else
    {
        // 继续之后回复计时器
        SetTimer(hWnd, 1, option.speed, NULL);
        state.pause = 0;
    }
}

void Int2Char(int num, char *str)
{
    // 将整型变量转换为字符串
    int i = 0;
    do
    {
        for (int j = i + 1; j > 0; j--)
        {
            str[j] = str[j - 1];
        }

        str[0] = (char)(num % 10) + 48;
        i++;
    } while ((num /= 10) != 0);
    str[i] = '\0';
}

// 打印暂停提示
void ShowPause(HWND hWnd)
{
    // 红黄笔刷
    HBRUSH hbrushYellow = CreateSolidBrush(RGB(255, 255, 0));
    HBRUSH hbrushRed = CreateSolidBrush(RGB(255, 0, 0));

    // 在相应的位置打印提示
    if (state.pause == 0)
    {
        SelectObject(hdc, hbrushRed);
        Rectangle(hdc, option.blockSize * (option.mapSize + 5) - 3, option.blockSize * 13, option.blockSize * (option.mapSize + 9) + 3, option.blockSize * 17);
        SelectObject(hdc, hFont1);
        SetTextColor(hdc, RGB(0, 0, 0));
        TextOut(hdc, option.blockSize * (option.mapSize + 5), option.blockSize * 13, "PAUSE", 5);
        TextOut(hdc, option.blockSize * (option.mapSize + 6), option.blockSize * 15, "(P)", 3);
    }
    else
    {
        SelectObject(hdc, hbrushYellow);
        Rectangle(hdc, option.blockSize * (option.mapSize + 5) - 3, option.blockSize * 13, option.blockSize * (option.mapSize + 9) + 3, option.blockSize * 19);
        SelectObject(hdc, hFont1);
        SetTextColor(hdc, RGB(0, 0, 0));
        TextOut(hdc, option.blockSize * (option.mapSize + 5) + (option.blockSize / 2), option.blockSize * 13, "CONT", 4);
        TextOut(hdc, option.blockSize * (option.mapSize + 5) + (option.blockSize / 2), option.blockSize * 15, "INUE", 4);
        TextOut(hdc, option.blockSize * (option.mapSize + 6), option.blockSize * 17, "(P)", 3);
    }

    DeleteObject(hbrushYellow);
    DeleteObject(hbrushRed);
}

// 加载配置
unsigned __int8 LoadOpt(char *filename)
{
    FILE *fp;
    // 打开文件
    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    // 格式化读取配置，如果读取失败返回0
    if (fscanf(fp, "SIZE_OF_MAP %d\n", &option.mapSize) != 1)
        return 0;
    if (fscanf(fp, "SIZE_OF_BLOCK %d\n", &option.blockSize) != 1)
        return 0;
    if (fscanf(fp, "INIT_LEN %d\n", &option.initLen) != 1)
        return 0;
    if (fscanf(fp, "SPEED %d\n", &option.speed) != 1)
        return 0;
    // 判断读取的配置是否在合法范围内，不是则取端点值
    if (option.blockSize < 6)
        option.blockSize = 6;
    if (option.blockSize > 18)
        option.blockSize = 18;
    if (option.mapSize < 15)
        option.mapSize = 15;
    if (option.mapSize > 40)
        option.mapSize = 40;
    if (option.initLen < 3)
        option.initLen = 3;
    if (option.initLen > option.mapSize / 2)
        option.initLen = option.mapSize / 2;
    if (option.speed < 30)
        option.speed = 30;
    return 1;
}