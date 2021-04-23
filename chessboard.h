#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
using namespace std;

// 棋盘结构体, 存储10×9的矩阵和生成时的合法标志
struct chessboard
{
    bool valid;
    int mat[10][9];
    int *const operator[](int i) { return mat[i]; }
};
// 坐标结构体
struct position
{
    int i, j;
};
// 棋子移动结构体, 存储初末位置以及由字符串生成时的合法标志
struct movement
{
    bool valid;
    position b, e;
};

extern const string charset, CHARSET[], ini_state;

int no_(char);
bool isvalid(chessboard);
void print_board(chessboard, int = 0, position[] = NULL);
string brd2str(chessboard);
chessboard str2brd(string);
string mov2str(movement);
movement str2mov(string);
int ischeck(chessboard);
vector<string> possible_move(chessboard, int, position = {-1, -1});
void move(chessboard &, string);
void play();
