#include "Camera.h"

struct SceneUniforms
{
    glm::mat4 PV;	//camera projection * view matrix
    glm::vec4 eye_w;	//world-space eye position
} SceneData;
 
int scene_loc = 0;

Camera::Camera(float fov, float aspect)
{
    this->fov = fov;
    this->aspect = aspect;

    // create scene uniform buffer for camera
    glGenBuffers(1, &scene_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, scene_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUniforms), nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, scene_loc, scene_ubo);
}

void Camera::lookAt(glm::vec3 from, glm::vec3 to, glm::vec3 up)
{
    this->camPos = from;
    this->lookAtPoint = to;
    this->lookUpDir = up;
}

void Camera::perspective(float fov, float aspect, float near, float far)
{
    this->fov = fov;
    this->aspect = aspect;
    this->nearClip = near;
    this->farClip = far;
}

// make sure to bind the shader program before update
void Camera::update()
{
    /*M = glm::rotate(yAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(mScale * mesh_data.mScaleFactor));
    M = glm::rotate(M, xAngle, glm::vec3(1.0f, 0.0f, 0.0f));*/
    viewMat = glm::lookAt(camPos, lookAtPoint, lookUpDir);
    projMat = glm::perspective(fov, aspect, nearClip, farClip);

    SceneData.eye_w = glm::vec4(camPos, 1.0);
    SceneData.PV = projMat * viewMat;

    // update the camera into the shader
    glBindBuffer(GL_UNIFORM_BUFFER, scene_ubo); 
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneData), &SceneData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}