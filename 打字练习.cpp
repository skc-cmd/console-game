#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include <sstream>

using namespace std;

// ==================== 通用工具函数 ====================
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// ==================== 打字练习游戏 ====================
class TypingGame {
private:
    // 单词库
    vector<string> wordBank;
    vector<string> currentWords;      // 当前屏幕上的单词
    vector<int> wordX;                // 单词X位置
    vector<int> wordY;                // 单词Y位置（下落）
    vector<string> typedPart;         // 已输入部分（用于高亮）
    
    int score;
    int totalWords;
    int wordsTyped;
    int wordsMissed;
    int screenWidth;
    int screenHeight;
    int groundY;                      // 地面位置（底线）
    
    bool gameOver;
    bool waitingForNext;              // 等待下一个单词
    clock_t waitTimer;                // 等待计时
    
    string currentInput;              // 当前输入
    int activeWordIndex;              // 正在输入的单词索引
    
    HANDLE hConsole;
    clock_t lastDropTime;
    int dropInterval;                 // 下落间隔（毫秒）
    int wordCount;                    // 已生成单词数
    int maxWords;                     // 最大单词数（15个）
    
    // ===== 懒加载相关 =====
    bool needRefresh;
    string screenBuffer;              // 屏幕缓冲区
    
public:
    TypingGame() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        srand((unsigned)time(NULL));
        
        // 初始化单词库
        wordBank = {
            "apple", "book", "cat", "dog", "fish", "game", "happy", "ice", "jump",
            "king", "love", "moon", "night", "open", "pink", "queen", "run",
            "sun", "tree", "umbrella", "very", "water", "xray", "yellow", "zoo",
            "computer", "science", "study", "world", "hello", "good", "time",
            "school", "friend", "family", "happy", "dream", "star", "cloud",
            "music", "dance", "light", "peace", "hope", "life", "love"
        };
        
        screenWidth = 80;
        screenHeight = 25;
        groundY = screenHeight - 4;
        maxWords = 15;
        
        initGame();
    }
    
    void initGame() {
        currentWords.clear();
        wordX.clear();
        wordY.clear();
        typedPart.clear();
        currentInput = "";
        activeWordIndex = -1;
        score = 0;
        wordsTyped = 0;
        wordsMissed = 0;
        totalWords = 0;
        wordCount = 0;
        gameOver = false;
        waitingForNext = false;
        waitTimer = 0;
        dropInterval = 1000;  // 每秒下落一格
        needRefresh = true;
        
        lastDropTime = clock();
        
        // 生成第一个单词
        spawnWord();
    }
    
    // 生成一个新单词
    void spawnWord() {
        if (wordCount >= maxWords) {
            if (currentWords.empty()) {
                gameOver = true;
                needRefresh = true;
            }
            return;
        }
        
        int idx = rand() % wordBank.size();
        string word = wordBank[idx];
        
        int x = rand() % (screenWidth - word.length() - 10) + 5;
        int y = 2;
        
        currentWords.push_back(word);
        wordX.push_back(x);
        wordY.push_back(y);
        typedPart.push_back("");
        
        wordCount++;
        totalWords++;
        needRefresh = true;
    }
    
    // 获取当前输入的单词索引
    int findMatchingWord() {
        for (int i = 0; i < currentWords.size(); i++) {
            if (currentWords[i].find(currentInput) == 0) {
                return i;
            }
        }
        return -1;
    }
    
    // 构建屏幕内容（懒加载）
    string buildScreen() {
        stringstream ss;
        
        // 标题
        ss << "  =========== 打字练习 ===========" << endl;
        ss << "  得分: " << score << "  已打: " << wordsTyped 
           << "  错过: " << wordsMissed << "  剩余: " 
           << (maxWords - wordCount + currentWords.size()) << endl;
        ss << "  输入单词，在落地前打完！" << endl;
        ss << "  -----------------------------" << endl;
        
        // 绘制游戏区域
        for (int y = 0; y < screenHeight - 3; y++) {
            ss << "  ";
            
            for (int x = 0; x < screenWidth; x++) {
                char ch = ' ';
                bool isWordChar = false;
                int wordIdx = -1;
                int charPos = -1;
                
                for (int i = 0; i < currentWords.size(); i++) {
                    if (y == wordY[i] && x >= wordX[i] && x < wordX[i] + (int)currentWords[i].length()) {
                        ch = currentWords[i][x - wordX[i]];
                        isWordChar = true;
                        wordIdx = i;
                        charPos = x - wordX[i];
                        break;
                    }
                }
                
                if (isWordChar) {
                    // 检查是否已输入该字符
                    if (charPos < (int)typedPart[wordIdx].length()) {
                        // 已输入 - 用大写显示（表示高亮）
                        ss << (char)toupper(ch);
                    } else {
                        ss << ch;
                    }
                } else {
                    ss << " ";
                }
            }
            ss << endl;
        }
        
        // 地面线
        ss << "  ";
        for (int i = 0; i < screenWidth; i++) {
            if (i == screenWidth / 2 - 10) {
                ss << "===== 底线 =====";
                i += 14;
            } else {
                ss << "─";
            }
        }
        ss << endl;
        
        // 输入区域
        ss << "  -----------------------------" << endl;
        ss << "  输入: " << currentInput;
        
        if (!currentInput.empty()) {
            int idx = findMatchingWord();
            if (idx != -1) {
                string remaining = currentWords[idx].substr(currentInput.length());
                if (!remaining.empty()) {
                    ss << remaining;
                }
            }
        }
        ss << endl;
        
        // 提示信息
        if (waitingForNext) {
            ss << "  ? 等待下一个单词..." << endl;
        } else if (gameOver) {
            ss << endl;
            ss << "  =========================================" << endl;
            ss << "  ||       游 戏 结 束 ！               ||" << endl;
            ss << "  ||       得分: " << score << "                          ||" << endl;
            ss << "  ||       正确: " << wordsTyped << "  错误: " << wordsMissed << "            ||" << endl;
            ss << "  ||       按 R 重新开始                ||" << endl;
            ss << "  ||       按 ESC 返回菜单             ||" << endl;
            ss << "  =========================================" << endl;
        }
        
        return ss.str();
    }
    
    // 绘制游戏（只有需要刷新时才真正绘制）
    void draw() {
        if (!needRefresh) return;
        needRefresh = false;
        
        // 构建新画面
        string newScreen = buildScreen();
        
        // 只有画面变化时才更新
        if (newScreen != screenBuffer) {
            system("cls");
            cout << newScreen;
            screenBuffer = newScreen;
        }
        
        hideCursor();
    }
    
    // 处理输入
    void processInput(char key) {
        if (gameOver) return;
        if (waitingForNext) return;
        if (currentWords.empty()) return;
        
        if (key == 8) {  // 退格键
            if (!currentInput.empty()) {
                currentInput.pop_back();
                int idx = findMatchingWord();
                if (idx != -1) {
                    typedPart[idx] = currentInput;
                }
                needRefresh = true;
            }
            return;
        }
        
        if (key >= 'a' && key <= 'z') {
            currentInput += key;
            
            int idx = findMatchingWord();
            if (idx != -1) {
                typedPart[idx] = currentInput;
                needRefresh = true;
                
                // 如果完全匹配
                if (currentInput == currentWords[idx]) {
                    // 成功打出一个单词
                    score += 10 + (screenHeight - wordY[idx]) / 2;
                    wordsTyped++;
                    
                    // 移除该单词
                    currentWords.erase(currentWords.begin() + idx);
                    wordX.erase(wordX.begin() + idx);
                    wordY.erase(wordY.begin() + idx);
                    typedPart.erase(typedPart.begin() + idx);
                    
                    currentInput = "";
                    needRefresh = true;
                    
                    // 检查是否所有单词都打完了
                    if (currentWords.empty() && wordCount >= maxWords) {
                        gameOver = true;
                        needRefresh = true;
                        return;
                    }
                    
                    // 等待2秒再生成下一个
                    waitingForNext = true;
                    waitTimer = clock();
                    needRefresh = true;
                }
            } else {
                // 输入错误，重置输入
                currentInput = "";
                for (int i = 0; i < typedPart.size(); i++) {
                    typedPart[i] = "";
                }
                needRefresh = true;
            }
        }
    }
    
    // 更新游戏
    void update() {
        if (gameOver) return;
        if (waitingForNext) {
            clock_t now = clock();
            if ((now - waitTimer) * 1000 / CLOCKS_PER_SEC >= 2000) {
                waitingForNext = false;
                if (wordCount < maxWords) {
                    spawnWord();
                } else if (currentWords.empty()) {
                    gameOver = true;
                    needRefresh = true;
                }
            }
            return;
        }
        
        // 单词下落
        clock_t now = clock();
        if ((now - lastDropTime) * 1000 / CLOCKS_PER_SEC >= dropInterval) {
            lastDropTime = now;
            bool moved = false;
            
            for (int i = currentWords.size() - 1; i >= 0; i--) {
                wordY[i]++;
                moved = true;
                
                // 检查是否到达底线
                if (wordY[i] >= groundY) {
                    wordsMissed++;
                    
                    currentWords.erase(currentWords.begin() + i);
                    wordX.erase(wordX.begin() + i);
                    wordY.erase(wordY.begin() + i);
                    typedPart.erase(typedPart.begin() + i);
                    
                    currentInput = "";
                    needRefresh = true;
                    
                    if (currentWords.empty() && wordCount >= maxWords) {
                        gameOver = true;
                        needRefresh = true;
                        return;
                    }
                }
            }
            
            if (moved) needRefresh = true;
            
            // 如果没有单词了但还没生成完，生成新的
            if (currentWords.empty() && wordCount < maxWords && !waitingForNext) {
                spawnWord();
            }
        }
    }
    
    void run() {
        initGame();
        showCursor();
        screenBuffer = "";  // 清空缓冲区
        
        while (true) {
            update();
            draw();
            
            if (gameOver) {
                if (_kbhit()) {
                    char key = _getch();
                    if (tolower(key) == 'r') {
                        initGame();
                        screenBuffer = "";
                        showCursor();
                    } else if (key == 27) {
                        return;
                    }
                }
                Sleep(50);
                continue;
            }
            
            if (_kbhit()) {
                char key = _getch();
                if (key == 27) return;
                if (tolower(key) == 'r') {
                    initGame();
                    screenBuffer = "";
                    showCursor();
                    continue;
                }
                processInput(key);
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
        cout << "  ||       打字练习游戏                  ||" << endl;
        cout << "  =========================================" << endl;
        cout << "  ||  1. 开始游戏                       ||" << endl;
        cout << "  ||  0. 退出                         ||" << endl;
        cout << "  =========================================" << endl;
        cout << "  规则：" << endl;
        cout << "  - 单词从上方下落，每秒下落一格" << endl;
        cout << "  - 在落地前打完单词，得分越高越靠上" << endl;
        cout << "  - 共15个单词，打完一个等2秒再出下一个" << endl;
        cout << "  - 按退格键删除，按ESC返回" << endl;
        cout << "  =========================================" << endl;
        cout << "  请选择: ";
        
        char choice = _getch();
        
        if (choice == '0') {
            system("cls");
            cout << "  感谢游玩！再见！" << endl;
            break;
        }
        else if (choice == '1') {
            TypingGame game;
            game.run();
        }
    }
    
    return 0;
}
