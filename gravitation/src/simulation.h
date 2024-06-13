#pragma once

#include <SFML/Graphics.hpp>
#include "../process.h"

#include <thread>

#include <string>

#include <sstream>
#include "settings.h"

#include "random.h"

class Simulation : SimulationSettings, SFMLSettings
{
private:
	bool paused_ = false;
	bool draw_ = true;

	sf::RenderWindow window_{};
	sf::Clock clock_{};

	sf::VertexArray stars_ = sf::VertexArray(sf::Points, number_of_stars);
	std::vector<sf::Vector2f> star_velocities_ = std::vector<sf::Vector2f>(number_of_stars);

	sf::VertexArray black_holes_ = sf::VertexArray(sf::Points, number_of_black_holes);
	std::vector<sf::Vector2f> black_hole_velocities_ = std::vector<sf::Vector2f>(number_of_black_holes);

	sf::RenderStates states_{};
	sf::Transform transform_{};


public:
	Simulation()
		: window_(sf::VideoMode(screen_width, screen_height), title)
	{
		window_.setFramerateLimit(max_fps);
		window_.setVerticalSyncEnabled(v_sync);
		window_.resetGLStates();

		transform_.scale(simulation_scale);
		states_.transform = transform_;
		states_.blendMode = sf::BlendAdd;


		// initializing the stars
		for (size_t i = 0; i < star_velocities_.size(); i++)
		{
			stars_[i].position = Random::rand_pos_in_rect(bounds);
			stars_[i].color = sf::Color(star_color, star_color, star_color);
			star_velocities_[i] = Random::rand_vector(-initial_star_velocity, initial_star_velocity);
		}

		// initializing the black holes
		for (size_t i = 0; i < black_hole_velocities_.size(); i++) 
		{
			black_holes_[i].position = Random::rand_pos_in_rect(bounds);
			black_holes_[i].color = sf::Color(255, 20, 255);
			black_hole_velocities_[i] = Random::rand_vector(-initial_bh_velocity, initial_bh_velocity);
		}
	}

	void run()
	{
		while (window_.isOpen())
		{
			handle_events();
			update_stars();
			update_black_holes();
			render();
		}
	}


private:
	void handle_events()
	{
		sf::Event event;
		while (window_.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window_.close();

			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Space)
					paused_ = not paused_;

				else if (event.key.code == sf::Keyboard::D)
					draw_ = not draw_;
			}
		}
	}


	void update_batch_of_stars(const unsigned begin_index, const unsigned end_index)
	{
		if (end_index > number_of_stars)
			return;

		for (size_t i = begin_index; i < end_index; ++i) 
		{
			sf::Vertex& point = stars_[i];
			sf::Vector2f& vel = star_velocities_[i];

			gravitate(stars_[i], vel);
			speed_limit(vel);
			border(point.position);

			point.position += vel;
		}
	}


	void update_stars()
	{
		// updating particles
		unsigned prev_i = 0;
		std::vector<std::thread> threads;
		threads.reserve(number_of_stars / threading_batches);

		for (size_t i = threading_batches; i <= number_of_stars; i += threading_batches)
		{
			threads.emplace_back([this, prev_i, i]() {
				update_batch_of_stars(prev_i, i);});
			prev_i = i;
		}

		for (auto& th : threads)
		{
			th.join();
		}
	}

	void update_black_holes()
	{
		for (size_t i = 0; i < number_of_black_holes; i++)
		{
			BHgravitate(black_holes_[i].position, black_hole_velocities_[i]);
			speed_limit(black_hole_velocities_[i]);

			border(black_holes_[i].position);
			black_holes_[i].position += black_hole_velocities_[i];
		}

	}

	void render()
	{
		if (draw_ == true) 
		{
			window_.clear();

			window_.draw(stars_, states_);
			window_.draw(black_holes_, transform_);

			window_.display();
		}

		// FPS management
		const sf::Int32 milliseconds_per_frame = clock_.restart().asMilliseconds();

		std::ostringstream oss;
		oss << title << milliseconds_per_frame << " ms/f";
		const std::string var = oss.str();
		window_.setTitle(var);
	}


	static void speed_limit(sf::Vector2f& velocity)
	{
		const float speed_sq = velocity.x * velocity.x + velocity.y * velocity.y;

		if (speed_sq > cosmic_speed_limit * cosmic_speed_limit)
		{
			const float speed = sqrt(speed_sq);
			const sf::Vector2f norm_vel = velocity / speed;
			velocity = norm_vel * cosmic_speed_limit;
		}
	}


	static void border(sf::Vector2f& position)
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


	sf::Vector2f gravitate(const sf::Vertex& point, sf::Vector2f& velocity)
	{
		const size_t black_hole_count = black_holes_.getVertexCount();
		for (size_t i = 0; i < black_hole_count; i++)
		{
			const sf::Vector2f black_hole_center = black_holes_[i].position;
			const float wrapped_dist_sq = wrapped_distance_squared(point.position, black_hole_center);
			const float force = gravitational_constant * (1 / wrapped_dist_sq);
			sf::Vector2f direction = direction_calculator(point.position, black_hole_center);
			velocity += (direction * force);
		}
		return velocity;
	}


	sf::Vector2f BHgravitate(const sf::Vector2f& position, sf::Vector2f& velocity)
	{

		for (size_t i = 0; i < number_of_black_holes; i++) 
		{
			if (black_holes_[i].position != position) 
			{
				const float d = wrapped_distance_squared(position, black_holes_[i].position);
				const float force = gravitational_constant / d;
				sf::Vector2f direction = direction_calculator(position, black_holes_[i].position);

				velocity += direction * force;
			}
		}
		return velocity;
	}

	static double wrapped_distance_squared(const sf::Vector2f position1, const sf::Vector2f position2)
	{
		float dx = abs(position2.x - position1.x);
		float dy = abs(position2.y - position1.y);

		if (dx > bounds.width / 2)
			dx = bounds.width - dx;
		if (dy > bounds.height / 2)
			dy = bounds.height - dy;

		return dx * dx + dy * dy;
	}


	static sf::Vector2f direction_calculator(const sf::Vector2f position1, const sf::Vector2f position2)
	{
		const float start_x = position1.x;
		const float start_y = position1.y;
		const float end_x = position2.x;
		const float end_y = position2.y;

		const float dist_x = abs(start_x - end_x);
		const float dist_y = abs(start_y - end_y);

		float result_x = 0.0f;
		if (dist_x < bounds.width / 2)
			result_x = dist_x;
		else
			result_x = dist_x - bounds.width;

		if (start_x > end_x)
			result_x *= -1;



		float result_y = 0.f;
		if (dist_y < bounds.height / 2)
			result_y = dist_y;
		else
			result_y = dist_y - bounds.height;

		if (start_y > end_y)
			result_y *= -1;

		return { result_x, result_y };
	}
};



