#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <vector>

using namespace std;

class Game2048 {
private:
    int grid[4][4];
    int score;
    bool gameOver;
    bool win;
    HANDLE hConsole;
    COORD cursorPos;

public:
    Game2048() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        srand((unsigned)time(NULL));
        initGame();
    }

    // 初始化游戏
    void initGame() {
        // 清空网格
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                grid[i][j] = 0;
            }
        }
        score = 0;
        gameOver = false;
        win = false;
        
        // 生成两个初始数字
        spawnNumber();
        spawnNumber();
    }

    // 在空白位置生成数字（2或4）
    void spawnNumber() {
        vector<pair<int, int>> emptyCells;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (grid[i][j] == 0) {
                    emptyCells.push_back(make_pair(i, j));
                }
            }
        }
        
        if (emptyCells.empty()) return;
        
        int idx = rand() % emptyCells.size();
        int row = emptyCells[idx].first;
        int col = emptyCells[idx].second;
        
        // 90%概率生成2，10%概率生成4
        grid[row][col] = (rand() % 10 < 9) ? 2 : 4;
    }

    // 检查游戏是否结束
    bool checkGameOver() {
        // 检查是否有空格
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (grid[i][j] == 0) return false;
            }
        }
        
        // 检查是否有相邻相同的数字
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                // 检查右边
                if (j < 3 && grid[i][j] == grid[i][j + 1]) return false;
                // 检查下边
                if (i < 3 && grid[i][j] == grid[i + 1][j]) return false;
            }
        }
        
        return true;
    }

    // 向左滑动
    bool slideLeft() {
        bool moved = false;
        
        for (int row = 0; row < 4; row++) {
            // 压缩行（去掉0）
            int writePos = 0;
            for (int col = 0; col < 4; col++) {
                if (grid[row][col] != 0) {
                    grid[row][writePos] = grid[row][col];
                    if (writePos != col) {
                        grid[row][col] = 0;
                        moved = true;
                    }
                    writePos++;
                }
            }
            
            // 合并相邻相同数字
            for (int col = 0; col < 3; col++) {
                if (grid[row][col] != 0 && grid[row][col] == grid[row][col + 1]) {
                    grid[row][col] *= 2;
                    score += grid[row][col];
                    grid[row][col + 1] = 0;
                    moved = true;
                    
                    // 检查是否达到2048
                    if (grid[row][col] == 2048) win = true;
                }
            }
            
            // 再次压缩（填补合并后的空白）
            writePos = 0;
            for (int col = 0; col < 4; col++) {
                if (grid[row][col] != 0) {
                    grid[row][writePos] = grid[row][col];
                    if (writePos != col) {
                        grid[row][col] = 0;
                        moved = true;
                    }
                    writePos++;
                }
            }
        }
        
        return moved;
    }

    // 向右滑动
    bool slideRight() {
        // 水平翻转后再左滑
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 2; j++) {
                swap(grid[i][j], grid[i][3 - j]);
            }
        }
        
        bool moved = slideLeft();
        
        // 再翻转回来
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 2; j++) {
                swap(grid[i][j], grid[i][3 - j]);
            }
        }
        
        return moved;
    }

    // 向上滑动
    bool slideUp() {
        // 转置后再左滑
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                swap(grid[i][j], grid[j][i]);
            }
        }
        
        bool moved = slideLeft();
        
        // 再转置回来
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                swap(grid[i][j], grid[j][i]);
            }
        }
        
        return moved;
    }

    // 向下滑动
    bool slideDown() {
        // 转置后再右滑
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                swap(grid[i][j], grid[j][i]);
            }
        }
        
        bool moved = slideRight();
        
        // 再转置回来
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                swap(grid[i][j], grid[j][i]);
            }
        }
        
        return moved;
    }

    // 绘制游戏界面
    void draw() {
        // 移动到左上角
        cursorPos.X = 0;
        cursorPos.Y = 0;
        SetConsoleCursorPosition(hConsole, cursorPos);
        
        // 隐藏光标
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        cout << "+================================+" << endl;
        cout << "|         2048 游 戏            |" << endl;
        cout << "+================================+" << endl;
        cout << "|  得分: " << score;
        for (int i = 0; i < 10 - to_string(score).length(); i++) cout << " ";
        cout << "|" << endl;
        cout << "+----+----+----+----+" << endl;
        
        for (int i = 0; i < 4; i++) {
            cout << "|";
            for (int j = 0; j < 4; j++) {
                if (grid[i][j] == 0) {
                    cout << "    ";
                } else {
                    // 不同数字不同颜色显示（这里用不同背景色代替）
                    int num = grid[i][j];
                    if (num < 10) cout << "  " << num << " ";
                    else if (num < 100) cout << " " << num << " ";
                    else if (num < 1000) cout << " " << num;
                    else cout << num;
                }
                cout << "|";
            }
            cout << endl;
            cout << "+----+----+----+----+" << endl;
        }
        
        cout << endl;
        cout << "  操作: WASD / 方向键" << endl;
        cout << "  按 R 重新开始" << endl;
        cout << "  按 ESC 退出" << endl;
        
        if (win) {
            cout << "\n+================================+" << endl;
            cout << "|   ?? 恭喜你赢了！2048！ ??   |" << endl;
            cout << "|     按 R 重新开始            |" << endl;
            cout << "+================================+" << endl;
        } else if (gameOver) {
            cout << "\n+================================+" << endl;
            cout << "|      游 戏 结 束 ！          |" << endl;
            cout << "|    最终得分: " << score;
            for (int i = 0; i < 10 - to_string(score).length(); i++) cout << " ";
            cout << "|" << endl;
            cout << "|    按 R 重新开始             |" << endl;
            cout << "+================================+" << endl;
        }
    }

    // 游戏主循环
    void run() {
        while (true) {
            draw();
            
            // 检查游戏状态
            if (checkGameOver() && !win) {
                gameOver = true;
                draw();
            }
            
            // 处理输入
            if (_kbhit()) {
                char key = _getch();
                bool moved = false;
                
                if (key == -32) {  // 方向键
                    key = _getch();
                    switch (key) {
                        case 72: moved = slideUp(); break;
                        case 80: moved = slideDown(); break;
                        case 75: moved = slideLeft(); break;
                        case 77: moved = slideRight(); break;
                    }
                } else {
                    switch (tolower(key)) {
                        case 'w': moved = slideUp(); break;
                        case 's': moved = slideDown(); break;
                        case 'a': moved = slideLeft(); break;
                        case 'd': moved = slideRight(); break;
                        case 'r': 
                            initGame();
                            continue;
                        case 27: return;  // ESC退出
                    }
                }
                
                // 如果移动有效且游戏未结束且未胜利
                if (moved && !gameOver && !win) {
                    spawnNumber();
                    if (checkGameOver()) {
                        gameOver = true;
                    }
                }
            }
            
            Sleep(50);
        }
    }
};

// 主函数
int main() {
    system("cls");
    
    Game2048 game;
    
    cout << "+================================+" << endl;
    cout << "|        2048 游 戏              |" << endl;
    cout << "+================================+" << endl;
    cout << "|  使用方向键或 WASD 控制方向    |" << endl;
    cout << "|  合并相同数字，达到 2048 获胜  |" << endl;
    cout << "|  按任意键开始游戏...           |" << endl;
    cout << "+================================+" << endl;
    
    _getch();
    system("cls");
    
    game.run();
    
    return 0;
}
