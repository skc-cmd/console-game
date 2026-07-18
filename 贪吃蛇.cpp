#include <iostream>
#include <vector>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

using namespace std;

// 游戏配置
struct Config {
    int width = 15;
    int height = 15;
    int foodSpawnInterval = 3;  // 随机刻：每3秒生成一个食物
    int moveInterval = 150;     // 蛇移动间隔（毫秒）
};

// 方向枚举
enum Direction { UP, DOWN, LEFT, RIGHT, NONE };

// 位置结构
struct Point {
    int x, y;
    Point(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class SnakeGame {
private:
    Config config;
    deque<Point> snake;
    vector<Point> foods;
    Direction dir;
    Direction nextDir;
    bool gameOver;
    int score;
    clock_t lastMove;
    clock_t lastFoodSpawn;

    // 控制台句柄
    HANDLE hConsole;
    COORD cursorPos;

public:
    SnakeGame() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        srand((unsigned)time(NULL));
        initGame();
    }

    // 设置地图大小
    void setMapSize(int w, int h) {
        if (w >= 5 && h >= 5) {
            config.width = w;
            config.height = h;
        }
    }

    // 设置随机刻（食物生成间隔，秒）
    void setFoodInterval(int seconds) {
        if (seconds >= 1) {
            config.foodSpawnInterval = seconds;
        }
    }

    // 初始化游戏
    void initGame() {
        snake.clear();
        foods.clear();
        
        // 初始蛇：2格长，水平放置
        int startX = config.width / 2;
        int startY = config.height / 2;
        snake.push_back(Point(startX, startY));
        snake.push_back(Point(startX - 1, startY));
        
        dir = RIGHT;
        nextDir = RIGHT;
        gameOver = false;
        score = 0;
        
        lastMove = clock();
        lastFoodSpawn = clock();
        
        // 初始生成2个食物
        spawnFood();
        spawnFood();
    }

    // 生成一个食物（在空白位置）
    void spawnFood() {
        if (foods.size() >= 5) return;  // 限制最大食物数量
        
        vector<Point> emptyCells;
        for (int y = 0; y < config.height; y++) {
            for (int x = 0; x < config.width; x++) {
                Point p(x, y);
                // 检查是否被蛇占据
                bool occupied = false;
                for (size_t i = 0; i < snake.size(); i++) {
                    if (snake[i] == p) {
                        occupied = true;
                        break;
                    }
                }
                // 检查是否已有食物
                for (size_t i = 0; i < foods.size(); i++) {
                    if (foods[i] == p) {
                        occupied = true;
                        break;
                    }
                }
                if (!occupied) {
                    emptyCells.push_back(p);
                }
            }
        }
        
        if (!emptyCells.empty()) {
            int idx = rand() % emptyCells.size();
            foods.push_back(emptyCells[idx]);
        }
    }

    // 改变方向
    void changeDirection(Direction newDir) {
        // 防止180度掉头
        if ((dir == UP && newDir == DOWN) ||
            (dir == DOWN && newDir == UP) ||
            (dir == LEFT && newDir == RIGHT) ||
            (dir == RIGHT && newDir == LEFT)) {
            return;
        }
        nextDir = newDir;
    }

    // 移动蛇
    bool moveSnake() {
        dir = nextDir;
        
        // 计算新头部位置
        Point head = snake.front();
        switch (dir) {
            case UP:    head.y--; break;
            case DOWN:  head.y++; break;
            case LEFT:  head.x--; break;
            case RIGHT: head.x++; break;
            default: return true;
        }
        
        // 检查是否吃到食物
        bool ate = false;
        for (vector<Point>::iterator it = foods.begin(); it != foods.end(); ) {
            if (*it == head) {
                it = foods.erase(it);
                ate = true;
                score++;
                break;
            } else {
                ++it;
            }
        }
        
        // 移动蛇
        snake.push_front(head);
        if (!ate) {
            snake.pop_back();  // 没吃到食物，移除尾部
        }
        
        // 检查碰撞
        // 撞墙
        if (head.x < 0 || head.x >= config.width || head.y < 0 || head.y >= config.height) {
            gameOver = true;
            return false;
        }
        
        // 撞自身（新头部不能与除尾部外的其他部分重叠）
        for (size_t i = 1; i < snake.size(); i++) {
            if (snake[i] == head) {
                gameOver = true;
                return false;
            }
        }
        
        return true;
    }

    // 绘制游戏界面
    void draw() {
        // 移动到左上角绘制（避免闪烁）
        cursorPos.X = 0;
        cursorPos.Y = 0;
        SetConsoleCursorPosition(hConsole, cursorPos);
        
        // 隐藏光标
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        // 绘制边框 - 上边框
        cout << "┌";
        for (int i = 0; i < config.width; i++) cout << "─";
        cout << "┐";
        
        // 显示信息
        cout << "  得分: " << score << "  |  蛇身: " << snake.size() << "  |  食物: " << foods.size() << endl;
        
        // 游戏区域
        for (int y = 0; y < config.height; y++) {
            cout << "│";
            for (int x = 0; x < config.width; x++) {
                Point p(x, y);
                bool printed = false;
                
                // 画蛇
                for (size_t i = 0; i < snake.size(); i++) {
                    if (snake[i] == p) {
                        if (i == 0) {
                            // 蛇头
                            switch (dir) {
                                case UP:    cout << "▲"; break;
                                case DOWN:  cout << "▼"; break;
                                case LEFT:  cout << "?"; break;
                                case RIGHT: cout << "?"; break;
                                default:    cout << "●"; break;
                            }
                        } else {
                            cout << "■";
                        }
                        printed = true;
                        break;
                    }
                }
                
                // 画食物
                if (!printed) {
                    for (size_t i = 0; i < foods.size(); i++) {
                        if (foods[i] == p) {
                            cout << "★";
                            printed = true;
                            break;
                        }
                    }
                }
                
                if (!printed) {
                    cout << " ";
                }
            }
            cout << "│";
            
            // 右侧信息
            if (y == 1) cout << "  操作: WASD / 方向键";
            else if (y == 2) cout << "  随机刻: " << config.foodSpawnInterval << "秒";
            else if (y == 3) cout << "  地图: " << config.width << "×" << config.height;
            else if (y == 5) cout << "  按 P 暂停/继续";
            else if (y == 6) cout << "  按 R 重新开始";
            else if (y == 7) cout << "  按 ESC 退出";
            
            cout << endl;
        }
        
        // 下边框
        cout << "└";
        for (int i = 0; i < config.width; i++) cout << "─";
        cout << "┘" << endl;
        
        if (gameOver) {
            cout << "\n╔════════════════════════════╗" << endl;
            cout << "║      游 戏 结 束 ！       ║" << endl;
            cout << "║    最终得分: " << score << "          ║" << endl;
            cout << "║    按 R 重新开始          ║" << endl;
            cout << "║    按 ESC 退出            ║" << endl;
            cout << "╚════════════════════════════╝" << endl;
        }
    }

    // 游戏循环
    void run() {
        bool paused = false;
        
        while (true) {
            // 处理输入（非阻塞）
            if (_kbhit()) {
                char key = _getch();
                if (key == -32) {  // 方向键
                    key = _getch();
                    switch (key) {
                        case 72: changeDirection(UP); break;
                        case 80: changeDirection(DOWN); break;
                        case 75: changeDirection(LEFT); break;
                        case 77: changeDirection(RIGHT); break;
                    }
                } else {
                    switch (tolower(key)) {
                        case 'w': changeDirection(UP); break;
                        case 's': changeDirection(DOWN); break;
                        case 'a': changeDirection(LEFT); break;
                        case 'd': changeDirection(RIGHT); break;
                        case 'p': paused = !paused; break;
                        case 'r': 
                            initGame();
                            paused = false;
                            break;
                        case 27: return;  // ESC退出
                    }
                }
            }
            
            if (!gameOver && !paused) {
                clock_t now = clock();
                
                // 蛇移动（使用clock()）
                int moveElapsed = (now - lastMove) * 1000 / CLOCKS_PER_SEC;
                if (moveElapsed >= config.moveInterval) {
                    moveSnake();
                    lastMove = now;
                }
                
                // 食物生成（随机刻）
                int foodElapsed = (now - lastFoodSpawn) / CLOCKS_PER_SEC;
                if (foodElapsed >= config.foodSpawnInterval) {
                    spawnFood();
                    lastFoodSpawn = now;
                }
            }
            
            draw();
            
            // 延迟50毫秒（用Sleep）
            Sleep(50);
        }
    }
};

// 主函数
int main() {
    system("cls");
    
    SnakeGame game;
    
    cout << "╔══════════════════════════════════════╗" << endl;
    cout << "║         ?? 贪吃蛇游戏 ??           ║" << endl;
    cout << "╠══════════════════════════════════════╣" << endl;
    cout << "║  默认地图: 15×15                    ║" << endl;
    cout << "║  随机刻: 3秒生成一个食物            ║" << endl;
    cout << "║  初始蛇长: 2格                     ║" << endl;
    cout << "╚══════════════════════════════════════╝" << endl;
    cout << endl;
    
    cout << "是否自定义设置？(y/n): ";
    char choice;
    cin >> choice;
    
    if (tolower(choice) == 'y') {
        int w, h, interval;
        cout << "请输入地图宽度 (5-30): ";
        cin >> w;
        cout << "请输入地图高度 (5-30): ";
        cin >> h;
        cout << "请输入食物生成间隔 (秒, 1-10): ";
        cin >> interval;
        
        game.setMapSize(w, h);
        game.setFoodInterval(interval);
    }
    
    cout << "\n按任意键开始游戏..." << endl;
    _getch();
    
    system("cls");
    game.run();
    
    return 0;
}
