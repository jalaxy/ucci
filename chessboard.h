#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
using namespace std;

// ���̽ṹ��, �洢10��9�ľ��������ʱ�ĺϷ���־
struct chessboard
{
    bool valid;
    int mat[10][9];
    int *const operator[](int i) { return mat[i]; }
};
// ����ṹ��
struct position
{
    int i, j;
};
// �����ƶ��ṹ��, �洢��ĩλ���Լ����ַ�������ʱ�ĺϷ���־
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
