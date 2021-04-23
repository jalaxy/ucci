// Encoding: ANSI_GB2312

#include "chessboard.h"

// 中英文字符数组, 字符集为GB2312
const string charset = " rnbakcpRNBAKCP",
             CHARSET[] = {"  ",
                          "車", "馬", "象", "士", "將", "砲", "卒",
                          "俥", "傌", "相", "仕", "帥", "炮", "兵"},
             ini_state = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";

/* 由charset中的字符转为对应编号
 * 参数: 字符
 * 返回值: 对应编号
 */
int no_(char ch)
{
    for (int i = 0; i < charset.length(); i++)
        if (ch == charset[i])
            return i;
    return 0;
}

/* 检查矩阵是否合法, 包括棋子个数的检查, 棋子可能到达的位置
 * 参数: 棋盘结构体
 * 返回值: 是否合法
 */
bool isvalid(chessboard brd)
{
    int num[15];
    const int num_max[15] = {90,
                             2, 2, 2, 2, 1, 2, 5,
                             2, 2, 2, 2, 1, 2, 5};
    memset(num, 0, sizeof(num));
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 9; j++)
        {
            int piece = brd[i][j];
            num[piece] += 1;
            if (num[piece] > num_max[piece])
                return 0;
            char ch = charset[piece];
            switch (ch)
            {
            case 'b':
                if (!(i == 0 && j == 2 || i == 0 && j == 6 ||
                      i == 2 && j == 0 || i == 2 && j == 4 ||
                      i == 2 && j == 8 ||
                      i == 4 && j == 2 || i == 4 && j == 6))
                    return 0;
                break;
            case 'B':
                if (!(i == 5 && j == 2 || i == 5 && j == 6 ||
                      i == 7 && j == 0 || i == 7 && j == 4 ||
                      i == 7 && j == 8 ||
                      i == 9 && j == 2 || i == 9 && j == 6))
                    return 0;
                break;
            case 'a':
                if (!(i == 0 && j == 3 || i == 0 && j == 5 ||
                      i == 1 && j == 4 ||
                      i == 2 && j == 3 || i == 2 && j == 5))
                    return 0;
                break;
            case 'A':
                if (!(i == 7 && j == 3 || i == 7 && j == 5 ||
                      i == 8 && j == 4 ||
                      i == 9 && j == 3 || i == 9 && j == 5))
                    return 0;
                break;
            case 'k':
                if (i > 2 || j < 3 || j > 5)
                    return 0;
                break;
            case 'K':
                if (i < 7 || j < 3 || j > 5)
                    return 0;
                break;
            case 'p':
                if (!(i > 4 || i > 2 && j % 2 == 0))
                    return 0;
                break;
            case 'P':
                if (!(i < 5 || i < 7 && j % 2 == 0))
                    return 0;
                break;
            }
        }
    if (num[5] != 1 || num[12] != 1)
        return 0;
    return 1;
}

/* 控制台标准输出打印棋盘函数
 * 参数: 棋盘结构体以及需要标注的位置(一般在移动时需要)
 * 返回值: 无
 */
void print_board(chessboard brd, int n, position hlt[])
{
    string ch_mat[21][19] = {{"  ", "a ", "  ", "b ", "  ", "c ", "  ", "d ", "  ", "e ", "  ", "f ", "  ", "g ", "  ", "h ", "  ", "i ", "  "}};
    for (int i = 0; i < 10; i++)
    {
        int k = 0;
        char s_tmp[32];
        itoa(9 - i, s_tmp, 10);
        (ch_mat[2 * i + 1][k++] = s_tmp) += " ";
        for (int j = 0; j < 9; j++)
        {
            if (brd[i][j] == 0)
                if (i == 0 || i == 5 && j != 0 && j != 8)
                    if (j == 0)
                        ch_mat[2 * i + 1][k++] = "┌-";
                    else if (j == 8)
                        ch_mat[2 * i + 1][k++] = "┐ ";
                    else
                        ch_mat[2 * i + 1][k++] = "┬-";
                else if (i == 9 || i == 4 && j != 0 && j != 8)
                    if (j == 0)
                        ch_mat[2 * i + 1][k++] = "└-";
                    else if (j == 8)
                        ch_mat[2 * i + 1][k++] = "┘ ";
                    else
                        ch_mat[2 * i + 1][k++] = "┴-";
                else if (j == 0)
                    ch_mat[2 * i + 1][k++] = "├-";
                else if (j == 8)
                    ch_mat[2 * i + 1][k++] = "┤ ";
                else
                    ch_mat[2 * i + 1][k++] = "┼-";
            else
                ch_mat[2 * i + 1][k++] = CHARSET[brd[i][j]];
            if (j < 8)
                ch_mat[2 * i + 1][k++] = "─-";
        }
        string str_tmp = " ";
        ch_mat[2 * i + 1][k++] = str_tmp + s_tmp;
        string str_template[5][19] = {
            {"  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "╲ ", "│ ", "╱ ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  "},
            {"  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "╱ ", "│ ", "╲ ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  "},
            {"  ", "│ ", "  ", "楚", "  ", "河", "  ", "　", "  ", "　", "  ", "　", "  ", "漢", "  ", "界", "  ", "│ ", "  "},
            {"  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  ", "│ ", "  "}};
        int index = i == 0 || i == 7 ? 0 : (i == 1 || i == 8 ? 1 : (i == 4 ? 2 : 3));
        if (i < 9)
            for (int j = 0; j < 19; j++)
                ch_mat[2 * i + 2][j] = str_template[index][j];
    }
    string str_template[19] = {"  ", "a ", "  ", "b ", "  ", "c ", "  ", "d ", "  ", "e ", "  ", "f ", "  ", "g ", "  ", "h ", "  ", "i ", "  "};
    for (int i = 0; i < 19; i++)
        ch_mat[20][i] = str_template[i];
    for (int k = 0; k < n; k++)
    {
        int i = 2 * hlt[k].i + 1, j = 2 * hlt[k].j + 1;
        string zero = "";
        ch_mat[i - 1][j - 1] = ch_mat[i - 1][j - 1] == "╲ " || ch_mat[i - 1][j - 1] == "╱ " || ch_mat[i - 1][j - 1] == "┘ " ? " ┌" : zero + ch_mat[i - 1][j - 1][0] + "┌";
        ch_mat[i + 1][j + 1] = ch_mat[i + 1][j + 1] == "╲ " || ch_mat[i + 1][j + 1] == "╱ " ? "┘ " : zero + "┘" + ch_mat[i + 1][j + 1][0];
    }
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 19; j++)
            cout << ch_mat[i][j].c_str();
        cout << endl;
    }
}

/* 棋盘与FEN串转换函数
 * 参数: 棋盘结构体
 * 返回值: FEN串描述棋盘部分
 */
string brd2str(chessboard brd)
{
    string s = "";
    for (int i = 0; i < 10; i++)
    {
        int j = 0;
        while (j < 9)
            if (brd[i][j] == 0)
            {
                int num = 0;
                while (j < 9 && brd[i][j] == 0)
                {
                    j++;
                    num++;
                }
                char s_tmp[32];
                itoa(num, s_tmp, 10);
                s += s_tmp;
            }
            else
            {
                s += charset[brd[i][j]];
                j++;
            }
        if (i < 9)
            s += '/';
    }
    return s;
}

/* FEN串与棋盘转换函数
 * 参数: FEN字符串第一部分
 * 返回值: 棋盘结构体, 包含是否合法
 */
chessboard str2brd(string s)
{
    int p = 0;
    int i = 0;
    chessboard brd;
    brd.valid = 0;
    while (p < s.length())
    {
        int j = 0;
        while (p < s.length() && j < 9)
        {
            if (s[p] >= '1' && s[p] <= '9')
            {
                for (int k = 0; k < s[p] - '0'; k++)
                    brd[i][j++] = 0;
                if (j > 9)
                    return brd;
            }
            else if (no_(s[p]) == 0)
                return brd;
            else
                brd[i][j++] = no_(s[p]);
            p++;
        }
        if (i < 9 && p < s.length() && s[p] == '/')
        {
            i++;
            p++;
        }
        else if (i == 9 && p == s.length())
            i++;
        else
            return brd;
    }
    if (p != s.length() || !isvalid(brd))
        return brd;
    brd.valid = 1;
    return brd;
}

/* 移动与字符串转换函数
 * 参数: 移动结构体
 * 返回值: 标准长度为4的字符串
 */
string mov2str(movement mov)
{
    string s = "";
    s += 'a' + mov.b.j;
    s += '0' + 9 - mov.b.i;
    s += 'a' + mov.e.j;
    s += '0' + 9 - mov.e.i;
    return s;
}

/* 字符串与移动结构体转换函数
 * 参数: 长度为4的标准字符串
 * 返回值: 移动结构体, 包含是否合法
 */
movement str2mov(string s)
{
    movement mov;
    mov.valid = 0;
    if (s.length() != 4 ||
        s[0] < 'a' || s[0] > 'i' || s[1] < '0' || s[1] > '9' ||
        s[2] < 'a' || s[2] > 'i' || s[3] < '0' || s[3] > '9')
        return mov;
    else
    {
        mov.b.i = 9 - (s[1] - '0');
        mov.b.j = s[0] - 'a';
        mov.e.i = 9 - (s[3] - '0');
        mov.e.j = s[2] - 'a';
        mov.valid = 1;
    }
    return mov;
}

/* 将军检查函数
 * 参数: 棋盘结构体
 * 返回值: 1: 红方将军; 2: 黑方将军; 3: 同时将军; -1: 错误
 */
int ischeck(chessboard brd)
{
    position pos_k = {-1, -1}, pos_K = {-1, -1};
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 9; j++)
            if (charset[brd[i][j]] == 'k')
                pos_k.i = i, pos_k.j = j;
            else if (charset[brd[i][j]] == 'K')
                pos_K.i = i, pos_K.j = j;
    if (pos_k.i < 0 || pos_k.j < 0 || pos_K.i < 0 || pos_K.j < 0)
        return -1;
    if (pos_k.j == pos_K.j)
    {
        bool flag = 1;
        for (int i = pos_k.i + 1; i < pos_K.i; i++)
            if (brd[i][pos_k.j] != 0)
            {
                flag = 0;
                break;
            }
        if (flag)
            return 3;
    }
    int r_check = 0, b_check = 0;
    int didj[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int idx = 0; idx < 4; idx++)
    {
        if (r_check)
            break;
        int di = didj[idx][0], dj = didj[idx][1];
        int i = pos_k.i + di, j = pos_k.j + dj;
        int num = 0;
        while (!r_check && i >= 0 && i <= 9 && j >= 0 && j <= 8)
        {
            if (num == 0 && charset[brd[i][j]] == 'R')
            {
                r_check = 1;
                break;
            }
            else if (num == 1 and charset[brd[i][j]] == 'C')
            {
                r_check = 1;
                break;
            }
            else if (brd[i][j] != 0)
                num++;
            if (num > 1)
                break;
            i += di;
            j += dj;
        }
    }
    if (charset[brd[pos_k.i][pos_k.j - 1]] == 'P' ||
        charset[brd[pos_k.i][pos_k.j + 1]] == 'P' ||
        charset[brd[pos_k.i + 1][pos_k.j]] == 'P')
        r_check = 1;
    int dN[8][2] = {{1, 2}, {-1, 2}, {1, -2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
    for (int idx = 0; idx < 8; idx++)
    {
        if (r_check)
            break;
        int i = pos_k.i + dN[idx][0], j = pos_k.j + dN[idx][1];
        if (i < 10 && i >= 0 && j < 9 && j >= 0)
            if (charset[brd[i][j]] == 'N')
                if (idx < 4 && brd[i][(j + pos_k.j) / 2] == 0)
                    r_check = 1;
                else if (idx > 3 && brd[(i + pos_k.i) / 2][j] == 0)
                    r_check = 1;
    }
    for (int idx = 0; idx < 4; idx++)
    {
        if (b_check)
            break;
        int di = didj[idx][0], dj = didj[idx][1];
        int i = pos_K.i + di, j = pos_K.j + dj;
        int num = 0;
        while (!b_check && i >= 0 && i <= 9 && j >= 0 && j <= 8)
        {
            if (num == 0 && charset[brd[i][j]] == 'r')
            {
                b_check = 1;
                break;
            }
            else if (num == 1 and charset[brd[i][j]] == 'c')
            {
                b_check = 1;
                break;
            }
            else if (brd[i][j] != 0)
                num++;
            if (num > 1)
                break;
            i += di;
            j += dj;
        }
    }
    if (charset[brd[pos_K.i][pos_K.j - 1]] == 'p' ||
        charset[brd[pos_K.i][pos_K.j + 1]] == 'p' ||
        charset[brd[pos_K.i - 1][pos_K.j]] == 'p')
        b_check = 1;
    int dn[8][2] = {{1, 2}, {-1, 2}, {1, -2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
    for (int idx = 0; idx < 8; idx++)
    {
        if (b_check)
            break;
        int i = pos_K.i + dn[idx][0], j = pos_K.j + dn[idx][1];
        if (i < 10 && i >= 0 && j < 9 && j >= 0)
            if (charset[brd[i][j]] == 'n')
                if (idx < 4 && brd[i][(j + pos_K.j) / 2] == 0)
                    b_check = 1;
                else if (idx > 3 && brd[(i + pos_K.i) / 2][j] == 0)
                    b_check = 1;
    }
    return r_check + (b_check << 1);
}

/* 生成可能的移动列表
 * 参数: 棋盘结构体, 走棋轮次以及待计算棋子位置(默认计算所有棋子)
 * 返回值: vector<string>类, 包含若干满足移动规则的字符串
 */
vector<string> possible_move(chessboard brd, int turn, position piece)
{
    vector<string> s_set;
    vector<position> pieces;
    if (piece.i < 0 || piece.i > 9 || piece.j < 0 || piece.j > 8)
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 9; j++)
            {
                position piece_new = {i, j};
                pieces.push_back(piece_new);
            }
    else
        pieces.push_back(piece);
    for (int idx = 0; idx < pieces.size(); idx++)
    {
        int i = pieces[idx].i, j = pieces[idx].j;
        if (brd[i][j] == 0 ||
            brd[i][j] < 8 && turn == 1 ||
            brd[i][j] > 7 && turn == 2)
            continue;
        char ch = charset[brd[i][j]];
        if (ch == 'R' || ch == 'r' || ch == 'C' || ch == 'c')
        {
            int directions[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
            for (int subidx = 0; subidx < 4; subidx++)
            {
                int direction[2] = {directions[subidx][0], directions[subidx][1]};
                int num = 0, num_eat = ch == 'R' || ch == 'r' ? 0 : 1;
                int i_d = i + direction[0], j_d = j + direction[1];
                while (i_d >= 0 && j_d >= 0 && i_d <= 9 && j_d <= 8)
                {
                    chessboard brd_tmp = brd;
                    brd_tmp[i][j] = 0;
                    brd_tmp[i_d][j_d] = brd[i][j];
                    movement mov = {1, {i, j}, {i_d, j_d}};
                    int check = ischeck(brd_tmp);
                    if (brd[i_d][j_d] == 0)
                    {
                        if (num == 0 && (check == 0 || check == turn))
                            s_set.push_back(mov2str(mov));
                    }
                    else
                    {
                        if ((brd[i_d][j_d] < 8 && turn == 1 ||
                             brd[i_d][j_d] > 7 && turn == 2) &&
                            num == num_eat)
                        {
                            if (check == 0 || check == turn)
                                s_set.push_back(mov2str(mov));
                            break;
                        }
                        num += 1;
                    }
                    i_d += direction[0];
                    j_d += direction[1];
                    // if (num > num_eat)
                    //     break;
                }
            }
        }
        else
        {
            int directions[8][2], num;
            if (ch == 'N' || ch == 'n')
            {
                num = 8;
                int tmp[8][2] = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
                memcpy(directions, tmp, num * sizeof(int) * 2);
            }
            else if (ch == 'B' || ch == 'b')
            {
                num = 4;
                int tmp[4][2] = {{2, 2}, {2, -2}, {-2, 2}, {-2, -2}};
                memcpy(directions, tmp, num * sizeof(int) * 2);
            }
            else if (ch == 'A' || ch == 'a')
            {
                num = 4;
                int tmp[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
                memcpy(directions, tmp, num * sizeof(int) * 2);
            }
            else if (ch == 'K' || ch == 'k')
            {
                num = 4;
                int tmp[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                memcpy(directions, tmp, num * sizeof(int) * 2);
            }
            else
            {
                num = 1;
                int tmp[1][2] = {{1, 0}};
                if (ch == 'P')
                    tmp[0][0] = -1;
                memcpy(directions, tmp, num * sizeof(int) * 2);
            }
            if (ch == 'P' && i < 5 || ch == 'p' && i > 4)
            {
                num += 2;
                int tmp[2][2] = {{0, 1}, {0, -1}};
                memcpy(directions + 1, tmp, 2 * sizeof(int) * 2);
            }
            for (int idx = 0; idx < num; idx++)
            {
                int i_d = i + directions[idx][0], j_d = j + directions[idx][1];
                if (i_d < 0 || i_d > 9 || j_d < 0 || j_d > 8)
                    continue;
                if (ch == 'B' && i_d < 5 || ch == 'b' && i_d > 4 ||
                    (ch == 'A' || ch == 'K') && (i_d < 7 || j_d < 3 || j_d > 5) ||
                    (ch == 'a' || ch == 'k') && (i_d > 2 || j_d < 3 || j_d > 5) ||
                    ch == 'P' && i_d > 6 || ch == 'p' && i_d < 3)
                    continue;
                if (ch == 'P' && i_d > 4 && j % 2 != 0 ||
                    ch == 'p' && i_d < 5 && j % 2 != 0)
                    continue;
                if (brd[i_d][j_d] != 0 && (brd[i_d][j_d] < 8 && turn == 2 ||
                                           brd[i_d][j_d] > 7 && turn == 1 ||
                                           brd[i_d][j_d] == 0))
                    continue;
                if ((ch == 'N' || ch == 'n') &&
                    (abs(directions[idx][0]) == 2 && brd[(i_d + i) / 2][j] != 0 ||
                     abs(directions[idx][1]) == 2 && brd[i][(j_d + j) / 2]) != 0)
                    continue;
                if ((ch == 'B' || ch == 'b') && brd[(i_d + i) / 2][(j_d + j) / 2] != 0)
                    continue;
                chessboard brd_tmp = brd;
                brd_tmp[i][j] = 0;
                brd_tmp[i_d][j_d] = brd[i][j];
                movement mov = {1, {i, j}, {i_d, j_d}};
                int check = ischeck(brd_tmp);
                if (check == 0 || check == turn)
                    s_set.push_back(mov2str(mov));
            }
        }
    }
    return s_set;
}

/* 移动棋子修改棋盘函数
 * 参数: 棋盘以及合法的字符串
 * 返回值: 无
 */
void move(chessboard &brd, string s)
{
    movement mov = str2mov(s);
    brd[mov.e.i][mov.e.j] = brd[mov.b.i][mov.b.j];
    brd[mov.b.i][mov.b.j] = 0;
}

/* 测试函数, 通过标准输入输出打印棋盘和所有可能的移动, 以及读取字符串改变棋盘状态
 * 参数: 无
 * 返回值: 无
 */
void play()
{
    chessboard brd = str2brd(ini_state);
    int turn = 1;
    system("cls");
    print_board(brd);
    while (1)
    {
        vector<string> s_set = possible_move(brd, turn);
        for (int i = 0; i < s_set.size(); i++)
            cout << s_set[i] << (i % 10 == 9 ? "\n" : "  ");
        cout << endl;
        if (s_set.empty())
        {
            cout << "Player " << (turn == 1 ? "BLACK" : "RED") << " wins!";
            break;
        }
        string s;
        bool flag = 1;
        while (flag)
        {
            cin >> s;
            for (int i = 0; i < s_set.size(); i++)
                if (s == s_set[i])
                {
                    flag = 0;
                    break;
                }
            if (flag)
                cout << "Invalid input\n";
        }
        move(brd, s);
        movement mov = str2mov(s);
        position hlt[2] = {mov.b, mov.e};
        system("cls");
        print_board(brd, 2, hlt);
        turn = 3 - turn;
    }
}