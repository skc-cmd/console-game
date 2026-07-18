#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

class Minesweeper {
private:
    int rows, cols, mineCount;
    std::vector<std::vector<char>> board;
    std::vector<std::vector<bool>> mines;
    std::vector<std::vector<bool>> revealed;
    std::vector<std::vector<bool>> flagged;
    bool gameOver;
    bool firstMove;
    int flagCount;
    int cursorRow, cursorCol;

public:
    Minesweeper(int r = 9, int c = 9, int m = 10) 
        : rows(r), cols(c), mineCount(m), gameOver(false), firstMove(true), 
          flagCount(0), cursorRow(0), cursorCol(0) {
        initBoard();
    }

    void initBoard() {
        board.assign(rows, std::vector<char>(cols, '#'));
        mines.assign(rows, std::vector<bool>(cols, false));
        revealed.assign(rows, std::vector<bool>(cols, false));
        flagged.assign(rows, std::vector<bool>(cols, false));
    }

    void placeMines(int safeRow, int safeCol) {
        int placed = 0;
        while (placed < mineCount) {
            int r = rand() % rows;
            int c = rand() % cols;
            if (mines[r][c]) continue;
            if (abs(r - safeRow) <= 1 && abs(c - safeCol) <= 1) continue;
            mines[r][c] = true;
            placed++;
        }
    }

    int countAdjacentMines(int row, int col) const {
        int count = 0;
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int nr = row + dr, nc = col + dc;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && mines[nr][nc])
                    count++;
            }
        }
        return count;
    }

    void reveal(int row, int col) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) return;
        if (revealed[row][col] || flagged[row][col]) return;

        revealed[row][col] = true;

        if (mines[row][col]) {
            gameOver = true;
            return;
        }

        if (countAdjacentMines(row, col) == 0) {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = row + dr, nc = col + dc;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols 
                        && !revealed[nr][nc] && !flagged[nr][nc]) {
                        reveal(nr, nc);
                    }
                }
            }
        }
    }

    void toggleFlag(int row, int col) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) return;
        if (revealed[row][col]) return;
        
        if (flagged[row][col]) {
            flagged[row][col] = false;
            flagCount--;
        } else {
            flagged[row][col] = true;
            flagCount++;
        }
    }

    bool checkWin() const {
        if (gameOver) return false;
        int revealedCount = 0;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (revealed[i][j]) revealedCount++;
        return revealedCount == rows * cols - mineCount;
    }

    void gotoxy(int x, int y) {
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    void printBoard() {
        gotoxy(0, 0);
        
        // 列号 - 每个占3格
        std::cout << "\n    ";
        for (int j = 0; j < cols; j++) {
            if (j < 10) std::cout << " " << j << " ";
            else std::cout << j << " ";
        }
        std::cout << "\n    ";
        for (int j = 0; j < cols; j++) {
            std::cout << "---";
        }
        std::cout << "\n";

        for (int i = 0; i < rows; i++) {
            if (i < 10) std::cout << " " << i << " │";
            else std::cout << i << " │";
            
            for (int j = 0; j < cols; j++) {
                bool isCursor = (i == cursorRow && j == cursorCol);
                
                // 每个格子严格3个字符
                if (isCursor) std::cout << "[";  // 左边界
                else std::cout << " ";
                
                if (revealed[i][j]) {
                    if (mines[i][j]) {
                        std::cout << "* ";  // 地雷
                    } else {
                        int cnt = countAdjacentMines(i, j);
                        if (cnt == 0) {
                            std::cout << "  ";  // 空格
                        } else {
                            std::cout << cnt << " ";  // 数字
                        }
                    }
                } else if (flagged[i][j]) {
                    std::cout << "F ";  // 标记
                } else {
                    std::cout << "# ";  // 未翻开
                }
                
                if (isCursor) std::cout << "]";  // 右边界
                else std::cout << " ";
            }
            if (i < 10) std::cout << "│" << i;
            else std::cout << "│" << i;
            std::cout << "\n";
        }

        std::cout << "    ";
        for (int j = 0; j < cols; j++) {
            std::cout << "---";
        }
        std::cout << "\n    ";
        for (int j = 0; j < cols; j++) {
            if (j < 10) std::cout << " " << j << " ";
            else std::cout << j << " ";
        }
        std::cout << "\n\n";
        
        std::cout << "?? 剩余: " << mineCount - flagCount << "  已标记: " << flagCount << "\n";
        std::cout << "?? 进度: " << countRevealed() << "/" << rows * cols - mineCount << "\n";
        std::cout << "\n[方向键] 移动  [空格] 翻开  [F] 标记  [Q] 退出\n";
    }

    int countRevealed() const {
        int count = 0;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (revealed[i][j]) count++;
        return count;
    }

    void revealAllMines() {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (mines[i][j]) revealed[i][j] = true;
            }
        }
    }

    void play() {
        srand(static_cast<unsigned>(time(nullptr)));
        
        system("cls");
        
        std::cout << "\n\033[1;36m";
        std::cout << "  ╔══════════════════════════════════════╗\n";
        std::cout << "  ║          ?? 扫雷游戏 ??            ║\n";
        std::cout << "  ╚══════════════════════════════════════╝\n";
        std::cout << "\033[0m\n";
        
        printBoard();

        while (!gameOver) {
            if (checkWin()) {
                printBoard();
                std::cout << "\n\033[1;32m";
                std::cout << "  ╔══════════════════════════════════════╗\n";
                std::cout << "  ║     ???? 恭喜你赢了！ ????          ║\n";
                std::cout << "  ╚══════════════════════════════════════╝\n";
                std::cout << "\033[0m\n";
                _sleep(2000);
                return;
            }

            int key = _getch();
            
            if (key == 224) {  // 方向键
                key = _getch();
                switch (key) {
                    case 72: cursorRow = (cursorRow - 1 + rows) % rows; break;
                    case 80: cursorRow = (cursorRow + 1) % rows; break;
                    case 75: cursorCol = (cursorCol - 1 + cols) % cols; break;
                    case 77: cursorCol = (cursorCol + 1) % cols; break;
                }
                printBoard();
            } 
            else if (key == ' ' || key == 13) {
                int r = cursorRow, c = cursorCol;
                
                if (flagged[r][c]) {
                    gotoxy(0, rows + 6);
                    std::cout << "? 该位置已标记为地雷！按 F 取消标记    ";
                    continue;
                }

                if (revealed[r][c]) {
                    gotoxy(0, rows + 6);
                    std::cout << "? 该位置已翻开                    ";
                    continue;
                }

                if (firstMove) {
                    placeMines(r, c);
                    firstMove = false;
                }

                reveal(r, c);
                printBoard();

                if (gameOver) {
                    revealAllMines();
                    printBoard();
                    std::cout << "\n\033[1;31m";
                    std::cout << "  ╔══════════════════════════════════════╗\n";
                    std::cout << "  ║     ?? 踩到地雷！游戏结束 ??        ║\n";
                    std::cout << "  ╚══════════════════════════════════════╝\n";
                    std::cout << "\033[0m\n";
                    _sleep(2000);
                    return;
                }
            }
            else if (key == 'f' || key == 'F') {
                toggleFlag(cursorRow, cursorCol);
                printBoard();
            }
            else if (key == 'q' || key == 'Q') {
                std::cout << "\n游戏已退出。\n";
                return;
            }
        }
    }
};

int main() {
    int rows = 9, cols = 9, mines = 10;
    
    std::cout << "\n\033[1;33m";
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         选择游戏难度                 ║\n";
    std::cout << "  ╠══════════════════════════════════════╣\n";
    std::cout << "  ║  1. 初级   (9×9,   10地雷)          ║\n";
    std::cout << "  ║  2. 中级   (16×16, 40地雷)          ║\n";
    std::cout << "  ║  3. 高级   (16×30, 99地雷)          ║\n";
    std::cout << "  ║  4. 自定义                          ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n";
    std::cout << "\033[0m";
    std::cout << "请输入数字 (1-4): ";
    
    int choice;
    std::cin >> choice;

    switch (choice) {
        case 1: rows = 9; cols = 9; mines = 10; break;
        case 2: rows = 16; cols = 16; mines = 40; break;
        case 3: rows = 16; cols = 30; mines = 99; break;
        case 4: {
            std::cout << "输入: 行数 列数 地雷数: ";
            std::cin >> rows >> cols >> mines;
            if (rows < 1 || cols < 1 || mines < 1 || mines >= rows * cols) {
                std::cout << "? 参数无效，使用默认设置\n";
                rows = 9; cols = 9; mines = 10;
            }
            break;
        }
        default: 
            std::cout << "使用默认: 初级难度\n";
            rows = 9; cols = 9; mines = 10;
    }

    Minesweeper game(rows, cols, mines);
    game.play();

    return 0;
}
