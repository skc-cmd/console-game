#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include <string>

using namespace std;

// ==================== 通用工具函数 ====================
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// ==================== 井字棋 ====================
class TicTacToe {
private:
    char board[3][3];
    char player;
    char computer;
    bool gameOver;
    int moveCount;
    int cursorX, cursorY;
    bool needRefresh;
    HANDLE hConsole;

public:
    TicTacToe() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        initGame();
    }

    void initGame() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = ' ';
            }
        }
        player = 'X';
        computer = 'O';
        gameOver = false;
        moveCount = 0;
        cursorX = 0;
        cursorY = 0;
        needRefresh = true;
    }

    void draw() {
        if (!needRefresh) return;
        needRefresh = false;
        
        system("cls");
        cout << "  =========== 井字棋 ===========" << endl;
        cout << "  你: X    电脑: O" << endl;
        cout << "  WASD/方向键移动  回车落子" << endl;
        cout << "  R重新开始  ESC返回菜单" << endl;
        cout << "  -----------------------------" << endl;
        
        cout << "      1   2   3" << endl;
        cout << "    +---+---+---+" << endl;
        for (int i = 0; i < 3; i++) {
            cout << "  " << i + 1 << " | ";
            for (int j = 0; j < 3; j++) {
                // 选中高亮（不管有没有棋子）
                if (cursorX == i && cursorY == j && !gameOver) {
                    cout << "[" << board[i][j] << "]";
                } else {
                    cout << " " << board[i][j] << " ";
                }
                cout << "| ";
            }
            cout << endl;
            cout << "    +---+---+---+" << endl;
        }
        cout << "  -----------------------------" << endl;
        
        if (gameOver) {
            if (checkWin(player)) {
                cout << "  ?? 你赢了！" << endl;
            } else if (checkWin(computer)) {
                cout << "  ?? 电脑赢了！" << endl;
            } else if (moveCount >= 9) {
                cout << "  平局！" << endl;
            }
            cout << "  按 R 重新开始" << endl;
        }
    }

    bool checkWin(char symbol) {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol)
                return true;
        }
        for (int j = 0; j < 3; j++) {
            if (board[0][j] == symbol && board[1][j] == symbol && board[2][j] == symbol)
                return true;
        }
        if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol)
            return true;
        if (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol)
            return true;
        return false;
    }

    bool isBoardFull() {
        return moveCount >= 9;
    }

    int evaluate() {
        if (checkWin(computer)) return 10;
        if (checkWin(player)) return -10;
        return 0;
    }

    int minimax(bool isMaximizing, int depth) {
        int score = evaluate();
        if (score == 10) return score - depth;
        if (score == -10) return score + depth;
        if (isBoardFull()) return 0;
        
        if (isMaximizing) {
            int best = -1000;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        board[i][j] = computer;
                        best = max(best, minimax(false, depth + 1));
                        board[i][j] = ' ';
                    }
                }
            }
            return best;
        } else {
            int best = 1000;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        board[i][j] = player;
                        best = min(best, minimax(true, depth + 1));
                        board[i][j] = ' ';
                    }
                }
            }
            return best;
        }
    }

    pair<int, int> getBestMove() {
        int bestVal = -1000;
        pair<int, int> bestMove = {-1, -1};
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = computer;
                    int moveVal = minimax(false, 0);
                    board[i][j] = ' ';
                    
                    if (moveVal > bestVal) {
                        bestVal = moveVal;
                        bestMove = {i, j};
                    }
                }
            }
        }
        return bestMove;
    }

    void playerMove() {
        if (board[cursorX][cursorY] != ' ') return;
        
        board[cursorX][cursorY] = player;
        moveCount++;
        needRefresh = true;
        
        if (checkWin(player) || isBoardFull()) {
            gameOver = true;
            needRefresh = true;
            return;
        }
        
        pair<int, int> bestMove = getBestMove();
        if (bestMove.first != -1) {
            board[bestMove.first][bestMove.second] = computer;
            moveCount++;
            needRefresh = true;
            
            if (checkWin(computer) || isBoardFull()) {
                gameOver = true;
                needRefresh = true;
                return;
            }
        }
    }

    void run() {
        initGame();
        while (true) {
            draw();
            
            if (gameOver) {
                if (_kbhit()) {
                    char key = _getch();
                    if (tolower(key) == 'r') {
                        initGame();
                        needRefresh = true;
                    } else if (key == 27) {
                        return;
                    }
                }
                Sleep(50);
                continue;
            }
            
            if (_kbhit()) {
                char key = _getch();
                bool moved = false;
                if (key == -32) {
                    key = _getch();
                    switch (key) {
                        case 72: cursorX = max(0, cursorX - 1); moved = true; break;
                        case 80: cursorX = min(2, cursorX + 1); moved = true; break;
                        case 75: cursorY = max(0, cursorY - 1); moved = true; break;
                        case 77: cursorY = min(2, cursorY + 1); moved = true; break;
                    }
                } else {
                    switch (tolower(key)) {
                        case 'w': cursorX = max(0, cursorX - 1); moved = true; break;
                        case 's': cursorX = min(2, cursorX + 1); moved = true; break;
                        case 'a': cursorY = max(0, cursorY - 1); moved = true; break;
                        case 'd': cursorY = min(2, cursorY + 1); moved = true; break;
                        case 13: playerMove(); break;
                        case 'r':
                            initGame();
                            needRefresh = true;
                            break;
                        case 27:
                            return;
                    }
                }
                if (moved) needRefresh = true;
            }
            Sleep(30);
        }
    }
};

// ==================== 五子棋 ====================
class Gomoku {
private:
    int size;
    vector<vector<char>> board;
    char player;
    char computer;
    bool gameOver;
    int moveCount;
    int cursorX, cursorY;
    bool needRefresh;
    HANDLE hConsole;

public:
    Gomoku(int s = 8) {
        size = s;
        player = 'X';
        computer = 'O';
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        initGame();
    }

    void setSize(int s) {
        if (s >= 5 && s <= 30) {
            size = s;
        }
    }

    void initGame() {
        board.assign(size, vector<char>(size, ' '));
        gameOver = false;
        moveCount = 0;
        cursorX = size / 2;
        cursorY = size / 2;
        needRefresh = true;
    }

    void draw() {
        if (!needRefresh) return;
        needRefresh = false;
        
        system("cls");
        
        // 隐藏光标
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        cout << "  =========== 五子棋 ===========" << endl;
        cout << "  你: X    电脑: O" << endl;
        cout << "  棋盘: " << size << "x" << size << endl;
        cout << "  WASD/方向键移动  回车落子" << endl;
        cout << "  R重新开始  ESC返回菜单" << endl;
        cout << "  -----------------------------" << endl;
        
        // 列号
        cout << "     ";
        for (int j = 0; j < size; j++) {
            if (j < 9) cout << j + 1 << "  ";
            else cout << j + 1 << " ";
        }
        cout << endl;
        
        cout << "    +";
        for (int j = 0; j < size; j++) {
            cout << "--+";
        }
        cout << endl;
        
        for (int i = 0; i < size; i++) {
            if (i < 9) cout << " " << i + 1 << " |";
            else cout << i + 1 << " |";
            
            for (int j = 0; j < size; j++) {
                // 选中高亮 - 不管有没有棋子都显示选择框
                if (cursorX == i && cursorY == j && !gameOver) {
                    cout << "[" << board[i][j] << "]";
                } else {
                    cout << " " << board[i][j] << " ";
                }
                cout << "|";
            }
            cout << endl;
            
            cout << "    +";
            for (int j = 0; j < size; j++) {
                cout << "--+";
            }
            cout << endl;
        }
        
        cout << "  -----------------------------" << endl;
        cout << "  当前光标: (" << cursorX + 1 << ", " << cursorY + 1 << ")" << endl;
        
        if (gameOver) {
            if (checkWin(player)) {
                cout << "  ?? 你赢了！" << endl;
            } else if (checkWin(computer)) {
                cout << "  ?? 电脑赢了！" << endl;
            } else if (moveCount >= size * size) {
                cout << "  平局！" << endl;
            }
            cout << "  按 R 重新开始" << endl;
        }
    }

    bool checkWin(char symbol) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] != symbol) continue;
                
                if (j + 4 < size) {
                    bool win = true;
                    for (int k = 1; k < 5; k++) {
                        if (board[i][j + k] != symbol) { win = false; break; }
                    }
                    if (win) return true;
                }
                
                if (i + 4 < size) {
                    bool win = true;
                    for (int k = 1; k < 5; k++) {
                        if (board[i + k][j] != symbol) { win = false; break; }
                    }
                    if (win) return true;
                }
                
                if (i + 4 < size && j + 4 < size) {
                    bool win = true;
                    for (int k = 1; k < 5; k++) {
                        if (board[i + k][j + k] != symbol) { win = false; break; }
                    }
                    if (win) return true;
                }
                
                if (i - 4 >= 0 && j + 4 < size) {
                    bool win = true;
                    for (int k = 1; k < 5; k++) {
                        if (board[i - k][j + k] != symbol) { win = false; break; }
                    }
                    if (win) return true;
                }
            }
        }
        return false;
    }

    bool isBoardFull() {
        return moveCount >= size * size;
    }

    int evaluatePosition(int row, int col, char symbol) {
        int score = 0;
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (int d = 0; d < 4; d++) {
            int count = 1;
            for (int step = 1; step < 5; step++) {
                int nr = row + directions[d][0] * step;
                int nc = col + directions[d][1] * step;
                if (nr >= 0 && nr < size && nc >= 0 && nc < size && board[nr][nc] == symbol)
                    count++;
                else if (nr >= 0 && nr < size && nc >= 0 && nc < size && board[nr][nc] == ' ')
                    break;
                else
                    break;
            }
            for (int step = 1; step < 5; step++) {
                int nr = row - directions[d][0] * step;
                int nc = col - directions[d][1] * step;
                if (nr >= 0 && nr < size && nc >= 0 && nc < size && board[nr][nc] == symbol)
                    count++;
                else if (nr >= 0 && nr < size && nc >= 0 && nc < size && board[nr][nc] == ' ')
                    break;
                else
                    break;
            }
            
            if (count >= 5) score += 100;
            else if (count == 4) score += 30;
            else if (count == 3) score += 10;
            else if (count == 2) score += 3;
        }
        return score;
    }

    pair<int, int> getBestMove() {
        int bestScore = -1;
        pair<int, int> bestMove = {-1, -1};
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] != ' ') continue;
                
                bool hasNeighbor = false;
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < size && nj >= 0 && nj < size && board[ni][nj] != ' ') {
                            hasNeighbor = true;
                            break;
                        }
                    }
                    if (hasNeighbor) break;
                }
                if (!hasNeighbor) continue;
                
                int score = evaluatePosition(i, j, computer);
                int playerScore = evaluatePosition(i, j, player);
                score += playerScore * 0.8;
                
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = {i, j};
                }
            }
        }
        
        if (bestMove.first == -1) {
            int center = size / 2;
            for (int offset = 0; offset < size / 2; offset++) {
                for (int di = -offset; di <= offset; di++) {
                    for (int dj = -offset; dj <= offset; dj++) {
                        int ni = center + di, nj = center + dj;
                        if (ni >= 0 && ni < size && nj >= 0 && nj < size && board[ni][nj] == ' ') {
                            return {ni, nj};
                        }
                    }
                }
            }
        }
        
        return bestMove;
    }

    void playerMove() {
        if (board[cursorX][cursorY] != ' ') return;
        
        board[cursorX][cursorY] = player;
        moveCount++;
        needRefresh = true;
        
        if (checkWin(player) || isBoardFull()) {
            gameOver = true;
            needRefresh = true;
            return;
        }
        
        pair<int, int> bestMove = getBestMove();
        if (bestMove.first != -1) {
            board[bestMove.first][bestMove.second] = computer;
            moveCount++;
            needRefresh = true;
            
            if (checkWin(computer) || isBoardFull()) {
                gameOver = true;
                needRefresh = true;
                return;
            }
        }
    }

    void run() {
        initGame();
        while (true) {
            draw();
            
            if (gameOver) {
                if (_kbhit()) {
                    char key = _getch();
                    if (tolower(key) == 'r') {
                        initGame();
                        needRefresh = true;
                    } else if (key == 27) {
                        return;
                    }
                }
                Sleep(50);
                continue;
            }
            
            if (_kbhit()) {
                char key = _getch();
                bool moved = false;
                if (key == -32) {
                    key = _getch();
                    switch (key) {
                        case 72: cursorX = max(0, cursorX - 1); moved = true; break;
                        case 80: cursorX = min(size - 1, cursorX + 1); moved = true; break;
                        case 75: cursorY = max(0, cursorY - 1); moved = true; break;
                        case 77: cursorY = min(size - 1, cursorY + 1); moved = true; break;
                    }
                } else {
                    switch (tolower(key)) {
                        case 'w': cursorX = max(0, cursorX - 1); moved = true; break;
                        case 's': cursorX = min(size - 1, cursorX + 1); moved = true; break;
                        case 'a': cursorY = max(0, cursorY - 1); moved = true; break;
                        case 'd': cursorY = min(size - 1, cursorY + 1); moved = true; break;
                        case 13: playerMove(); break;
                        case 'r':
                            initGame();
                            needRefresh = true;
                            break;
                        case 27:
                            return;
                    }
                }
                if (moved) needRefresh = true;
            }
            Sleep(30);
        }
    }
};

// ==================== 主程序 ====================
int main() {
    system("cls");
    hideCursor();
    
    while (true) {
        system("cls");
        cout << "  =========================================" << endl;
        cout << "  ||     井字棋 & 五子棋 合辑          ||" << endl;
        cout << "  =========================================" << endl;
        cout << "  ||  1. 井字棋 (3x3)                  ||" << endl;
        cout << "  ||  2. 五子棋 (8x8)                  ||" << endl;
        cout << "  ||  3. 五子棋 (自定义棋盘大小)      ||" << endl;
        cout << "  ||  0. 退出                         ||" << endl;
        cout << "  =========================================" << endl;
        cout << "  请选择: ";
        
        char choice = _getch();
        
        if (choice == '0') {
            system("cls");
            cout << "  感谢游玩！再见！" << endl;
            break;
        }
        else if (choice == '1') {
            TicTacToe game;
            game.run();
        }
        else if (choice == '2') {
            Gomoku game(8);
            game.run();
        }
        else if (choice == '3') {
            int size;
            system("cls");
            cout << "  =========================================" << endl;
            cout << "  ||     五子棋 - 自定义棋盘          ||" << endl;
            cout << "  =========================================" << endl;
            cout << "  请输入棋盘大小 (5-30): ";
            cin >> size;
            
            if (size < 5) size = 5;
            if (size > 30) size = 30;
            
            Gomoku game(size);
            game.run();
        }
    }
    
    return 0;
}
