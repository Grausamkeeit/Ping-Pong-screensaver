#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <utility>

const int width = 50;
const int height = 20;
int ballX, ballY, ballDirX, ballDirY;
int score1, score2;
int paddle1Y, paddle2Y;
const int paddleSize = 6;
bool isLeftAIStronger;
const int scoreToWin = 5; // Score needed to win the game
int strongAIDelay;
int weakAIDelay;

void ResetBall() {
    ballX = width / 2;
    ballY = height / 2;
    ballDirX = (rand() % 2) * 2 - 1; // Random horizontal direction
    ballDirY = (rand() % 2) * 2 - 1; // Random vertical direction
}

void Setup() {
    srand(time(0));
    isLeftAIStronger = rand() % 2; // Randomly choose the stronger AI
    ResetBall();
    paddle1Y = paddle2Y = height / 2 - paddleSize / 2;
    score1 = score2 = 0;
    strongAIDelay = rand() % 2 + 1; // Random delay from 1 to 3 for the strong AI
    weakAIDelay = strongAIDelay + (rand() % 2 ? 1 : 2); // Delay for the weak AI by 1 or 2 more
}

void Draw() {
    clear();
    for (int i = 0; i < width; ++i) {
        mvaddch(0, i, '#'); // Top border
        mvaddch(height - 1, i, '#'); // Bottom border
    }
    for (int i = 1; i < height - 1; ++i) {
        mvaddch(i, 0, '#'); // Left border
        mvaddch(i, width - 1, '#'); // Right border
    }
    mvaddch(ballY, ballX, 'O'); // Ball
    for (int i = 0; i < paddleSize; ++i) {
        mvaddch(paddle1Y + i, 1, '|'); // Left paddle
        mvaddch(paddle2Y + i, width - 2, '|'); // Right paddle
    }
    char scoreText[40];
    sprintf(scoreText, "P1 Score: %d | P2 Score: %d", score1, score2); // Score text
    int scorePos = (width - strlen(scoreText)) / 2; // Center position for score
    mvprintw(0, scorePos, "%s", scoreText); // Print score at the top center
    refresh();
}

void AnimateFireworks() {
    int min_fireworks = width / 4; // Minimum number of fireworks - 1/4 width of the field
    int max_fireworks = (3 * width) / 4; // Maximum number - 3/4 width of the field
    int num_fireworks = rand() % (max_fireworks - min_fireworks + 1) + min_fireworks; // Random number of fireworks

    std::vector<std::pair<int, int>> positions(num_fireworks);
    std::vector<int> explosion_heights(num_fireworks);
    std::vector<int> explosion_radii(num_fireworks);

    // Initialize starting positions of fireworks, explosion heights, and radii
    for (int i = 0; i < num_fireworks; ++i) {
        positions[i] = {rand() % width, height - 2}; // Start from the bottom
        explosion_heights[i] = rand() % ((height / 2) - (height / 3)) + (height / 3); // Explosion at a random height starting from 1/3 of the field
        explosion_radii[i] = rand() % 3 + 1; // Random explosion radius from 1 to 3
    }

    bool exploded[num_fireworks] = {false};
    while (true) {
        bool all_exploded = true;
        clear();
        for (int i = 0; i < num_fireworks; ++i) {
            if (!exploded[i] && positions[i].second <= explosion_heights[i]) {
                // Draw explosion in the shape of a sphere
                for (int y = -explosion_radii[i]; y <= explosion_radii[i]; y++) {
                    for (int x = -explosion_radii[i]; x <= explosion_radii[i]; x++) {
                        if (x * x + y * y <= explosion_radii[i] * explosion_radii[i]) {
                            int explosionX = positions[i].first + x;
                            int explosionY = positions[i].second + y;
                            if (explosionX >= 0 && explosionX < width && explosionY >= 0 && explosionY < height) {
                                mvaddch(explosionY, explosionX, '*');
                            }
                        }
                    }
                }
                exploded[i] = true;
            } else if (!exploded[i]) {
                // Draw ascending firework
                mvaddch(positions[i].second, positions[i].first, '*');
                positions[i].second--; // Firework ascends
                all_exploded = false;
            }
        }
        refresh();
        if (all_exploded) {
            usleep(1000000); // Delay 1 second after all fireworks have exploded
            break;
        }
        usleep(200000); // Delay between animation updates
    }
}

void DisplayWinner() {
    clear();
    const char* winnerText = score1 >= scoreToWin ? "AI 1 (Left) Wins!" : "AI 2 (Right) Wins!";
    mvprintw(height / 2, (width - strlen(winnerText)) / 2, "%s", winnerText); // Print winner text
    refresh();
    usleep(2000000); // Pause before the fireworks

    // Start fireworks animation
    AnimateFireworks();
    getch(); // Wait for a key press to exit
}

void MovePaddleAI(int &paddleY, int ballY, bool isStronger, int ballDirX) {
    static int delayCounterStrong = 0;
    static int delayCounterWeak = 0;
    int delay = isStronger ? strongAIDelay : weakAIDelay;
    int &delayCounter = isStronger ? delayCounterStrong : delayCounterWeak;

    // Reaction time to ball movement
    if (++delayCounter >= delay) {
        // Move down if the ball is below the paddle
        if (paddleY + paddleSize / 2 < ballY && paddleY + paddleSize <= height - 2) {
            paddleY++;
        }
        // Move up if the ball is above the paddle
        else if (paddleY + paddleSize / 2 > ballY && paddleY > 1) {
            paddleY--;
        }
        delayCounter = 0;
    }
}

void InputAI() {
    MovePaddleAI(paddle1Y, ballY, isLeftAIStronger, ballDirX);
    MovePaddleAI(paddle2Y, ballY, !isLeftAIStronger, ballDirX);
}

void Logic() {
    ballX += ballDirX;
    ballY += ballDirY;
    // Bounce off the top and bottom
    if (ballY == 1 || ballY == height - 2) ballDirY = -ballDirY;
    // Bounce off paddles
    if (ballX == 2 && ballY >= paddle1Y && ballY < paddle1Y + paddleSize) ballDirX = -ballDirX;
    if (ballX == width - 3 && ballY >= paddle2Y && ballY < paddle2Y + paddleSize) ballDirX = -ballDirX;
    // Scoring
    if (ballX == 1) {
        score2++;
        if (score2 >= scoreToWin) return;
        ResetBall();
    }
    if (ballX == width - 2) {
        score1++;
        if (score1 >= scoreToWin) return;
        ResetBall();
    }
}

int main() {
    initscr(); // Initialize ncurses
    cbreak(); // Disable line buffering
    noecho(); // Don't echo keypresses
    curs_set(0); // Hide cursor
    keypad(stdscr, TRUE); // Enable keyboard mapping
    nodelay(stdscr, TRUE); // Make getch non-blocking

    bool gameOver = false;
    while (!gameOver) {
        Setup();
        while (score1 < scoreToWin && score2 < scoreToWin) {
            Draw();
            InputAI();
            Logic();
            if (getch() == 'q') { // Press 'q' to quit
                gameOver = true;
                break;
            }
            usleep(50000); // 50 ms delay
        }
        if (!gameOver) {
            DisplayWinner();
            score1 = 0; // Reset scores
            score2 = 0;
        }
    }

    endwin(); // End ncurses mode
    return 0;
}
