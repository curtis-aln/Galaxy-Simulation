#pragma once

#include <iostream>


inline double wrapped_distance_squared(const sf::Vector2f position1, const sf::Vector2f position2, const unsigned screen_width, const unsigned screen_height)
{
	float dx = abs(position2.x - position1.x);
	float dy = abs(position2.y - position1.y);

	if (dx > screen_width / 2)
		dx = screen_width - dx;
	if (dy > screen_height / 2)
		dy = screen_height - dy;

	return dx*dx + dy*dy;
}

inline sf::Vector2f direction_calculator(const sf::Vector2f position1, const sf::Vector2f position2, const unsigned screen_width, const unsigned screen_height)
{
	const float start_x = position1.x;
	const float start_y = position1.y;
	const float end_x = position2.x;
	const float end_y = position2.y;

	const float dist_x = abs(start_x - end_x);
	const float dist_y = abs(start_y - end_y);

	float result_x = 0.0f;
	if (dist_x < screen_width / 2)
		result_x = dist_x;
	else
		result_x = dist_x - screen_width;

	if (start_x > end_x)
		result_x *= -1;


	
	float result_y = 0.f;
	if (dist_y < screen_height / 2)
		result_y = dist_y;
	else
		result_y = dist_y - screen_height;

	if (start_y > end_y)
		result_y *= -1;

	return {result_x, result_y};
}


inline sf::Vector2f gravitate(const float gravitational_const, sf::VertexArray* black_holes, const sf::Vertex &point, sf::Vector2f &velocity, const unsigned screen_width, const unsigned screen_height)
{
	const size_t black_hole_count = black_holes->getVertexCount();
	for (size_t i = 0; i < black_hole_count; i++) 
	{
		const sf::Vector2f black_hole_center = (*black_holes)[i].position;
		const float wrapped_dist_sq = wrapped_distance_squared(point.position, black_hole_center, screen_width, screen_height);
		const float force = gravitational_const * (1 / wrapped_dist_sq);
		sf::Vector2f direction = direction_calculator(point.position, black_hole_center, screen_width, screen_height);
		velocity += (direction * force);
	}
	return velocity;
}


sf::Vector2f BHgravitate(float fa, sf::VertexArray* blackHoles, sf::Vector2f &position, sf::Vector2f &velocity,
	int screenWidth, int screenHeight) {

	for (size_t i = 0; i < (*blackHoles).getVertexCount(); i++) {
		if ((*blackHoles)[i].position != position) {
			float d = wrapped_distance_squared(position, (*blackHoles)[i].position, screenWidth, screenHeight);
			float force = fa / d;
			sf::Vector2f direction = direction_calculator(position, (*blackHoles)[i].position, screenWidth, screenHeight);

			velocity += direction * force;
		}
	}
	return velocity;
}


inline void speed_limit(const float max_speed, sf::Vector2f &velocity)
{
	const float speed_sq = velocity.x * velocity.x + velocity.y * velocity.y;

	if (speed_sq > max_speed * max_speed)
	{
		const float speed = sqrt(speed_sq);
		const sf::Vector2f norm_vel = velocity / speed;
		velocity = norm_vel * max_speed;
	}
}


inline void border(const sf::Rect<float>& bounds, sf::Vector2f &position)
{
	if (position.x > bounds.left + bounds.width)
		position.x -= bounds.left + bounds.width;
	
	else if (position.x < bounds.left)
		position.x += bounds.left + bounds.height;

	if (position.y < bounds.top)
		position.y += bounds.top + bounds.height;
	
	else if (position.y > bounds.top + bounds.height)
		position.y -= bounds.top + bounds.height;
}
