#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define SIZE 9					// 地图大小
#define MINE_COUNT 1			// 地雷个数
// 地图结构体
typedef struct
{
	unsigned __int8 isMine;					// 是否是地雷
	unsigned __int8 count;					// 周围地雷数目
	unsigned __int8 isSweeped;				// 是否被扫过
} MAP;
// 坐标结构体
typedef struct
{
	unsigned __int8 x;
	unsigned __int8 y;
} COORDINATE;


MAP map[SIZE][SIZE];			// 定义地图数组
unsigned __int8 rest;                       //剩余的安全位置，为0则游戏胜利

//声明函数
void GotoXY(COORDINATE coord);
void GameInit();
void SetMine(COORDINATE coordinate);
void MapInit(COORDINATE coordinate);
void ShowMap();
COORDINATE InputCoord();
int Sweep(COORDINATE coord);
void fail(COORDINATE coord);
void Win();
void CountMine();
void SetSweeped(COORDINATE coord);

int main()
{
	COORDINATE input;
	GameInit();
	ShowMap();
	input = InputCoord();
	MapInit(input);
	Sweep(input);
	ShowMap();
	while (rest)
	{
		input = InputCoord();
		if (Sweep(input))
			fail(input);		// 判断是否触雷
		ShowMap();
	}
	Win();
	return 0;
}

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
void GameInit()
{
    // 计算安全区域
	rest = SIZE * SIZE - MINE_COUNT;
	// 地图初始化
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			map[i][j] = (MAP)
			{
			0, 0, 0};
		}

	}

}

// 布雷函数
void SetMine(COORDINATE coordinate)
{
	int mineX, mineY;
	int count = MINE_COUNT;
	srand((unsigned int)time(NULL)); //随机数种子
	while (count)
	{
		mineX = rand() % SIZE + 0;
		mineY = rand() % SIZE + 0;	// 随机布雷
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
    // 将光标移动到(0, 0)
	GotoXY((COORDINATE) 
		   {
		   0, 0});
	printf("  ");
	for (int i = 0; i < SIZE; i++)	// 在第一行输出Y坐标轴
	{
		printf("%d ", i);
	}
	printf(" ->y\n");
	for (int i = 0; i < SIZE; i++)
	{
		printf("%d ", i);		// X坐标轴输出
		for (int j = 0; j < SIZE; j++)
		{
			if (!map[i][j].isSweeped)
				printf("* "); //未翻开的区域打印‘* ’
			else if (map[i][j].isMine)
				printf("@ "); //地雷打印‘@ ’
			else
				printf("%d ", map[i][j].count); //其他已经翻开的区域显示周围的雷的数目
		}
		printf("\n");
	}
	printf("|\nx\n");
}

// 输入坐标
COORDINATE InputCoord()
{
	COORDINATE coord;
    GotoXY((COORDINATE)
				   {
				   4 + SIZE, 0});
    printf("                                                                              "); //清行
	printf("\rPlease input the coordinate \"x,y\" you want to sweep --> ");
	int n;
	n = scanf("%d,%d", &coord.x, &coord.y);
	while (n != 2)				// 判断格式是否输入正确
	{
		while (getchar() != '\n');	// 清楚缓冲区
		GotoXY((COORDINATE)
			   {
			   3 + SIZE, 0}
		);
		printf("\n                                                                   ");
		GotoXY((COORDINATE)
			   {
			   3 + SIZE, 0}
		);
		printf("\nPlease input the coordinate \"x,y\" again --> ");
		n = scanf("%d,%d", &coord.x, &coord.y);
	}
	while (coord.x >= SIZE || coord.y >= SIZE || coord.x < 0 || coord.y < 0) //判断输入的坐标是否在地图里面
	{
		GotoXY((COORDINATE)
			   {
			   4 + SIZE, 0}
		);
        printf("                                                                              ");
		printf("\rThe coordinate is out of map. Please input the coordinate \"x,y\" again --> ");
		n = scanf("%d,%d", &coord.x, &coord.y);
		while (n != 2)			// 判断是否输入正确
		{
			while (getchar() != '\n');	// 清楚缓冲区
			GotoXY((COORDINATE)
				   {
				   4 + SIZE, 0}
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
                
                if (!(i ==0 && j == 0))
                {
                    coord1 = coord;
                    coord1.x += i;
                    coord1.y += j;
                    if (coord1.x >= 0 && coord1.y >= 0 && coord1.x < SIZE && coord1.y < SIZE)
                    {
                        Sweep(coord1);
                    }
        
                }
                
            }
		}
        // if (coord.x + 1 < SIZE) Sweep((COORDINATE){coord.x + 1, coord.y});
        // if (coord.x - 1 >= 0) Sweep((COORDINATE){coord.x - 1, coord.y});
        // if (coord.y + 1 < SIZE) Sweep((COORDINATE){coord.x, coord.y + 1});
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
void fail(COORDINATE coord)
{
	SetSweeped(coord);
	ShowMap();
	printf("\n\nThe mine exploded, you lost!\n");
	system("pause");
	exit(0);
}

// 打印胜利的显示
void Win()
{
	printf("\n\nYou are the Winner!!\n");
	system("pause");
}

// 计算每一个格子周围8个格子里面的地雷总数
void CountMine(){
    int count = 0;
    COORDINATE coord1;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
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
                            if (coord1.x >= 0 && coord1.y >= 0 && coord1.x < SIZE && coord1.y < SIZE) // 边缘
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

