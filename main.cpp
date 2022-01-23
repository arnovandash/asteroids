#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <utility>

typedef float Point[3];


class Game : public olc::PixelGameEngine
{
public:
	Game()
	{
		sAppName = "Asteroids";
	}

private:
	struct sSpaceObject {
		int nSize;
		float x, y, dx, dy;
		float angle;
	};

	std::vector<sSpaceObject> vecAsteroids;
	std::vector<sSpaceObject> vecBullets;
	sSpaceObject player;
	bool bDead = false;
	int nScore = 0;

	std::vector<std::pair<float, float>> vecModelShip;
	std::vector<std::pair<float, float>> vecModelAsteroid;


protected:
	bool OnUserCreate() override
	{
		vecModelShip = {
			{ 0.0f, -5.0f },
			{ -2.5f, +2.5f },
			{ +2.5f, +2.5f }
		}; // An isoceles triangle

		// Generate an asteroid
		int verts = 20;
		srand(time(0));
		for (int i = 0; i < verts; i++) {
			float radius = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
			float a = ((float)i / (float)verts) * 6.28318f; // Divides the circle up into degrees per segment; 6.28318 = 2PI
			vecModelAsteroid.push_back(std::make_pair(radius * sinf(a), radius * cosf(a)));
		}

		ResetGame();
		return true;
	}

	bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
	}

	void ResetGame() {
		vecAsteroids.clear(); 
		vecBullets.clear();

		vecAsteroids.push_back({ (int)32, 30.0f, 60.0f, 20.0f, -6.0f, 0.00f });
		vecAsteroids.push_back({ (int)32, 120.0f, 20.0f, 80.0f, 5.0f, 0.00f });
		vecAsteroids.push_back({ (int)32, 20.0f, 20.0f, 8.0f, -6.0f, 0.0f });
		vecAsteroids.push_back({ (int)32, 100.0f, 20.0f, -5.0f, 3.0f, 0.0f });

		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.nSize = 2;
		player.angle = 0.0f;
		bDead = false;
		nScore = 0;
	}

 	bool OnUserUpdate(float fElapsedTime) override
	{
		if (bDead)
			ResetGame();

		// Escape to quit
		if (GetKey(olc::Key::ESCAPE).bHeld) return false;

		// called once per frame
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

		// Player Rotation
		if (GetKey(olc::Key::LEFT).bHeld) player.angle -= 5.0f * fElapsedTime;
		if (GetKey(olc::Key::RIGHT).bHeld) player.angle += 5.0f * fElapsedTime;

		// Player Thrust
		if (GetKey(olc::Key::UP).bHeld) {
			// Acceleration changes velocity
			player.dx += sin(player.angle) * 60.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 60.0f * fElapsedTime;
		}

		// Player collision check with asteroids
		for (auto& a : vecAsteroids) {
				if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y)) {
					bDead = true;//asteroid hit
					ResetGame();
				}
			}

		// Fire bullets
		if (GetKey(olc::Key::SPACE).bReleased) {
			vecBullets.push_back({0, player.x, player.y, 100.0f * sinf(player.angle), 100.0f * -cosf(player.angle), 0 });
		}

		// Velocity changes position
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		WrapCoordinates(player.x, player.y, player.x, player.y);

		
		// Update and draw asteroids
		for (auto &a : vecAsteroids) {
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;

			WrapCoordinates(a.x, a.y, a.x, a.y);
			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, (float)a.nSize, olc::WHITE);
		}

		std::vector<sSpaceObject> newAsteroids;

		// Update and draw bullets
		for (auto &b : vecBullets) {
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;

			// Collision check with asteroids
			for (auto& a : vecAsteroids) {
				if (IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y)) {
					//asteroid hit
					b.x = -10;

					if (a.nSize > 4) {
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						// bitshift >> 1, divides by 2
						newAsteroids.push_back({ (int)a.nSize >> 1, a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), 0.0f });
						newAsteroids.push_back({ (int)a.nSize >> 1, a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), 0.0f });
						// Move parent asteroid off screen thereby deleting it
					}
					a.x = -100;
					nScore += 100;
				}
			}
			//WrapCoordinates(b.x, b.y, b.x, b.y);
			Draw(b.x, b.y, olc::WHITE);
		}


		// Append new asteroids to existing vecotr
		for (auto a : newAsteroids)
			vecAsteroids.push_back(a);

		// Remove off-screen bullets
		if (vecBullets.size() > 0) {
			auto i = remove_if(vecBullets.begin(), vecBullets.end(),
				[&](sSpaceObject o) {return (o.x < 1 || o.y < 1 || o.x > ScreenWidth() - 1 || o.y > ScreenHeight() - 1); });
			if (i != vecBullets.end())
				vecBullets.erase(i);
		}

		// Remove off-screen asteroids
		if (vecAsteroids.size() > 0) {
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), [&](sSpaceObject o) { return (o.x < 0); });
			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}


		// If no asteroids, level complete! :) - you win MORE asteroids!
		if (vecAsteroids.empty()) 
		{
			// Level Clear
			nScore += 1000; // Large score for level progression
			vecAsteroids.clear();
			vecBullets.clear();

			// Add two new asteroids, but in a place where the player is not, we'll simply
			// add them 90 degrees left and right to the player, their coordinates will
			// be wrapped by th enext asteroid update
			vecAsteroids.push_back({ (int)16, 30.0f * sinf(player.angle - 3.14159f / 2.0f) + player.x,
											  30.0f * cosf(player.angle - 3.14159f / 2.0f) + player.y,
											  10.0f * sinf(player.angle), 10.0f * cosf(player.angle), 0.0f });

			vecAsteroids.push_back({ (int)16, 30.0f * sinf(player.angle + 3.14159f / 2.0f) + player.x,
											  30.0f * cosf(player.angle + 3.14159f / 2.0f) + player.y,
											  10.0f * sinf(-player.angle), 10.0f * cosf(-player.angle), 0.0f });
		}


		// Draw ship
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle, player.nSize);

		// Draw Score
		DrawString(2, 2, "SCORE: " + std::to_string(nScore), olc::WHITE, 1);
		return true;
	}

	void DrawWireFrameModel(const std::vector<std::pair<float, float>>& vecModelCoordinates, float x, float y, float rotation = 0.0f, float scale = 1.0f, olc::Pixel col = olc::WHITE) {
		// pair.first = x coordinate
		// pair.second = y coordinate

		// Create translated model vector of coordinate pairs
		std::vector<std::pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotate the model
		for (int i = 0; i < verts; i++) {
			/* Using 2D rotation matrix */
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(rotation) - vecModelCoordinates[i].second * sinf(rotation);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first  * sinf(rotation) + vecModelCoordinates[i].second * cosf(rotation);
		}

		// Scale the model
		for (int i = 0; i < verts; i++) {
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * scale;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * scale;
		}

		// Translate the model
		for (int i = 0; i < verts; i++) {
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		// Draw closed polygon
		for (int i = 0; i < verts + 1; i++) {
			int j = (i + 1);
			//DrawLine(vecTransformedCoordinates[i % 3].first, vecTransformedCoordinates[i % 3].second, vecTransformedCoordinates[j % 3].first, vecTransformedCoordinates[j % 3].second);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second, 
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, col);
		}
	}


	void WrapCoordinates(float ix, float iy, float& ox, float& oy) {
		ox = ix;
		oy = iy;
		if (ix < 0.0f) ox = ix + (float)ScreenWidth();
		if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();

		if (iy < 0.0f) oy = iy + (float)ScreenHeight();
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
	}

	virtual bool Draw(int32_t x, int32_t y, olc::Pixel p = olc::WHITE) {
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);
		PixelGameEngine::Draw(fx, fy, p);
		return true;
	}
};


int main()
{
	Game asteroids;
	if (asteroids.Construct(640, 480, 2, 2))
		asteroids.Start();

	return 0;
}