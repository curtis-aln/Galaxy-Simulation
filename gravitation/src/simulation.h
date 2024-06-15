#pragma once

#include <SFML/Graphics.hpp>
#include <thread>

#include <string>

#include <sstream>
#include "settings.h"

#include "random.h"
#include "toroidal_space.h"
#include <iostream>

inline sf::Vector2f normalize(const sf::Vector2f vec)
{
	const float length = sqrt(vec.x * vec.x + vec.y * vec.y);
	return vec / length;
}

inline sf::Vector2f perpendicular(const sf::Vector2f vec)
{
	return { vec.y, -vec.x };
}


struct BlackHole
{
	sf::Vector2f position;
	sf::Vector2f velocity;
	sf::Vector2f acceleration;

	void update(const float dt)
	{
		velocity += acceleration * dt;
		position += velocity * dt;
		acceleration = sf::Vector2f(0, 0);
	}
};


class Simulation : SimulationSettings, SFMLSettings
{
private:
	bool paused_ = false;
	bool draw_ = true;

	unsigned frames = 0;

	sf::RenderWindow window_{};
	sf::Clock clock_{};

	sf::VertexArray stars_ = sf::VertexArray(sf::Points, number_of_stars);
	std::vector<sf::Vector2f> star_velocities_ = std::vector<sf::Vector2f>(number_of_stars);

	std::vector<BlackHole> black_holes_;
	sf::CircleShape black_hole_renderer_;

	sf::RenderStates states_{};
	sf::Transform transform_{};


public:
	Simulation()
		: window_(sf::VideoMode(screen_width, screen_height), title)
	{
		window_.setFramerateLimit(max_fps);
		window_.setVerticalSyncEnabled(v_sync);
		window_.resetGLStates();

		transform_.scale(sf::Vector2f(simulation_scale, simulation_scale));
		states_.transform = transform_;
		states_.blendMode = sf::BlendAdd;

		init_black_holes();
		init_stars();
	}


	void init_black_holes()
	{
		black_hole_renderer_.setFillColor(black_hole_color);
		black_hole_renderer_.setRadius(black_hole_radius);

		black_holes_.resize(number_of_black_holes);
		for (size_t i = 0; i < number_of_black_holes; i++)
		{
			black_holes_[i].position = Random::rand_pos_in_rect(bounds);
			black_holes_[i].velocity = Random::rand_vector(-initial_bh_velocity, initial_bh_velocity);
		}
	}


	void init_stars()
	{
		for (size_t i = 0; i < star_velocities_.size(); i++)
		{
			const sf::Vector2f parent_pos = black_holes_[i % number_of_black_holes].position;
			stars_[i].position = Random::rand_pos_in_circle<float>(parent_pos, star_spawn_radius);
			stars_[i].color = star_color;

			// The star will initially start by going in the direction perpendicular to the black hole
			const float dist = toroidal_distance(parent_pos, stars_[i].position, bounds);

			const sf::Vector2f norm = toroidal_direction(parent_pos, stars_[i].position, bounds) / dist;
			const sf::Vector2f perp = perpendicular(norm);

			const float speed = sqrt(dist);

			star_velocities_[i] = perp * speed;
		}
	}


	void run()
	{
		while (window_.isOpen())
		{
			++frames;
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

				else if (event.key.code == sf::Keyboard::Escape)
				{
					window_.close();
				}
			}
		}
	}


	void update_batch_of_stars(const unsigned begin_index, const unsigned end_index)
	{
		if (end_index > number_of_stars)
			return;

		for (size_t i = begin_index; i < end_index; ++i) 
		{
			sf::Vector2f& position = stars_[i].position;
			sf::Vector2f& vel = star_velocities_[i];

			gravitate(position, vel, star_mass);
			speed_limit(vel);
			border(position);

			position += vel * dt;
			vel *= 0.9999f;
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
		for (BlackHole& black_hole : black_holes_)
		{
			gravitate(black_hole.position, black_hole.velocity, bh_mass, G/5);
			speed_limit(black_hole.velocity, cosmic_speed_limit / 10);

			black_hole.update(dt);
			border(black_hole.position);
		}

	}

	void render()
	{
		if (draw_ == true) 
		{
			window_.clear();

			window_.draw(stars_, states_);

			for (BlackHole& black_hole : black_holes_)
			{
				black_hole_renderer_.setPosition(black_hole.position - sf::Vector2f(black_hole_radius, black_hole_radius));
				window_.draw(black_hole_renderer_, states_);
			}

			window_.display();
		}

		// FPS management
		const auto fps = static_cast< sf::Int32>(1.f / clock_.restart().asSeconds());

		std::ostringstream oss;
		oss << title << fps << " fps";
		const std::string var = oss.str();
		window_.setTitle(var);
	}


	static void speed_limit(sf::Vector2f& velocity, const float max_speed = cosmic_speed_limit)
	{
		const float speed_sq = velocity.x * velocity.x + velocity.y * velocity.y;

		if (speed_sq > max_speed * max_speed)
		{
			const float speed = sqrt(speed_sq);
			const sf::Vector2f norm_vel = velocity / speed;
			velocity = norm_vel * max_speed;
		}
	}


	static void border(sf::Vector2f& position)
	{
		if (position.x > bounds.left + bounds.width)
			position.x -= bounds.left + bounds.width;

		else if (position.x < bounds.left)
			position.x += bounds.left + bounds.width;

		if (position.y < bounds.top)
			position.y += bounds.top + bounds.height;

		else if (position.y > bounds.top + bounds.height)
			position.y -= bounds.top + bounds.height;
	}



	void gravitate(const sf::Vector2f& position, sf::Vector2f& velocity, const float mass, const float grav_const=G) const
	{
		for (size_t i = 0; i < number_of_black_holes; i++)
		{
			const sf::Vector2f bh_position = black_holes_[i].position;
			if (bh_position != position)
			{
				const float distance_sq = toroidal_distance_sq(position, bh_position, bounds);

				if (distance_sq < black_hole_radius * black_hole_radius * 2)
				{
					velocity *= 1.01f;
					continue;
				}

				const float mass_product = mass * bh_mass;
				const float force = grav_const * (mass_product / distance_sq);
				sf::Vector2f direction = toroidal_direction(position, bh_position, bounds);

				velocity += direction * force * dt;
			}
		}
	}
};