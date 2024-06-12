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
	bool paused = false;
	bool draw = true;

	sf::RenderWindow window{};
	sf::Clock clock{};

	sf::VertexArray stars = sf::VertexArray(sf::Points, number_of_stars);
	std::vector<sf::Vector2f> star_velocities = std::vector<sf::Vector2f>(number_of_stars);

	sf::VertexArray blackHoles = sf::VertexArray(sf::Points, number_of_black_holes);
	std::vector<sf::Vector2f> blackHole_velocities = std::vector<sf::Vector2f>(number_of_black_holes);

	sf::RenderStates states{};
	sf::Transform transform{};


public:
	Simulation()
		: window(sf::VideoMode(screen_width, screen_height), "Galaxy simulation")
	{
		window.setFramerateLimit(99999);
		window.setVerticalSyncEnabled(false);
		window.resetGLStates();

		transform.scale(simulation_scale);
		states.transform = transform;
		states.blendMode = sf::BlendAdd;
		
		const float initial_star_velocity = 50;
		const float initial_bh_velocity = 15;

		// initializing the stars
		for (size_t i = 0; i < star_velocities.size(); i++)
		{
			stars[i].position = Random::rand_pos_in_rect(bounds);
			stars[i].color = sf::Color(star_color, star_color, star_color);
			star_velocities[i] = Random::rand_vector(-initial_star_velocity, initial_star_velocity);
		}

		// initializing the black holes
		for (size_t i = 0; i < blackHole_velocities.size(); i++) 
		{
			blackHoles[i].position = Random::rand_pos_in_rect(bounds);
			blackHoles[i].color = sf::Color(255, 20, 255);
			blackHole_velocities[i] = Random::rand_vector(-initial_bh_velocity, initial_bh_velocity);
		}
	}

	void run()
	{
		while (window.isOpen())
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
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Space)
					paused = not paused;

				else if (event.key.code == sf::Keyboard::D)
					draw = not draw;
			}
		}
	}


	void update_batch_of_stars(const unsigned begin_index, const unsigned end_index)
	{
		if (end_index > number_of_stars)
			return;

		for (size_t i = begin_index; i < end_index; ++i) 
		{
			sf::Vertex& point = stars[i];
			sf::Vector2f& vel = star_velocities[i];

			gravitate(gravitational_constant, &blackHoles, stars[i], vel, border_width, border_height);
			speed_limit(cosmic_speed_limit, vel);
			border(bounds, point.position);

			point.position += vel;
		}

	}


	void update_stars()
	{
		// updating particles
		unsigned prev_i = 0;
		std::vector<std::thread> threads;
		threads.reserve(static_cast<int>(number_of_stars / threading_batches));

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
			BHgravitate(fa2, &blackHoles, blackHoles[i].position, blackHole_velocities[i], border_width, border_height);
			speed_limit(3, blackHole_velocities[i]);

			border(bounds, blackHoles[i].position);
			//(*blackHole_velocities)[i] += sf::Vector2f(randfloat(-0.001f, 0.001f), randfloat(-0.001f, 0.001f));
			blackHoles[i].position += blackHole_velocities[i];
		}

	}

	void render()
	{
		if (draw == true) 
		{
			window.clear();

			window.draw(stars, states);
			window.draw(blackHoles, transform);

			window.display();
		}

		// FPS management
		const float millisecondsPerFrame = clock.restart().asMilliseconds();

		std::ostringstream oss;
		oss << "Galaxy Simulation" << millisecondsPerFrame << " ms/f";
		const std::string var = oss.str();
		window.setTitle(var);
	}
};



