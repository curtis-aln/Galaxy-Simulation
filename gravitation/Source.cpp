/*
	a gravity simulation where tonnes of particles circle the mouse position
*/

#include <iostream>
#include <SFML/Graphics.hpp>
#include <thread>

#include <ctime>
#include <array>
#include <future>
#include <string>

#include <sstream>
#include "process.h"

const int blackHoleCount = 6;
const int particleCount = 100000;
const int batchSizes = particleCount/2;

const int defcol = 35;

const int screen_width = 1340;
const int screen_height = 700;

int borderWidth = 1340 / scale.x;
int borderHeight = 700 / scale.y;

float fa1 = 10000;
int fa2 = 200;
float cosmicSpeedLimit = 900;

bool paused = false;
bool draw = true;

sf::Vector2f scale(0.01, 0.01);


int randint(int start, int end) {
	return rand() % (end-start) + start;
}

float randfloat(float start, float end) {
	return (float(rand()) / float((RAND_MAX)) * (end - start)) + start;
}

void updatePoint(sf::Vertex &point, sf::Vector2f &velocity, sf::VertexArray* blackHoles) {
	gravitate(fa1, blackHoles, point, velocity, borderWidth, borderHeight);
	speed_limit(cosmicSpeedLimit, velocity);
	border(borderWidth, borderHeight, point.position, velocity);

	// updating the star's position
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
	sf::VertexArray* blackHoles, std::vector<sf::Vector2f>* blackHole_velocities) {

	sf::Clock clock;

	sf::Transform transform;
	transform.scale(scale);
	
	while (window.isOpen()) {

		// event handeler
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Space)
					paused = not paused;

				else if (event.key.code == sf::Keyboard::D)
					draw = not draw;

				else if (event.key.code == sf::Keyboard::Q) {
					fa1 += 1;
					std::cout << fa1 << "\n";
				}

				else if (event.key.code == sf::Keyboard::W) {
					fa1 -= 1;
					std::cout << fa1 << "\n";
				}

				else if (event.key.code == sf::Keyboard::E) {
					cosmicSpeedLimit += 4;
					std::cout << cosmicSpeedLimit << "\n";
				}

				else if (event.key.code == sf::Keyboard::R) {
					cosmicSpeedLimit -= 4;
					std::cout << cosmicSpeedLimit << "\n";
				}
			}
		}

		// clearing the window
		window.clear();


		// updating particles
		int prev_i = 0;
		std::vector<std::thread> threads;
		threads.reserve((int)(particleCount / batchSizes));

		for (size_t i = batchSizes; i <= particleCount; i += batchSizes) {
			threads.emplace_back(std::thread(updateBatchOfPoints, prev_i, i, stars, star_velocities, blackHoles));
			prev_i = i;
		}

		for (auto& th : threads) {
			th.join();
		}

	

		for (size_t i = 0; i < blackHoleCount; i++) {
			BHgravitate(fa2, blackHoles, (*blackHoles)[i].position, (*blackHole_velocities)[i], borderWidth, borderHeight);
			speed_limit(3, (*blackHole_velocities)[i]);
				
			border(borderWidth, borderHeight, (*blackHoles)[i].position, (*blackHole_velocities)[i]);
			//(*blackHole_velocities)[i] += sf::Vector2f(randfloat(-0.001f, 0.001f), randfloat(-0.001f, 0.001f));
			(*blackHoles)[i].position += (*blackHole_velocities)[i];		
		}

		if (draw == true) {
			window.draw(*stars, (sf::BlendAdd, transform));
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

void initilise(sf::RenderWindow& window) {
	// creating the stars array
	sf::VertexArray stars = sf::VertexArray(sf::Points, particleCount);
	std::vector<sf::Vector2f> star_velocities(particleCount);

	// creating the blackHoles array
	sf::VertexArray blackHoles(sf::Points, blackHoleCount);
	std::vector<sf::Vector2f> blackHole_velocities(blackHoleCount);

	// initilising the stars
	for (size_t i = 0; i < star_velocities.size(); i++) {
		stars[i].position = sf::Vector2f(randfloat(0, borderWidth), randfloat(0, borderHeight));
		stars[i].color = sf::Color(defcol, defcol, defcol);
		star_velocities[i] = sf::Vector2f(randfloat(-1, 1), randfloat(-1, 1));
	}


	// initilising the black holes
	for (size_t i = 0; i < blackHole_velocities.size(); i++) {
		blackHoles[i].position = sf::Vector2f(randfloat(0, borderWidth), randfloat(0, borderHeight));
		blackHoles[i].color = sf::Color(255, 20, 255);
		blackHole_velocities[i] = sf::Vector2f(randfloat(-0.1, 0.1), randfloat(-0.1, 0.1));
	}


	// running the simulation
	run_sim(window, &stars, &star_velocities, &blackHoles, &blackHole_velocities);
}



int main() {
	// initilising random
	std::srand(static_cast<unsigned>(time(NULL)));

	// initilising screen variables
	sf::RenderWindow window(sf::VideoMode(screen_width, screen_height), "Galaxy simulation");

	// FPS setting
	window.setFramerateLimit(1580);

	initilise(window);

	return 0;
}


