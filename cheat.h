#pragma once
#include <iostream>
#include <fstream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
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