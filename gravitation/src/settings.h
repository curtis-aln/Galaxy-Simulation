#pragma once

#include <SFML/Graphics.hpp>

struct SFMLSettings
{
	inline static constexpr unsigned int screen_width = 1920;
	inline static constexpr unsigned int screen_height = 1080;

	inline static const sf::Vector2f simulation_scale = { 0.01f, 0.01f };

	inline static const auto border_width = screen_width / simulation_scale.x;
	inline static const auto border_height = screen_height / simulation_scale.y;
	inline static sf::FloatRect bounds = { 0, 0, border_width, border_height };

	inline static const std::string title = "Galaxy Simulation";
	inline static constexpr bool v_sync = false;
	inline static constexpr unsigned max_fps = 120u;
};


struct SimulationSettings
{
	inline static constexpr unsigned number_of_black_holes = 3u;
	inline static constexpr unsigned number_of_stars = 600'000u;

	inline static constexpr float cosmic_speed_limit = 20000.f;

	inline static constexpr float dt = 0.008f;

	// Multi-threading settings
	inline static constexpr unsigned threads = 8u;
	inline static constexpr unsigned threading_batches = number_of_stars / threads;


	// Graphical Settings
	inline static constexpr unsigned star_color = 30;


	inline static float gravitational_constant = 20'000;
	inline static constexpr unsigned int fa2 = 20;



	inline static constexpr float initial_star_velocity = 50;
	inline static constexpr float initial_bh_velocity = 15;
};