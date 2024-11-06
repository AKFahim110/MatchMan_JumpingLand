#include <graphics.h>
#include <conio.h>
#include "EasyXPng.h"
#include "Timer.h"
#include <vector>
#include <cstdlib> // For rand()
#include <ctime>   // For time()
using namespace std;

#define WIDTH 800
#define HEIGHT 700
#define LAND_HEIGHT 30 // Height of the land
#define MAX_LANDS 5    // Maximum number of lands on screen at a time

enum PlayerStatus {
	standleft, standright, runleft, runright, jumpleft, jumpright, dead
};

struct Land {
	float x, y;
	IMAGE img;
	bool hasEnemy;
	bool hasStar; // Flag for star presence
	bool starCollected; // Flag to track if star is collected
};

class Player {
public:
	IMAGE im_show;
	IMAGE im_standright;
	IMAGE im_standleft;
	IMAGE im_jumpright;
	IMAGE im_jumpleft;
	IMAGE im_die;
	vector<IMAGE> ims_runright;
	vector<IMAGE> ims_runleft;
	int animId;
	PlayerStatus playerStatus;
	float x_left, y_bottom;
	float vx, vy;
	float gravity;
	float width, height;
	int score;
	bool isAlive;
	float jumpStrength;
	float maxFallSpeed;
	bool canJump;
	float initialY;
	float groundY;

	void draw() {
		if (isAlive) {
			putimagePng(x_left, y_bottom - height, &im_show);
		}
		else {
			putimagePng(x_left, y_bottom - height, &im_die);
		}

	}


	void initialize() {
		ims_runleft.clear();
		ims_runright.clear();

		loadimage(&im_standright, _T("Z:/Desktop/assets/hw6/MatchMan/standright.png"));
		loadimage(&im_standleft, _T("Z:/Desktop/assets/hw6/MatchMan/standleft.png"));
		loadimage(&im_jumpright, _T("Z:/Desktop/assets/hw6/MatchMan/jumpright.png"));
		loadimage(&im_jumpleft, _T("Z:/Desktop/assets/hw6/MatchMan/jumpleft.png"));

		playerStatus = standright;
		im_show = im_standright;
		width = im_standright.getwidth();
		height = im_standright.getheight();

		// Set the initial position to ground level
		initialY = HEIGHT - height; // Assuming ground is at the bottom of the window minus player height
		updateXY(WIDTH / 2, initialY); // Center horizontally and set to ground level

		TCHAR filename[80];
		for (int i = 0; i <= 7; i++) {
			_stprintf_s(filename, _T("Z:/Desktop/assets/hw6/MatchMan/runright%d.png"), i + 1);
			IMAGE im;
			loadimage(&im, filename);
			ims_runright.push_back(im);
		}
		for (int i = 0; i <= 7; i++) {
			_stprintf_s(filename, _T("Z:/Desktop/assets/hw6/MatchMan/runleft%d.png"), i + 1);
			IMAGE im;
			loadimage(&im, filename);
			ims_runleft.push_back(im);
		}

		animId = 0;
		score = 0;
		isAlive = true;
		jumpStrength = -30;  // Adjustable jump strength
		maxFallSpeed = 15;   // Maximum falling speed
		gravity = 3;
		vx = 10;
		vy = 0;
		canJump = true;

		updateXY(WIDTH / 2, HEIGHT / 2);
		initialY = HEIGHT / 2;
	}


	void updateXY(float mx, float my) {
		x_left = mx;
		y_bottom = my;
	}

	void runRight() {
		if (!isAlive) return;

		x_left += vx;
		if (playerStatus == jumpleft || playerStatus == jumpright) {
			im_show = im_jumpright; // Assuming you're jumping right
		}
		else {
			if (playerStatus != runright) {
				playerStatus = runright;
				animId = 0;
			}
			else {
				animId++;
				if (animId >= ims_runright.size())
					animId = 0;
			}
			im_show = ims_runright[animId];
		}

		if (x_left > WIDTH - width) {
			x_left = WIDTH - width;
		}
	}

	void runLeft() {
		if (!isAlive) return;

		x_left -= vx;
		if (playerStatus == jumpleft || playerStatus == jumpright) {
			im_show = im_jumpleft; // Assuming you're jumping left
		}
		else {
			if (playerStatus != runleft) {
				playerStatus = runleft;
				animId = 0;
			}
			else {
				animId++;
				if (animId >= ims_runleft.size())
					animId = 0;
			}
			im_show = ims_runleft[animId];
		}

		if (x_left < 0) {
			x_left = 0;
		}
	}

	void standStill() {
		if (!isAlive) return;

		if (playerStatus == runleft || playerStatus == standleft) {
			playerStatus = standleft;
			im_show = im_standleft;
		}
		else if (playerStatus == runright || playerStatus == standright) {
			playerStatus = standright;
			im_show = im_standright;
		}
	}

	void beginJump() {
		if (!isAlive || !canJump) return;

		// Set jump strength
		vy = jumpStrength;
		canJump = false; // Player can't jump again until they land

		// Determine the current jump animation
		if (playerStatus == runleft || playerStatus == standleft) {
			im_show = im_jumpleft;
			playerStatus = jumpleft;
		}
		else {
			im_show = im_jumpright;
			playerStatus = jumpright;
		}
	}

	void updateYcoordinate() {
		if (!isAlive) return;

		// Apply gravity
		if (!canJump) {
			vy += gravity;

			// Cap the falling speed
			if (vy > maxFallSpeed) {
				vy = maxFallSpeed;
			}
			y_bottom += vy;

			// Check if the player is below the ground level
			if (y_bottom > groundY) {
				y_bottom = groundY; // Reset to ground level
				land(y_bottom); // Call land method to reset vertical speed and allow jumping
			}
		}
	}



	void land(float landY) {
		y_bottom = landY;
		vy = 0; // Reset vertical speed
		canJump = true; // Allow jumping again

		// Set to standing animation after landing
		if (playerStatus == jumpleft) {
			playerStatus = standleft;
			im_show = im_standleft;
		}
		if (playerStatus == jumpright) {
			playerStatus = standright;
			im_show = im_standright;
		}
	}

	void fall() {
		vy += gravity;

		if (vy > maxFallSpeed) {
			vy = maxFallSpeed;
		}

		y_bottom += vy;

		// Check if the player is below a certain threshold
		if (y_bottom > HEIGHT + height) {
			// Only die if the player has been falling for a while or has no land
			if (vy > 0) { // Ensure the player is indeed falling
				die();
			}
		}
	}


	void die() {
		isAlive = false;
		playerStatus = dead;
		im_show = im_die;
		vy = 0;
	}

	void addScore(int points) {
		if (isAlive) {
			score += points;
		}
	}

	bool isJumping() const {
		return playerStatus == jumpleft || playerStatus == jumpright;
	}

	bool isFalling() const {
		return isJumping() && vy > 0;
	}

	float getCenterX() const {
		return x_left + width / 2;
	}

	float getCenterY() const {
		return y_bottom - height / 2;
	}

	float getFeetY() const {
		return y_bottom; // Feet Y coordinate
	}
};

// Global variables
IMAGE im_land;
IMAGE im_bk;
IMAGE im_star;
Player player;
Timer timer;
vector<Land> lands;
int bg_dx = 0;
int bg_speed = 1;
int land_speed = 3;
IMAGE enemy_img;
bool gameOver = false;

void startup() {
	player.initialize();
	loadimage(&im_land, _T("Z:/Desktop/assets/hw6/MatchMan/land.png"));
	loadimage(&im_bk, _T("Z:/Desktop/assets/hw6/MatchMan/landscape1.png"));
	loadimage(&im_star, _T("Z:/Desktop/assets/hw6/MatchMan/star.png"));

	lands.clear();
	for (int i = 0; i < MAX_LANDS; i++) {
		Land land;
		land.x = WIDTH + i * 200;
		land.y = (rand() % 2 == 0) ? HEIGHT / 2 + LAND_HEIGHT : HEIGHT / 2 - LAND_HEIGHT;
		land.img = im_land;
		land.hasEnemy = (rand() % 4 == 0);
		land.hasStar = (rand() % 3 == 0);
		land.starCollected = false;
		lands.push_back(land);
	}

	initgraph(WIDTH, HEIGHT);
	srand(static_cast<unsigned int>(time(0)));
	BeginBatchDraw();
}

void drawGameOver() {
	settextcolor(RED);
	settextstyle(48, 0, _T("Arial"));
	outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 24, _T("Game Over!"));
	settextstyle(32, 0, _T("Arial"));
	TCHAR scoreText[50];
	_stprintf_s(scoreText, _T("Score: %d"), player.score);
	outtextxy(WIDTH / 2 - 60, HEIGHT / 2 + 30, scoreText);
	EndBatchDraw();
}

void updateWithInput() {
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		player.runLeft();
	}
	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		player.runRight();
	}
	else {
		player.standStill();
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		player.beginJump();
	}
	player.updateYcoordinate();
}

void checkCollision() {
	bool isOnLand = false;
	for (auto& land : lands) {
		// Check if the player is above the current land segment
		if (land.x < player.getCenterX() && player.getCenterX() < land.x + im_land.getwidth()) {
			// Check if player’s feet are within the land's vertical range
			if (land.y <= player.getFeetY() && player.getFeetY() <= land.y + LAND_HEIGHT) {
				player.land(land.y); // Snap player to land height
				isOnLand = true; // Player is on land
				break; // Stop checking once we've landed
			}
		}
	}
	// If player is not on any land, they should fall
	if (!isOnLand) {
		player.fall(); // Trigger falling state if no land beneath
	}
}

void checkStarCollection() {
	for (auto& land : lands) {
		if (land.hasStar && !land.starCollected) {
			float starX = land.x + im_star.getwidth() / 2;
			float starY = land.y - im_star.getheight();
			// Check if player is close enough to collect the star
			if (abs(player.getCenterX() - starX) < 30 && abs(player.getCenterY() - starY) < 30) {
				player.addScore(10); // Increase score
				land.starCollected = true; // Mark star as collected
			}
		}
	}
}


void scrollBackground() {
	bg_dx -= bg_speed;
	if (bg_dx < -im_bk.getwidth()) {
		bg_dx = 0;
	}
}

void scrollLands() {
	for (auto& land : lands) {
		land.x -= land_speed;
		if (land.x < -im_land.getwidth()) {
			land.x = WIDTH;
			land.y = (rand() % 2 == 0) ? HEIGHT / 2 + LAND_HEIGHT : HEIGHT / 2 - LAND_HEIGHT;
			land.hasEnemy = (rand() % 4 == 0);
			land.hasStar = (rand() % 3 == 0);
			land.starCollected = false;
		}
	}
}

void updateWithoutInput() {
	scrollBackground();
	scrollLands();
	checkCollision();
	checkStarCollection();
	if (!player.isAlive) {
		gameOver = true;
	}
}

void render() {
	putimage(bg_dx, 0, &im_bk);
	putimage(bg_dx + im_bk.getwidth(), 0, &im_bk);

	for (const auto& land : lands) {
		putimage(land.x, land.y, &land.img);
		if (land.hasStar && !land.starCollected) {
			putimage(land.x + 20, land.y - im_star.getheight(), &im_star);
		}
	}

	player.draw();

	settextcolor(WHITE);
	TCHAR scoreText[50];
	_stprintf_s(scoreText, _T("Score: %d"), player.score);
	outtextxy(10, 10, scoreText);
	EndBatchDraw();
}

void gameLoop() {
	while (true) {
		if (gameOver) {
			drawGameOver();
			break;
		}
		updateWithInput();
		updateWithoutInput();
		render();
		FlushBatchDraw();
		Sleep(10);
	}
	_getch();
	closegraph();
}

int main() {
	startup();
	gameLoop();
	return 0;
}
