.PHONY : run
run : boids
	./boids

boids : boids.cpp
	g++ boids.cpp -o boids -O3 -lSDL2