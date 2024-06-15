#pragma once

template<typename Type>
sf::Vector2<Type> toroidal_direction(const sf::Vector2<Type> start, const sf::Vector2<Type> end, const sf::Rect<Type>& bounds)
{
	const Type dist_x = abs(start.x - end.x);
	const Type dist_y = abs(start.y - end.y);

	Type result_x = 0.0f;
	if (dist_x < bounds.width / 2)
		result_x = dist_x;
	else
		result_x = dist_x - bounds.width;

	if (start.x > end.x)
		result_x *= -1;


	Type result_y = 0.f;
	if (dist_y < bounds.height / 2)
		result_y = dist_y;
	else
		result_y = dist_y - bounds.height;

	if (start.y > end.y)
		result_y *= -1;

	return { result_x, result_y };
}

template<typename Type>
Type toroidal_distance_sq(const sf::Vector2f& position1, const sf::Vector2f& position2, const sf::Rect<Type>& bounds)
{
	Type dx = abs(position2.x - position1.x);
	Type dy = abs(position2.y - position1.y);

	if (dx > bounds.width / 2)
		dx = bounds.width - dx;
	if (dy > bounds.height / 2)
		dy = bounds.height - dy;

	return dx * dx + dy * dy;
}

template<typename Type>
Type toroidal_distance(const sf::Vector2<Type>& vector_1, const sf::Vector2<Type>& vector_2, const sf::Rect<Type>& bounds)
{
	return sqrt(toroidal_distance_sq(vector_1, vector_2, bounds));
}
