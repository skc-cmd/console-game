#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// ==================== 通用工具函数 ====================
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// ==================== 数字华容道 ====================
class SlidingPuzzle {
private:
    int size;
    vector<vector<int>> board;
    int emptyRow, emptyCol;
    int cursorRow, cursorCol;
    int moves;
    bool gameOver;
    bool needRefresh;
    string screenBuffer;
    HANDLE hConsole;

public:
    SlidingPuzzle(int s = 4) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        size = s;
        initGame();
    }

    void initGame() {
        board.assign(size, vector<int>(size, 0));
        int num = 1;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                board[i][j] = num++;
            }
        }
        board[size-1][size-1] = 0;
        
        emptyRow = size - 1;
        emptyCol = size - 1;
        cursorRow = 0;
        cursorCol = 0;
        moves = 0;
        gameOver = false;
        needRefresh = true;
        screenBuffer = "";
        
        shuffleBoard();
    }

    bool isSolvable(vector<int>& flat) {
        int inversions = 0;
        for (int i = 0; i < flat.size(); i++) {
            if (flat[i] == 0) continue;
            for (int j = i + 1; j < flat.size(); j++) {
                if (flat[j] == 0) continue;
                if (flat[i] > flat[j]) inversions++;
            }
        }
        
        if (size % 2 == 1) {
            return inversions % 2 == 0;
        } else {
            int emptyRowFromBottom = size - (emptyRow + 1);
            return (inversions + emptyRowFromBottom) % 2 == 0;
        }
    }

    void shuffleBoard() {
        vector<int> flat;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                flat.push_back(board[i][j]);
            }
        }
        
        do {
            random_shuffle(flat.begin(), flat.end());
        } while (!isSolvable(flat));
        
        int idx = 0;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                board[i][j] = flat[idx++];
                if (board[i][j] == 0) {
                    emptyRow = i;
                    emptyCol = j;
                }
            }
        }
        
        needRefresh = true;
    }

    bool checkWin() {
        int num = 1;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (i == size - 1 && j == size - 1) {
                    if (board[i][j] != 0) return false;
                } else {
                    if (board[i][j] != num++) return false;
                }
            }
        }
        return true;
    }

    bool tryMove(int row, int col) {
        if (row < 0 || row >= size || col < 0 || col >= size) return false;
        if (board[row][col] == 0) return false;
        
        int dr = abs(row - emptyRow);
        int dc = abs(col - emptyCol);
        
        if ((dr == 1 && dc == 0) || (dr == 0 && dc == 1)) {
            board[emptyRow][emptyCol] = board[row][col];
            board[row][col] = 0;
            emptyRow = row;
            emptyCol = col;
            moves++;
            needRefresh = true;
            
            if (checkWin()) {
                gameOver = true;
                needRefresh = true;
            }
            return true;
        }
        return false;
    }

    string buildScreen() {
        stringstream ss;
        
        int maxNum = size * size - 1;
        int numWidth = to_string(maxNum).length();
        int cellWidth = numWidth + 2;
        if (cellWidth < 4) cellWidth = 4;
        
        ss << "  =========== 数字华容道 ===========" << endl;
        ss << "  大小: " << size << "x" << size << "  步数: " << moves << endl;
        ss << "  WASD/方向键移动  [ ] 选择框  空格/回车移动" << endl;
        ss << "  R重新开始  ESC返回菜单" << endl;
        ss << "  -----------------------------" << endl;
        
        // 列号
        ss << "     ";
        for (int j = 0; j < size; j++) {
            string label = to_string(j + 1);
            int padding = (cellWidth - label.length()) / 2;
            ss << string(padding, ' ') << label << string(cellWidth - padding - label.length(), ' ');
        }
        ss << endl;
        
        // 顶部分隔线
        ss << "    +";
        for (int j = 0; j < size; j++) {
            ss << string(cellWidth, '-') << "+";
        }
        ss << endl;
        
        for (int i = 0; i < size; i++) {
            // 行号
            string rowLabel = to_string(i + 1);
            ss << "  " << rowLabel << " |";
            
            for (int j = 0; j < size; j++) {
                int val = board[i][j];
                bool isCursor = (cursorRow == i && cursorCol == j);
                bool isEmpty = (val == 0);
                
                string content;
                if (isEmpty) {
                    content = string(cellWidth, ' ');
                } else {
                    string numStr = to_string(val);
                    int padding = (cellWidth - numStr.length()) / 2;
                    content = string(padding, ' ') + numStr + string(cellWidth - padding - numStr.length(), ' ');
                }
                
                if (isCursor) {
                    // 光标位置 - 用方括号包起来（空格用特殊符号）
                    if (isEmpty) {
                        ss << "[";
                        for (int k = 0; k < cellWidth; k++) {
                            if (k == cellWidth / 2) {
                                ss << "?";  // 空格用方块符号
                            } else {
                                ss << " ";
                            }
                        }
                        ss << "]";
                    } else {
                        ss << "[" << content << "]";
                    }
                } else {
                    if (isEmpty) {
                        ss << " " << content << " ";
                    } else {
                        ss << " " << content << " ";
                    }
                }
                ss << "|";
            }
            ss << endl;
            
            // 行分隔线
            ss << "    +";
            for (int j = 0; j < size; j++) {
                ss << string(cellWidth, '-') << "+";
            }
            ss << endl;
        }
        
        ss << "  -----------------------------" << endl;
        ss << "  空格位置: (" << emptyRow + 1 << ", " << emptyCol + 1 << ")" << endl;
        
        if (!gameOver) {
            if (board[cursorRow][cursorCol] == 0) {
                ss << "  ? 光标在空格上（不能移动）" << endl;
            } else {
                int dr = abs(cursorRow - emptyRow);
                int dc = abs(cursorCol - emptyCol);
                if ((dr == 1 && dc == 0) || (dr == 0 && dc == 1)) {
                    ss << "  ? 按空格/回车移动 " << board[cursorRow][cursorCol] << " 到空格" << endl;
                } else {
                    ss << "  ? " << board[cursorRow][cursorCol] << " 不能移动（不相邻空格）" << endl;
                }
            }
        }
        
        if (gameOver) {
            ss << endl;
            ss << "  =========================================" << endl;
            ss << "  ||        ?? 恭喜通关！               ||" << endl;
            ss << "  ||        总步数: " << moves << "                       ||" << endl;
            ss << "  ||        按 R 重新开始               ||" << endl;
            ss << "  ||        按 ESC 返回菜单             ||" << endl;
            ss << "  =========================================" << endl;
        }
        
        return ss.str();
    }

    void draw() {
        if (!needRefresh) return;
        needRefresh = false;
        
        string newScreen = buildScreen();
        
        if (newScreen != screenBuffer) {
            system("cls");
            cout << newScreen;
            screenBuffer = newScreen;
        }
        
        hideCursor();
    }

    void processInput(char key) {
        if (gameOver) {
            if (tolower(key) == 'r') {
                initGame();
            }
            return;
        }
        
        int newRow = cursorRow;
        int newCol = cursorCol;
        bool moved = false;
        
        // 处理方向键和WASD
        switch (key) {
            case 'w':
            case 'W':
                newRow = max(0, cursorRow - 1);
                moved = true;
                break;
            case 's':
            case 'S':
                newRow = min(size - 1, cursorRow + 1);
                moved = true;
                break;
            case 'a':
            case 'A':
                newCol = max(0, cursorCol - 1);
                moved = true;
                break;
            case 'd':
            case 'D':
                newCol = min(size - 1, cursorCol + 1);
                moved = true;
                break;
            case 72:  // 上箭头
                newRow = max(0, cursorRow - 1);
                moved = true;
                break;
            case 80:  // 下箭头
                newRow = min(size - 1, cursorRow + 1);
                moved = true;
                break;
            case 75:  // 左箭头
                newCol = max(0, cursorCol - 1);
                moved = true;
                break;
            case 77:  // 右箭头
                newCol = min(size - 1, cursorCol + 1);
                moved = true;
                break;
            case 13:  // 回车
            case 32:  // 空格
                if (board[cursorRow][cursorCol] != 0) {
                    tryMove(cursorRow, cursorCol);
                }
                return;
            case 'r':
            case 'R':
                initGame();
                return;
            case 27:  // ESC
                return;
        }
        
        if (moved) {
            cursorRow = newRow;
            cursorCol = newCol;
            needRefresh = true;
        }
    }

    void run() {
        initGame();
        
        while (true) {
            draw();
            
            if (_kbhit()) {
                int key = _getch();
                if (key == 224) {  // 方向键的前缀
                    key = _getch();  // 获取实际方向键码
                    processInput(key);
                } else {
                    processInput(key);
                }
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
        cout << "  ||       数字华容道                   ||" << endl;
        cout << "  =========================================" << endl;
        cout << "  ||  1. 默认 4x4                      ||" << endl;
        cout << "  ||  2. 自定义大小                    ||" << endl;
        cout << "  ||  0. 退出                         ||" << endl;
        cout << "  =========================================" << endl;
        cout << "  规则：" << endl;
        cout << "  - WASD/方向键移动光标 [ ]" << endl;
        cout << "  - 空格/回车移动光标下的数字到空格" << endl;
        cout << "  - 只有相邻才能移动" << endl;
        cout << "  - 按顺序排列 1~n 即胜利" << endl;
        cout << "  =========================================" << endl;
        cout << "  请选择: ";
        
        char choice = _getch();
        
        if (choice == '0') {
            system("cls");
            cout << "  感谢游玩！再见！" << endl;
            break;
        }
        else if (choice == '1') {
            SlidingPuzzle game(4);
            game.run();
        }
        else if (choice == '2') {
            int size;
            system("cls");
            cout << "  =========================================" << endl;
            cout << "  ||       自定义棋盘                  ||" << endl;
            cout << "  =========================================" << endl;
            cout << "  请输入棋盘大小 (3-8): ";
            cin >> size;
            
            if (size < 3) size = 3;
            if (size > 8) size = 8;
            
            SlidingPuzzle game(size);
            game.run();
        }
    }
    
    return 0;
}
