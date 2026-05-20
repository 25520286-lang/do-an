#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <string>
#include <fstream> 

using namespace std;

// // Class cha dùng cho tất cả block
class Piece {
public:
     // Hàm xoay block
    // virtual để hỗ trợ đa hình
    virtual void rotate(char shape[4][4]) {

        char temp[4][4];

     // Copy shape hiện tại sang temp
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i][j] = shape[i][j];
            }
        }

        // Xoay block 90 độ
        // Công thức:
        // shape[j][3 - i]
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[j][3 - i] = temp[i][j];
            }
        }
    }
};


class OPiece : public Piece {
public:

    void rotate(char shape[4][4]) override {

    }
};

#define H 20
#define W 15
#define OFFSET_X 25

enum Color {
    CYAN = 11, YELLOW = 14, PURPLE = 13, GREEN = 10, RED = 12, BLUE = 9, WHITE = 15, GRAY = 8
};

char board[H][W] = {};
char blocks[][4][4] = {
    {{' ','1',' ',' '},{' ','1',' ',' '},{' ','1',' ',' '},{' ','1',' ',' '}},
    {{' ',' ',' ',' '},{' ','2','2',' '},{' ','2','2',' '},{' ',' ',' ',' '}},
    {{' ',' ',' ',' '},{' ','3',' ',' '},{'3','3','3',' '},{' ',' ',' ',' '}},
    {{' ',' ',' ',' '},{' ','4','4',' '},{'4','4',' ',' '},{' ',' ',' ',' '}},
    {{' ',' ',' ',' '},{'5','5',' ',' '},{' ','5','5',' '},{' ',' ',' ',' '}},
    {{' ',' ',' ',' '},{'6',' ',' ',' '},{'6','6','6',' '},{' ',' ',' ',' '}},
    {{' ',' ',' ',' '},{' ',' ','7',' '},{'7','7','7',' '},{' ',' ',' ',' '}}
};

int score = 0, speed = 200;
int x = 4, y = 0, b = 1, next_b = 0;
bool isGravityMode = false; // Biến check chế độ

// Tạo object cho block thường
Piece normalPiece;
// Tạo object cho block O
OPiece oPiece;

int linesCleared = 0;       // Đếm số hàng đã xóa
int sprintTarget = 20;      // Mục tiêu xóa 20 hàng
clock_t sprintStartTime;    // Lưu thời điểm bắt đầu chơi
int gameMode = 0;           // 0: Normal, 1: Gravity, 2: Sprint
clock_t spawnTime; // Đưa dòng này lên đây (biến toàn cục)

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void applyBlockColor(char c) {
    switch (c) {
        case '1': setColor(CYAN); break;
        case '2': setColor(YELLOW); break;
        case '3': setColor(PURPLE); break;
        case '4': setColor(GREEN); break;
        case '5': setColor(RED); break;
        case '6': setColor(BLUE); break;
        case '7': setColor(WHITE); break;
        case '#': setColor(GRAY); break;
        default: setColor(WHITE); break;
    }
}

void gotoxy(int x, int y) {
    COORD c = {(short)x, (short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void drawBigTitle() {
    int tx = OFFSET_X + 11;
    int ty = 2;
    int colors[] = {12, 14, 10, 11, 9, 13};
    string art[5][6] = {
        {"[][][] ", "[][][] ", " [][][] ", "[][][] ", " [] ", "[][][]"},
        {"  []   ", "[]      ", "  []   ", "[]  [] ", " [] ", "[]    "},
        {"  []   ", "[][]    ", "  []   ", "[][]    ", "[] ", " [][] "},
        {"  []   ", "[]      ", "  []   ", "[]  [] ", " [] ", "    []"},
        {"  []   ", "[][][] ", "   []   ", "[]  [] ", " [] ", "[][][]"}
    };
    setColor(7);
    gotoxy(tx - 4, ty - 1); cout << "===============================================";
    for (int i = 0; i < 5; i++) {
        gotoxy(tx, ty + i);
        for (int j = 0; j < 6; j++) {
            setColor(colors[j]); cout << art[i][j];
        }
    }
    setColor(7);
    gotoxy(tx - 4, ty + 5); cout << "===============================================";
    gotoxy(tx - 4, ty + 6); cout << "================= MENU GAME ===================";
    setColor(15);
}

// HÀM MỚI: CHỌN CHẾ ĐỘ CHƠI
// Thêm hàm hiển thị hướng dẫn tóm tắt trong menu chọn mode
void drawModePreview(int choice) {
    int px = OFFSET_X + 15;
    int py = 15;
    gotoxy(px + 7, py);

    if (choice == 0) {
        setColor(CYAN); cout << "[ MODE: CO DIEN ]      ";
        setColor(WHITE);
        gotoxy(px, py + 1); cout << "- Gach roi tu tu theo thoi gian.          ";
        gotoxy(px, py + 2); cout << "- Toc do tang dan khi an diem.            ";
    }
    else if (choice == 1) {
        setColor(RED);   cout << "[ MODE: TRONG LUC ]    ";
        setColor(WHITE);
        gotoxy(px, py + 1); cout << "- Co 3s de di chuyen.                     ";
        gotoxy(px, py + 2); cout << "- Sau do gach ban THANG xuong day.        ";
    }
    else if (choice == 2) {
        setColor(YELLOW); cout << "[ MODE: SPRINT ]       ";
        setColor(WHITE);
        gotoxy(px, py + 1); cout << "- Chay dua voi thoi gian.                 ";
        gotoxy(px, py + 2); cout << "- Xoa 20 hang nhanh nhat de THANG.        ";
    }
    else {
        setColor(GRAY);   cout << "[ QUAY LAI MENU ]      ";
        setColor(WHITE);
        gotoxy(px, py + 1); cout << "- Quay lai man hinh chinh.                ";
        gotoxy(px, py + 2); cout << "                                          "; // Xóa dòng cũ
    }
}

// Cập nhật lại hàm Menu phụ chọn mode
int selectModeMenu() {
    int choice = 0;
    system("cls"); // Xóa màn hình 1 lần duy nhất

    int mx = OFFSET_X + 23;
    gotoxy(mx - 4, 8); setColor(YELLOW); cout << "--- CHON CHE DO CHOI ---";

    while (true) {
        // Lựa chọn 0
        gotoxy(mx - 3, 10);
        if (choice == 0) { setColor(CYAN); cout << " > CO DIEN (Normal) <  "; }
        else { setColor(WHITE); cout << "   CO DIEN (Normal)    "; }

        // Lựa chọn 1
        gotoxy(mx - 4, 11);
        if (choice == 1) { setColor(CYAN); cout << " > TRONG LUC (Gravity) <"; }
        else { setColor(WHITE); cout << "   TRONG LUC (Gravity)  "; }

        // Lựa chọn 2
        gotoxy(mx - 3, 12);
        if (choice == 2) { setColor(CYAN); cout << " > SPRINT (20 Lines) < "; }
        else { setColor(WHITE); cout << "   SPRINT (20 Lines)   "; }

        // Lựa chọn 3: THOÁT
        gotoxy(mx - 3, 13);
        if (choice == 3) { setColor(RED); cout << " > THOAT RA MENU <    "; }
        else { setColor(WHITE); cout << "   THOAT RA MENU      "; }

        drawModePreview(choice);

        char c = _getch();
        if (c == 'w' || c == 72) choice = (choice - 1 + 4) % 4; // Chia lấy dư cho 4
        if (c == 's' || c == 80) choice = (choice + 1) % 4;     // Chia lấy dư cho 4

        if (c == 13) {
            if (choice == 3) return -1; // Trả về -1 để báo hiệu quay lại menu chính
            return choice;
        }
    }
}

void initBoard() {
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            if ((i == H - 1) || (j == 0) || (j == W - 1)) board[i][j] = '#';
            else board[i][j] = ' ';
}

bool canMove(int dx, int dy) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ') {
                int tx = x + j + dx;
                int ty = y + i + dy;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
    return true;
}
void applyGravity() {
    while (canMove(0, 1)) {
        y++;
    }
}

void loadingScreen() {
    system("cls");
    int lx = OFFSET_X + 21;
    int ly = H / 2 + 2;
    setColor(CYAN);
    string msg = "    Loading...";
    for (int i = 0; i <= 100; i += 5) {
        gotoxy(lx, ly); cout << msg;
        gotoxy(lx - 2, ly + 2);
        cout << "[";
        int barWidth = 20;
        int pos = (i * barWidth) / 100;
        for (int j = 0; j < barWidth; ++j) {
            if (j < pos) cout << "=";
            else cout << " ";
        }
        cout << "] " << i << "%";
        Sleep(50);
    }
    setColor(WHITE);
    gotoxy(lx, ly + 4); cout << "    complete!";
    Sleep(500);
}

void showGuide() {
    system("cls");
    setColor(YELLOW);
    int gx = OFFSET_X + 11;
    gotoxy(gx, 8);  cout << "    === HUONG DAN ===";
    setColor(WHITE);
    gotoxy(gx, 10); cout << "      A,a: Sang trai";
    gotoxy(gx, 11); cout << "      D,d: Sang phai";
    gotoxy(gx, 12); cout << "      W,w: Xoay khoi";
    gotoxy(gx, 13); cout << "      X,x: Roi nhanh";
    setColor(WHITE);
    gotoxy(gx, 16); cout << "Bam phim bat ky de quay lai...";
    _getch();
}

int menu() {
    int choice = 0;
    system("cls"); // Xóa màn hình 1 lần duy nhất khi mở Menu
    drawBigTitle(); // Vẽ tiêu đề ASCII cố định bên trên

    while (true) {
        int mx = OFFSET_X + 21;

        // Vẽ lựa chọn 1
        gotoxy(mx, 10);
        if (choice == 0) { setColor(CYAN); cout << " > BAT DAU GAME < "; }
        else { setColor(WHITE); cout << "   BAT DAU GAME    "; }

        // Vẽ lựa chọn 2
        gotoxy(mx, 11);
        if (choice == 1) { setColor(CYAN); cout << " > HUONG DAN CHOI <"; }
        else { setColor(WHITE); cout << "   HUONG DAN CHOI   "; }

        // Vẽ lựa chọn 3
        gotoxy(mx, 12);
        if (choice == 2) { setColor(RED); cout << " > THOAT GAME <   "; }
        else { setColor(WHITE); cout << "   THOAT GAME      "; }

        char c = _getch();
        if (c == 'w' || c == 72) choice = (choice - 1 + 3) % 3;
        if (c == 's' || c == 80) choice = (choice + 1) % 3;
        if (c == 13) return choice;

        // KHÔNG gọi system("cls") ở đây nữa
    }
}

void boardDelBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ' && y + i < H) board[y + i][x + j] = ' ';
}

void block2Board() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b][i][j] != ' ') board[y + i][x + j] = blocks[b][i][j];
}

void draw() {
    gotoxy(OFFSET_X, 0);
    for (int i = 0; i < H; i++) {
        gotoxy(OFFSET_X, i);
        for (int j = 0; j < W; j++) {
            if (board[i][j] == ' ') cout << "  ";
            else {
                applyBlockColor(board[i][j]);
                if (board[i][j] == '#') cout << "##";
                else cout << "[]";
            }
        }
    }

    setColor(WHITE);
    gotoxy(OFFSET_X, H + 1);
    cout << "Score: " << score << "    ";

    gotoxy(OFFSET_X + 20, H + 1);
    if(gameMode == 1) { setColor(RED); cout << "MODE: GRAVITY"; }
    else if(gameMode == 2) { setColor(CYAN); cout << "MODE: SPRINT "; }
    else { setColor(GREEN); cout << "MODE: NORMAL "; }

    setColor(YELLOW);
    gotoxy(OFFSET_X, H + 2);
    double totalTime = (double)(clock() - sprintStartTime) / CLOCKS_PER_SEC;
    cout << "Total Time: " << (int)totalTime << "s    ";

    gotoxy(OFFSET_X, H + 3);
    if (gameMode == 1) {
        double timeLeft = 3.0 - ((double)(clock() - spawnTime) / CLOCKS_PER_SEC);
        if (timeLeft < 0) timeLeft = 0;

        setColor(RED);
        gotoxy(OFFSET_X, H + 3); // Đảm bảo đúng vị trí
        // Thêm khoảng trắng ở cuối để xóa ký tự thừa của lần vẽ trước
        printf("GRAVITY DROP IN: %.1fs    ", timeLeft);
    }
}

void drawNextBlock() {
    int startX = OFFSET_X + (W * 2) + 6;
    setColor(WHITE);
    gotoxy(startX, 2); cout << "NEXT:";
    for (int i = 0; i < 4; i++) {
        gotoxy(startX, 3 + i);
        for (int j = 0; j < 4; j++) {
            if (blocks[next_b][i][j] != ' ') {
                applyBlockColor(blocks[next_b][i][j]); cout << "[]";
            }
            else cout << "  ";
        }
    }
    setColor(WHITE);
}

void removeLine() {
    for (int i = H - 2; i > 0; i--) {
        int count = 0;
        for (int j = 1; j < W - 1; j++) if (board[i][j] != ' ') count++;
        if (count == W - 2) {
            for (int ii = i; ii > 0; ii--) for (int jj = 1; jj < W - 1; jj++) board[ii][jj] = board[ii - 1][jj];
            score += 10;
            linesCleared++; // THÊM DÒNG NÀY
            if (speed > 50) speed -= 5;
            i++;
        }
    }
}

int showSubMenu(string title) {
    int choice = 0;
    system("cls");

    int finalTime = (int)((clock() - sprintStartTime) / CLOCKS_PER_SEC);
    
    //  LẤY VÀ HIỂN THỊ KỶ LỤC KHI GAME OVER ---
    int highestRecord = getAndUpdateHighScore(score); 

    while (true) {
        int mx = OFFSET_X + 21;

        gotoxy(mx - 6, 4);
        setColor(WHITE);
        cout << "DIEM SO: " << score << " | THOI GIAN: " << finalTime << "s";

        // Nếu là màn hình GAME OVER thì in thêm dòng Kỷ lục cao nhất
        if (title == "GAME OVER!") {
            gotoxy(mx - 6, 5);
            setColor(YELLOW); // Màu vàng cho nổi bật
            cout << "KY LUC CAO NHAT: " << highestRecord << " DIEM";
        }

        gotoxy(mx, 8); setColor(YELLOW); cout << "=== " << title << " ===";

        string options[] = { "Choi lai", "Quay lai chon Mode", "Quay lai Menu chinh", "Thoat" };
        for (int i = 0; i < 4; i++) {
            gotoxy(mx , 10 + i);

            if (choice == i) {
                if (i == 3) setColor(RED);   
                else setColor(CYAN);         
                cout << "> " << options[i] << " <  ";
            }
            else {
                setColor(WHITE); 
                cout << "  " << options[i] << "      ";
            }
        }

        char c = _getch();
        if (c == 224) c = _getch(); 
        if (c == 'w' || c == 'W' || c == 72) choice = (choice - 1 + 4) % 4;
        if (c == 's' || c == 'S' || c == 80) choice = (choice + 1) % 4;
        if (c == 13) return choice;
    }
}
// Hàm kiểm tra, cập nhật và trả về điểm kỷ lục cao nhất
int getAndUpdateHighScore(int currentScore) {
    int highScore = 0;

    //  Đọc điểm kỷ lục cũ từ file lên
    ifstream fileIn("highscore.txt");
    if (fileIn >> highScore) {
    } else {
        highScore = 0; // Nếu file chưa có thì mặc định kỷ lục là 0
    }
    fileIn.close();

    // Nếu lượt chơi này phá kỷ lục, cập nhật kỷ lục mới vào file
    if (currentScore > highScore) {
        highScore = currentScore;
        ofstream fileOut("highscore.txt");
        fileOut << highScore;
        fileOut.close();
    }

    return highScore;
}

void handlePause() {
    int startX = OFFSET_X + (W * 2) + 6;
    setColor(YELLOW);
    gotoxy(startX, 10); cout << "=== PAUSED ===";
    gotoxy(startX, 11); cout << "Bam P de tiep tuc";
    gotoxy(startX, 12); cout << "Bam Q de thoat";
    
    while (true) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'p' || ch == 'P') {
                // Xóa chữ tạm dừng bên cạnh board khi chơi tiếp
                gotoxy(startX, 10); cout << "              ";
                gotoxy(startX, 11); cout << "                 ";
                gotoxy(startX, 12); cout << "              ";
                break;
            }
            if (ch == 'q' || ch == 'Q') {
                // Đồng bộ phím Q để thoát ra menu chính nếu đang tạm dừng
                system("cls");
                // Ép chương trình nhảy về nhãn start_menu    
                break; 
            }
        }
        Sleep(50);
    }
}
int main() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false; // Tắt con trỏ
    SetConsoleCursorInfo(out, &cursorInfo);

// NHÃN QUAY LẠI MENU CHÍNH
start_menu:
    int choice = menu();
    if (choice == 1) { showGuide(); goto start_menu; }
    if (choice == 2) return 0;

// NHÃN QUAY LẠI CHỌN CHẾ ĐỘ
choose_mode:
    int modeChoice = selectModeMenu();

    if (modeChoice == -1) {
        goto start_menu; // Quay lại menu chính ngay lập tức
    }

    // Nếu không phải -1, mới gán mode và chạy tiếp
    gameMode = modeChoice;
    isGravityMode = (modeChoice == 1);

    loadingScreen();

// NHÃN CHƠI LẠI (RESET THÔNG SỐ)
start_game:
    system("cls");
    initBoard();

    // Reset thông số game
    score = 0;
    speed = 200;
    linesCleared = 0;
    sprintStartTime = clock();
    b = rand() % 7;
    next_b = rand() % 7;
    x = 4; y = 0;

    bool hasDropped = false;
    clock_t lastTime = clock();
    spawnTime = clock();

    block2Board();
    draw();
    drawNextBlock();
    int lastSec = -1;
    while (1) {
            if (gameMode == 1) {
        double timeLeft = 3.0 - ((double)(clock() - spawnTime) / CLOCKS_PER_SEC);
        if (timeLeft < 0) timeLeft = 0;

        // Chỉ vẽ lại nếu số giây lẻ thay đổi (ví dụ 2.9 -> 2.8)
        // Hoặc đơn giản là vẽ lại mỗi 100ms
        if ((int)(timeLeft * 10) != lastSec) {
            draw();
            lastSec = (int)(timeLeft * 10);
        }
    }
        // --- LOGIC THẮNG SPRINT ---
        if (gameMode == 2) {
            gotoxy(OFFSET_X + 32, H - 5); setColor(CYAN);
            cout << "PROGRESS: " << linesCleared << "/" << sprintTarget << "  ";
            gotoxy(OFFSET_X + 32, H - 4); setColor(WHITE);
            cout << "TIME: " << (double)(clock() - sprintStartTime) / CLOCKS_PER_SEC << "s    ";

            if (linesCleared >= sprintTarget) {
                int subChoice = showSubMenu("BAN DA THANG!");
                if (subChoice == 0) goto start_game;
                if (subChoice == 1) goto choose_mode;
                if (subChoice == 2) goto start_menu;
                if (subChoice == 3) exit(0);
            }
        }
        if (gameMode == 1) draw();

       // 1. XỬ LÝ NHẬP PHÍM
        if (_kbhit()) {
            boardDelBlock();
            int c = _getch();
            //Tạm dừng khi đang chơi giữa chừng
            if (c == 'p' || c == 'P') {
                block2Board(); handlePause(); 
                lastTime = clock(); spawnTime = clock();
            }

            if (c == 0 || c == 224) c = _getch();

            // Di chuyển và xoay
            if ((c == 'a' || c == 'A' || c == 75) && canMove(-1, 0)) x--;
            if ((c == 'd' || c == 'D' || c == 77) && canMove(1, 0)) x++;
            if ((c == 's' || c == 'S' || c == 80) && canMove(0, 1)) y++;
            if (c == 'w' || c == 'W' || c == 72 || c == 32) {
                   // Nếu là block O
                if (b == 1)

                  // Gọi rotate của class OPiece
                  // Hàm này rỗng nên block O không xoay
                    oPiece.rotate(blocks[b]);
                else
                  // Các block khác dùng rotate bình thường
                    normalPiece.rotate(blocks[b]);
                // Kiểm tra nếu block sau khi xoay bị đụng tường
                if (!canMove(0, 0)) {

                    // Rollback bằng cách xoay thêm 3 lần
                    // Vì xoay 4 lần sẽ quay về trạng thái cũ
                    normalPiece.rotate(blocks[b]);
                    normalPiece.rotate(blocks[b]);
                    normalPiece.rotate(blocks[b]);
                }
            }

            // Rơi nhanh (Chỉ dùng cho Normal/Sprint)
            if ((c == 'x' || c == 'X') && !isGravityMode) {
                while(canMove(0, 1)) y++;
            }

            // Thoát ngang (Phải nằm TRONG ngoặc của _kbhit hoặc xử lý biến c cẩn thận)
            if (c == 'q' || c == 'Q') goto start_menu;

            block2Board();
            draw();
        }

        // 2. LOGIC TỰ ĐỘNG
        if (isGravityMode) {
            if (!hasDropped && (clock() - spawnTime >= 3000)) {
                boardDelBlock();
                applyGravity();
                hasDropped = true;
                block2Board();
                draw();
                lastTime = clock();
            }

            if (hasDropped && (clock() - lastTime >= speed)) {
                removeLine();
                x = 4; y = 0;
                b = next_b; next_b = rand() % 7;
                spawnTime = clock();
                hasDropped = false;

                if (!canMove(0, 0)) {
                    int subChoice = showSubMenu("GAME OVER!");
                    if (subChoice == 0) goto start_game;
                    if (subChoice == 1) goto choose_mode;
                    if (subChoice == 2) goto start_menu;
                    if (subChoice == 3) exit(0);
                }

                block2Board();
                draw();
                drawNextBlock();
            }
        }
        else {
            if (clock() - lastTime >= speed) {
                boardDelBlock();
                if (canMove(0, 1)) {
                    y++;
                    block2Board();
                } else {
                    block2Board();
                    removeLine();
                    x = 4; y = 0;
                    b = next_b; next_b = rand() % 7;
                    spawnTime = clock();
                    if (!canMove(0, 0)) {
                        int subChoice = showSubMenu("GAME OVER!");
                        if (subChoice == 0) goto start_game;
                        if (subChoice == 1) goto choose_mode;
                        if (subChoice == 2) goto start_menu;
                        if (subChoice == 3) exit(0);
                    }

                    block2Board();
                    drawNextBlock();
                }
                draw();
                lastTime = clock();
            }
        }
        Sleep(1);
    }
    return 0;
}
