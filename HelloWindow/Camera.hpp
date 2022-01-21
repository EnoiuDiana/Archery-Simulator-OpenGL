#ifndef Camera_hpp
#define Camera_hpp

//#include <glm/glm.hpp>
#include "Mesh.hpp"
#include <glm/gtx/transform.hpp>
#include <GLFW\glfw3.h>
#include <string>
#include <vector>
#include "Collision.hpp"


namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN };

    class Camera
    {
    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        glm::vec3 getPosition();
        glm::vec3 getFrontDirection();
        void setTerrainVertices(std::vector<Vertex> theTerrainVertices);
        bool checkIfInsideWalls(float newPositionX, float newPositionZ);


    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        std::vector<Vertex> terrainVertices;
        bool hasTerrainVertices = false;
    };

}

#endif /* Camera_hpp */
