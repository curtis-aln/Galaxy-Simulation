/*
	a gravity simulation where tonnes of particles circle the mouse position
*/

// TODO improvements
// [] - use custom random
// [] - OOP
// [] - Youtube video with 3 million particles and 4 black holes

#include <iostream>
#include <SFML/Graphics.hpp>
#include <thread>

#include <ctime>
#include <future>
#include <string>

#include <sstream>
#include "process.h"

// SFML settings
constexpr unsigned int screen_width = 1920;
constexpr unsigned int screen_height = 1080;

const sf::Vector2f simulation_scale = { 0.01f, 0.01f };

const auto border_width  = static_cast<float>(screen_width / simulation_scale.x);
const auto border_height = static_cast<float>(screen_height / simulation_scale.y);
const sf::FloatRect bounds = { 0, 0, border_width, border_height };


// Simulation Settings
constexpr unsigned number_of_black_holes = 3u;
constexpr unsigned number_of_stars = 600'000u;

constexpr float cosmic_speed_limit = 190000.f;


// Multi-threading settings
constexpr unsigned threads = 8u;
constexpr unsigned threading_batches = number_of_stars / threads;


// Graphical Settings
constexpr unsigned star_color = 30;



float fa1 = 200000;
constexpr unsigned int fa2 = 20;

bool paused = false;
bool draw = true;


int randint(int start, int end) {
	return rand() % (end-start) + start;
}

float randfloat(float start, float end) {
	return (float(rand()) / float((RAND_MAX)) * (end - start)) + start;
}

void updatePoint(sf::Vertex &point, sf::Vector2f &velocity, sf::VertexArray* blackHoles)
{
	gravitate(fa1, blackHoles, point, velocity, border_width, border_height);
	speed_limit(cosmic_speed_limit, velocity);
	border(bounds, point.position);

	point.position += velocity;
}


void updateBatchOfPoints(int startI, int endI, sf::VertexArray* stars,
	std::vector<sf::Vector2f>* star_velocities, sf::VertexArray* blackHoles) {

	// first we need to make sure the endI isn't greater than the amount of stars
	if (endI > (*stars).getVertexCount())
		return;


	for (size_t i = startI; i < endI; i+=4) {
		updatePoint((*stars)[i], (*star_velocities)[i], blackHoles);
		updatePoint((*stars)[i + 1], (*star_velocities)[i + 1], blackHoles);
		updatePoint((*stars)[i + 2], (*star_velocities)[i + 2], blackHoles);
		updatePoint((*stars)[i + 3], (*star_velocities)[i + 3], blackHoles);
	}

}


void run_sim(sf::RenderWindow& window,
	sf::VertexArray* stars, std::vector<sf::Vector2f>* star_velocities,
	sf::VertexArray* blackHoles, std::vector<sf::Vector2f>* blackHole_velocities)
{

	sf::Clock clock;

	sf::RenderStates states;
	sf::Transform transform;
	transform.scale(simulation_scale);

	states.transform = transform;
	states.blendMode = sf::BlendAdd;

	window.setFramerateLimit(99999);
	window.setVerticalSyncEnabled(false);
	window.resetGLStates();

	sf::Clock fps_display_clock = sf::Clock::Clock();
	
	while (window.isOpen()) 
	{

		// event handeler
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

				else if (event.key.code == sf::Keyboard::Q) 
				{
					fa1 += 1;
					std::cout << fa1 << "\n";
				}

				else if (event.key.code == sf::Keyboard::W) 
				{
					fa1 -= 1;
					std::cout << fa1 << "\n";
				}
			}
		}

		// clearing the window
		window.clear();


		// updating particles
		int prev_i = 0;
		std::vector<std::thread> threads;
		threads.reserve((int)(number_of_stars / threading_batches));

		for (size_t i = threading_batches; i <= number_of_stars; i += threading_batches) 
		{
			threads.emplace_back(updateBatchOfPoints, prev_i, i, stars, star_velocities, blackHoles);
			prev_i = i;
		}

		for (auto& th : threads) 
		{
			th.join();
		}

	

		for (size_t i = 0; i < number_of_black_holes; i++) 
		{
			BHgravitate(fa2, blackHoles, (*blackHoles)[i].position, (*blackHole_velocities)[i], border_width, border_height);
			speed_limit(3, (*blackHole_velocities)[i]);
				
			border(bounds, (*blackHoles)[i].position);
			//(*blackHole_velocities)[i] += sf::Vector2f(randfloat(-0.001f, 0.001f), randfloat(-0.001f, 0.001f));
			(*blackHoles)[i].position += (*blackHole_velocities)[i];		
		}

		if (draw == true) {
			window.draw(*stars, states);
			window.draw(*blackHoles, transform);
		}
		

		// FPS management
		float millisecondsPerFrame = clock.restart().asMilliseconds();

		std::ostringstream oss;
		oss << "Galaxy Simulation" << millisecondsPerFrame << " millisecondsPerFrame";
		std::string var = oss.str();
		window.setTitle(var);

		// updating screen
		window.display();
	}
}

void initilise(sf::RenderWindow& window)
{
	// creating the stars array
	sf::VertexArray stars = sf::VertexArray(sf::Points, number_of_stars);
	std::vector<sf::Vector2f> star_velocities(number_of_stars);

	// creating the blackHoles array
	sf::VertexArray blackHoles(sf::Points, number_of_black_holes);
	std::vector<sf::Vector2f> blackHole_velocities(number_of_black_holes);

	// initilising the stars
	for (size_t i = 0; i < star_velocities.size(); i++) {
		stars[i].position = sf::Vector2f(randfloat(0, border_width), randfloat(0, border_height));
		stars[i].color = sf::Color(star_color, star_color, star_color);
		star_velocities[i] = sf::Vector2f(randfloat(-1, 1), randfloat(-1, 1));
	}


	// initilising the black holes
	for (size_t i = 0; i < blackHole_velocities.size(); i++) {
		blackHoles[i].position = sf::Vector2f(randfloat(0, border_width), randfloat(0, border_height));
		blackHoles[i].color = sf::Color(255, 20, 255);
		blackHole_velocities[i] = sf::Vector2f(randfloat(-0.1, 0.1), randfloat(-0.1, 0.1));
	}


	// running the simulation
	run_sim(window, &stars, &star_velocities, &blackHoles, &blackHole_velocities);
}



int main() {
	// initilising random
	std::srand(static_cast<unsigned>(time(nullptr)));

	// initilising screen variables
	sf::RenderWindow window(sf::VideoMode(screen_width, screen_height), "Galaxy simulation");

	// FPS setting
	window.setFramerateLimit(1580);

	initilise(window);

	return 0;
}


