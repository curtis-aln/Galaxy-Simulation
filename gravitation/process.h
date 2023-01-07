#pragma once

#include <algorithm>
#include <execution>
#include <numeric>

#include <iostream>


double wrappedDistanceSquared(sf::Vector2f position1, sf::Vector2f position2, int screenWidth, int screenHeight) {
	float dx = abs(position2.x - position1.x);
	float  dy = abs(position2.y - position1.y);

	if (dx > screenWidth / 2)
		dx = screenWidth - dx;
	if (dy > screenHeight / 2)
		dy = screenHeight - dy;

	return dx*dx + dy*dy;
}

sf::Vector2f directionCalculator(sf::Vector2f position1, sf::Vector2f position2, int screenWidth, int screenHeight) {
	float StartX = position1.x; float StartY = position1.y;
	float EndX = position2.x; float EndY = position2.y;

	float dist_x = abs(StartX - EndX);
	float result_x = 0.0f;

	if (dist_x < screenWidth / 2)
		result_x = dist_x;
	else
		result_x = dist_x - screenWidth;

	if (StartX > EndX)
		result_x *= -1;


	float dist_y = abs(StartY - EndY);
	float result_y = 0;

	if (dist_y < screenHeight / 2)
		result_y = dist_y;
	else
		result_y = dist_y - screenHeight;

	if (StartY > EndY)
		result_y *= -1;

	return sf::Vector2f(result_x, result_y);
}


sf::Vector2f gravitate(float fa, sf::VertexArray* blackHoles, sf::Vertex &point, sf::Vector2f &velocity,
	int screenWidth, int screenHeight) {
	for (int i = 0; i < (*blackHoles).getVertexCount(); i++) {
		float force = (fa / wrappedDistanceSquared(point.position, (*blackHoles)[i].position, screenWidth, screenHeight)) * 0.01;
		sf::Vector2f direction = directionCalculator(point.position, (*blackHoles)[i].position, screenWidth, screenHeight);
		velocity += (direction * force);
	}
	return velocity;
}


sf::Vector2f BHgravitate(float fa, sf::VertexArray* blackHoles, sf::Vector2f &position, sf::Vector2f &velocity,
	int screenWidth, int screenHeight) {

	for (size_t i = 0; i < (*blackHoles).getVertexCount(); i++) {
		if ((*blackHoles)[i].position != position) {
			float d = wrappedDistanceSquared(position, (*blackHoles)[i].position, screenWidth, screenHeight);
			float force = fa / d;
			sf::Vector2f direction = directionCalculator(position, (*blackHoles)[i].position, screenWidth, screenHeight);

			velocity += direction * force;
		}
	}
	return velocity;
}


sf::Vector2f speed_limit(int max_speed, sf::Vector2f &velocity) {
	if (velocity.x > max_speed)
		velocity.x = max_speed;

	else if (velocity.x < -max_speed)
		velocity.x = -max_speed;

	if (velocity.y < -max_speed)
		velocity.y = -max_speed;
	else if (velocity.y > max_speed)
		velocity.y = max_speed;
	
	return velocity;
}


sf::Vector2f border(int borderX, int borderY, sf::Vector2f &position, sf::Vector2f &velocity) {
	if (position.x > borderX)
		position.x = 0;
	
	else if (position.x < 0)
		position.x = borderX;

	if (position.y < 0)
		position.y = borderY;
	
	else if (position.y > borderY)
		position.y = 0;
	
	return position;
}
