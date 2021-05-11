#include <fstream>
#include <sstream>
#include "chessboard.h"
using namespace std;
const string CHARSET_PGN[] = {"  ",
                              "车", "马", "象", "士", "将", "炮", "卒",
                              "车", "马", "相", "仕", "帅", "炮", "兵",
                              "前", "后", "中"};
const string CHARSET_PGN_POS[] = {"１", "２", "３", "４", "５", "６", "７", "８", "９",
                                  "一", "二", "三", "四", "五", "六", "七", "八", "九"};
int no_(string str, int turn)
{
    for (int i = (turn == 1 ? 8 : 1); i < (turn == 1 ? 15 : 8); i++)
        if (str == CHARSET_PGN[i])
            return i;
    return 0;
}
int no__(string str, bool flag = 0)
{
    for (int i = 0; i < 18; i++)
        if (str == CHARSET_PGN_POS[i])
            return i < 9 ? (flag ? i + 1 : i) : (flag ? i - 8 : 17 - i);
    return 0;
}
position search(chessboard &brd, int piece, int flag, int col, int num = 0)
{
    //normal: 0, forward: 1, back: 2, mid: 3
    position pos = {-1, -1};
    int cnt = 0;
    switch (flag)
    {
    case 0:
        for (int i = 0; i < 10; i++)
            if (brd[i][col] == piece)
            {
                pos = {i, col};
                if (cnt == num)
                    break;
                cnt++;
            }
        break;
    case 1:
        for (int j = 0; j < 9; j++)
        {
            cnt = 0;
            for (int i = 0; i < 10; i++)
                if (brd[i][j] == piece)
                    cnt++;
            if (cnt < 2)
                continue;
            for (int i = 0; i < 10; i++)
                if (brd[i][j] == piece)
                    if (piece > 7 && (i < pos.i || pos.i < 0) ||
                        piece < 8 && (i > pos.i || pos.i < 0))
                        pos = {i, j};
        }
        break;
    case 2:
        for (int j = 0; j < 9; j++)
        {
            cnt = 0;
            for (int i = 0; i < 10; i++)
                if (brd[i][j] == piece)
                    cnt++;
            if (cnt < 2)
                continue;
            for (int i = 0; i < 10; i++)

                if (brd[i][j] == piece)
                    if (piece > 7 && (i > pos.i || pos.i < 0) ||
                        piece < 8 && (i < pos.i || pos.i < 0))
                        pos = {i, j};
        }
        break;
    case 3:
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 9; j++)
                if (brd[i][j] == piece)
                    if (piece > 7 && (i > pos.i || pos.i < 0) ||
                        piece < 8 && (i < pos.i || pos.i < 0))
                        if (pos.i >= 0)
                            return {i, j};
    }
    return pos;
}
movement pgn2mov(chessboard &brd, string str, int turn)
{
    movement mov = {-1, -1};
    string ch1 = str.substr(0, 2), ch2 = str.substr(2, 2),
           ch3 = str.substr(4, 2), ch4 = str.substr(6, 2),
           piece = (ch1 == "前" || ch1 == "后" || ch1 == "中" ? ch2 : ch1);
    if (ch1 == "前" || ch1 == "后" || ch1 == "中")
        mov.b = search(brd, no_(piece, turn), ch1 == "前" ? 1 : (ch1 == "后" ? 2 : 3), 0);
    else
        mov.b = search(brd, no_(piece, turn), 0, no__(ch2));
    vector<string> s_set = possible_move(brd, turn, mov.b);
    int delta = (ch3 == "进" ? 1 : (ch3 == "退" ? -1 : 0));
    if (turn == 1)
        delta = -delta;
    if (piece == "车" || piece == "炮" || piece == "将" ||
        piece == "帅" || piece == "卒" || piece == "兵")
        if (delta == 1)
            mov.e = {mov.b.i + no__(ch4, 1), mov.b.j};
        else if (delta == -1)
            mov.e = {mov.b.i - no__(ch4, 1), mov.b.j};
        else
            mov.e = {mov.b.i, no__(ch4)};
    else if (piece == "马")
        if (delta == 1)
            mov.e = {mov.b.i + 3 - abs(no__(ch4) - mov.b.j), no__(ch4)};
        else
            mov.e = {mov.b.i - 3 + abs(no__(ch4) - mov.b.j), no__(ch4)};
    else if (piece == "象" || piece == "相")
        if (delta == 1)
            mov.e = {mov.b.i + 2, no__(ch4)};
        else
            mov.e = {mov.b.i - 2, no__(ch4)};
    else if (piece == "士" || piece == "仕")
        if (delta == 1)
            mov.e = {mov.b.i + 1, no__(ch4)};
        else
            mov.e = {mov.b.i - 1, no__(ch4)};
    for (int i = 0; i < s_set.size(); i++)
        if (mov2str(mov) == s_set[i])
            return mov;
    if (ch1 == "前" || ch1 == "后" || ch1 == "中")
        mov.b = search(brd, no_(piece, turn), ch1 == "前" ? 1 : (ch1 == "后" ? 2 : 3), 0);
    else
        mov.b = search(brd, no_(piece, turn), 0, no__(ch2), 1);
    delta = (ch3 == "进" ? 1 : (ch3 == "退" ? -1 : 0));
    if (turn == 1)
        delta = -delta;
    if (piece == "车" || piece == "炮" || piece == "将" ||
        piece == "帅" || piece == "卒" || piece == "兵")
        if (delta == 1)
            mov.e = {mov.b.i + no__(ch4, 1), mov.b.j};
        else if (delta == -1)
            mov.e = {mov.b.i - no__(ch4, 1), mov.b.j};
        else
            mov.e = {mov.b.i, no__(ch4)};
    else if (piece == "马")
        if (delta == 1)
            mov.e = {mov.b.i + 3 - abs(no__(ch4) - mov.b.j), no__(ch4)};
        else
            mov.e = {mov.b.i - 3 + abs(no__(ch4) - mov.b.j), no__(ch4)};
    else if (piece == "象" || piece == "相")
        if (delta == 1)
            mov.e = {mov.b.i + 2, no__(ch4)};
        else
            mov.e = {mov.b.i - 2, no__(ch4)};
    else if (piece == "士" || piece == "仕")
        if (delta == 1)
            mov.e = {mov.b.i + 1, no__(ch4)};
        else
            mov.e = {mov.b.i - 1, no__(ch4)};
    return mov;
}
vector<string> read_pgn(string filename)
{
    chessboard brd = str2brd(ini_state);
    vector<string> s_set;
    fstream in(filename, ios::in);
    string s;
    do
        getline(in, s);
    while (s[0] == '[');
    double no;
    while (!in.fail())
    {
        stringstream ss(s);
        ss >> no;
        if (ss.fail())
            break;
        string str;
        ss >> str;
        if (str.length() != 8)
            break;
        string smov = mov2str(pgn2mov(brd, str, 1));
        s_set.push_back(smov);
        move(brd, smov);
        ss >> str;
        if (str.length() != 8)
            break;
        smov = mov2str(pgn2mov(brd, str, 2));
        s_set.push_back(smov);
        move(brd, smov);
        getline(in, s);
    }
    in.close();
    return s_set;
}
int main()
{
    const int size = 12141;
    fstream out("opening\\record.txt", ios::out);
    for (int i = 0; i < size; i++)
    {
        string filename = "opening\\pgn\\";
        char stmp[8];
        filename += itoa(i, stmp, 10);
        filename += ".pgn";
        vector<string> s_set = read_pgn(filename);
        for (int i = 0; i < s_set.size(); i++)
            out << (i == 0 ? "" : " ") << s_set[i];
        out << endl;
        cout << i << endl;
    }
    out.close();
    system("pause");
    return 0;
}