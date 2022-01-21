#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget); //cameraDirection
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {

        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 Camera::getPosition() {

        return this->cameraPosition;
    }

    glm::vec3 Camera::getFrontDirection() {

        return this->cameraFrontDirection;
    }

    bool Camera::checkIfInsideWalls(float newPositionX, float newPositionZ) {
        Collision myCollisionDetection = Collision();
        glm::vec2 M = glm::vec2(newPositionX, newPositionZ);
        glm::vec2 B = glm::vec2(-1.49f, 4.35f);
        glm::vec2 A = glm::vec2(-1.41f, 4.46f);
        glm::vec2 D = glm::vec2(-2.16f, 5.07f);

        bool isInWalls = myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M);

        if (isInWalls == true) {
            return isInWalls;
        }

        B = glm::vec2(-2.20f, 5.04f);
        A = glm::vec2(-2.15f, 5.00f);
        D = glm::vec2(-3.06f, 3.96f);

        isInWalls = myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M);

        if (isInWalls == true) {
            return isInWalls;
        }

        B = glm::vec2(-3.01f, 4.01f);
        A = glm::vec2(-3.07f, 3.92f);
        D = glm::vec2(-2.27f, 3.34f);

        isInWalls = myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M);

        if (isInWalls == true) {
            return isInWalls;
        }


        B = glm::vec2(-2.34f, 3.39f);
        A = glm::vec2(-2.26f, 3.32f);
        D = glm::vec2(-1.57f, 4.22f);

        isInWalls = myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M);

        if (isInWalls == true) {
            return isInWalls;
        }

        B = glm::vec2(-1.67f, 4.18f);
        A = glm::vec2(-1.64f, 4.22f);
        D = glm::vec2(-1.96f, 4.46f);

        isInWalls = myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M); 

        if (isInWalls == true) {
            return isInWalls;
        }

        B = glm::vec2(-2.19f, 4.59f);
        A = glm::vec2(-2.16f, 4.63f);
        D = glm::vec2(-2.33f, 4.77f);

        isInWalls = myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M);

        return isInWalls;
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 move_direction;
        glm::vec3 newPosition;
        //TODO
        switch (direction) {
        case MOVE_FORWARD:
            move_direction = glm::vec3(cameraFrontDirection.x, 0.0f, cameraFrontDirection.z);
            newPosition = cameraPosition + move_direction * speed;
            if (!checkIfInsideWalls(newPosition.x, newPosition.z)) {
                cameraPosition = newPosition; 
            }
            break;

        case MOVE_BACKWARD:
            move_direction = glm::vec3(cameraFrontDirection.x, 0.0f, cameraFrontDirection.z);
            newPosition = cameraPosition - move_direction * speed;
            if (!checkIfInsideWalls(newPosition.x, newPosition.z)) {
                cameraPosition = newPosition;
            }
            break;

        case MOVE_RIGHT:
            cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
            move_direction = glm::vec3(cameraRightDirection.x, 0.0f, cameraRightDirection.z);
            newPosition = cameraPosition + move_direction * speed;
            if (!checkIfInsideWalls(newPosition.x, newPosition.z)) {
                cameraPosition = newPosition;
            }
            break;

        case MOVE_LEFT:
            cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
            move_direction = glm::vec3(cameraRightDirection.x, 0.0f, cameraRightDirection.z);
            newPosition = cameraPosition - move_direction * speed;
            if (!checkIfInsideWalls(newPosition.x, newPosition.z)) {
                cameraPosition = newPosition;
            }
            break;

        case MOVE_UP:
            cameraPosition = glm::vec3(cameraPosition.x, cameraPosition.y + speed, cameraPosition.z);
            break;

        case MOVE_DOWN:
            cameraPosition = glm::vec3(cameraPosition.x, cameraPosition.y - speed, cameraPosition.z);
            break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
    }

    void Camera::setTerrainVertices(std::vector<Vertex> theTerrainVertices) {
        terrainVertices = theTerrainVertices;
        hasTerrainVertices = true;
    }

}