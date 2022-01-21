#ifndef Collision_hpp
#define Collision_hpp

#include <glm/glm.hpp>

class Collision
{
public:
	Collision();
	~Collision();
	bool checkIfPointInsideRectangle(glm::vec2 A, glm::vec2 B, glm::vec2 D, glm::vec2 M);

private:

};


#endif
