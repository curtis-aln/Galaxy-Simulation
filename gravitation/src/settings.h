#pragma once

#include <SFML/Graphics.hpp>

struct SFMLSettings
{
	inline static constexpr unsigned int screen_width = 1920;
	inline static constexpr unsigned int screen_height = 1080;

	inline static constexpr float simulation_scale = 0.002f;

	inline static constexpr auto border_width = screen_width / simulation_scale;
	inline static constexpr auto border_height = screen_height / simulation_scale;
	inline static sf::FloatRect bounds = { 0, 0, border_width, border_height };

	inline static const std::string title = "Galaxy Simulation";
	inline static constexpr bool v_sync = false;
	inline static constexpr unsigned max_fps = 1050u;
};


struct SimulationSettings
{
	// initialization
	inline static constexpr float initial_bh_velocity = 50;

	inline static constexpr float star_spawn_radius = 40'000.f;

	inline static constexpr unsigned number_of_black_holes = 2u;
	inline static constexpr unsigned number_of_stars = 600'000u;


	// Physics settings
	inline static float G = 20000;
	inline static constexpr float cosmic_speed_limit = 100'000.f;
	inline static constexpr float dt = 1.5f;

	inline static constexpr float star_mass = 1;
	inline static constexpr float bh_mass   = 1;


	// Multi-threading settings
	inline static constexpr unsigned threads = 8u;
	inline static constexpr unsigned threading_batches = number_of_stars / threads;


	// Graphical Settings
	inline static constexpr int sf = 10;
	inline static const sf::Color star_color = { 8 * sf, 2 * sf, 4 * sf };

	inline static const sf::Color black_hole_color = { 255, 20, 255 };
	inline static constexpr float black_hole_radius = 600.f;
};