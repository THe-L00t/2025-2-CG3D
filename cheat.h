#pragma once
#include <iostream>
#include <fstream>
#include <gl/glew.h> //--- ÇÊ¿äÇÑ Çì´õÆÄÀÏ include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <random>
#include <array>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>

#ifndef CHEAT
#define CHEAT

std::array<std::array<float, 18>, 6> cube = { {
		// ¸é 1: ¾Õ¸é (z = 0.5)
	   {-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f},

		// ¸é 2: µÞ¸é (z = -0.5)
		{-0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
		 -0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f},

		 // ¸é 3: ¿ÞÂÊ¸é (x = -0.5)
		 {-0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
		  -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f},

		  // ¸é 4: ¿À¸¥ÂÊ¸é (x = 0.5)
		 { 0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f},

		 // ¸é 5: À­¸é (y = 0.5)
	 {-0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
	  -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f},

	  // ¸é 6: ¾Æ·§¸é (y = -0.5)
 {-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f}
} };

std::array<std::array<float, 18>, 5> pyramid = { {
		// ¸é 1: ¹Ù´Ú¸é (y = -0.5, »ç°¢Çü = »ï°¢Çü 2°³)
		{-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
		 -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f},

		 // ¸é 2: ¾Õ¸é »ï°¢Çü (z = 0.5 ÂÊ)
		 {-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f,
		  -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f},

		  // ¸é 3: ¿À¸¥ÂÊ¸é »ï°¢Çü (x = 0.5 ÂÊ)
		  { 0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f,
			0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f},

			// ¸é 4: µÞ¸é »ï°¢Çü (z = -0.5 ÂÊ)
			{ 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f,
			  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f},

			  // ¸é 5: ¿ÞÂÊ¸é »ï°¢Çü (x = -0.5 ÂÊ)
			  {-0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f,
			   -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f}
} };

std::array<std::array<float, 4>, 6> col = { {
	{1,0,0,1},	// R
	{0,1,0,1},	// G
	{0,0,1,1},	// B
	{1,1,0,1},	// Y
	{1,0,1,1},	// M
	{0,1,1,1}	// c
} };

struct Point {
	const Point& operator+(const Point& other) {
		return Point{ x + other.x, y + other.y };
	}
	const Point& operator-(const Point& other) {
		return Point{ x - other.x, y - other.y };
	}
	bool operator<(const Point& other) {
		return x < other.x and y < other.y;
	}
	const Point& operator*(float& other) {
		return Point{ x * other, y * other };
	}
	Point operator*(const float& other) {
		return Point{ x * other, y * other };
	}
	float x;
	float y;
};

struct Color {
	float r{};
	float g{};
	float b{};
	float al{};
};

struct rect {
	Point RT{}, LB{};
	Color c{};
};

class GameTimer
{
public:
	static GameTimer* Instance;

public:
	GameTimer()
	{
		if (Instance == nullptr) Instance = this;
		startTime = std::chrono::high_resolution_clock::now();
		prevTime = startTime;
	}
	void Update()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - prevTime);
		deltaTime = static_cast<float>(duration.count()) / 1000;
		timer += deltaTime;
		prevTime = end;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> prevTime;
public:
	float timer = 0;
	float deltaTime = 0;
}; 


#endif