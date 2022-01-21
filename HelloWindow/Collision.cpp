#include "Collision.hpp"

bool Collision::checkIfPointInsideRectangle(glm::vec2 A, glm::vec2 B, glm::vec2 D, glm::vec2 M) {
    //point M is inside a rectangle if (0 < AM.AB < AB.AB) && (0 < AM.AD < AD.AD)
    glm::vec2 AM = M - A;
    glm::vec2 AB = B - A;
    glm::vec2 AD = D - A;
    if (0 < glm::dot(AM, AB) && glm::dot(AM, AB) < glm::dot(AB, AB) && 0 < glm::dot(AM, AD) && glm::dot(AM, AD) < glm::dot(AD, AD)) {
        return true;
    }
    else {
        return false;
    }
}
Collision::Collision()
{
}

Collision::~Collision()
{
}