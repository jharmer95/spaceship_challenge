CXX=g++-10.0.1
CXXFLAGS=-std=c++2a -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Weffc++ -Werror

spaceship_challenge: spaceship_challenge.cpp
	$(CXX) -o $@ $< $(CXXFLAGS)

run: spaceship_challenge
	./spaceship_challenge

clean:
	rm spaceship_challenge	

