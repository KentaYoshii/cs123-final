#include "camera.h"
#include <iostream>

/**
 * @brief Initializes the camera from the scene json camera data
 * @param cd SceneCameraData that we are reading from
 */
void Camera::initializeCamera(SceneCameraData &cd, Settings &s) {
  // Set the basic camera params

  // Screen-related
  m_width = s.screenWidth;
  m_height = s.screenHeight;
  m_aspectRatio = m_width / static_cast<float>(m_height);

  // Frustum
  m_near = s.nearPlane;
  m_far = s.farPlane;

  // View Angle
  m_heightAngle = cd.heightAngle;
  m_viewAngleHeight = 2 * m_far * glm::tan(m_heightAngle / 2);
  m_viewAngleWidth = m_aspectRatio * m_viewAngleHeight;

  // Virtual Camera spatial info
  m_look = cd.look;
  m_pos = cd.pos;
  m_up = cd.up;

  // Set the view matrix
  setViewMatrix();
};

/**
 * @brief Compute the view matrix of the camera given "pos", "look", and "up"
 * vector. Additionally, set the world space position of the camera
 * @param pos Position of the virtual camera
 * @param look Look vector of the virtual camera
 * @param up  Up vector of the virtual camera
 */
void Camera::setViewMatrix() {
  // Translation Matrix
  glm::vec4 c1(1.f, 0, 0, 0);
  glm::vec4 c2(0, 1.f, 0, 0);
  glm::vec4 c3(0, 0, 1.f, 0);
  glm::vec4 c4(-m_pos[0], -m_pos[1], -m_pos[2], 1);
  glm::mat4 translationMat(c1, c2, c3, c4);

  // Find UVW
  glm::vec3 w = -glm::normalize(m_look);
  glm::vec3 v = glm::normalize(m_up - (glm::dot(m_up, w) * w));
  glm::vec3 u = glm::cross(v, w);

  // Construct Rotation Matrix
  glm::vec4 rc1(u[0], v[0], w[0], 0);
  glm::vec4 rc2(u[1], v[1], w[1], 0);
  glm::vec4 rc3(u[2], v[2], w[2], 0);
  glm::vec4 rc4(0, 0, 0, 1.f);
  glm::mat4 rotationMat(rc1, rc2, rc3, rc4);

  // Compute the View and Inverse View Matrices
  m_view = rotationMat * translationMat;
  m_invView = glm::inverse(m_view);
}

/**
 * @brief Gets the View Matrix of this camera
 * @returns glm::mat4 representing camera view matrix
 */
glm::mat4 Camera::getViewMatrix() const { return m_view; }

/**
 * @brief Gets the Position of this camera in the world space
 * @returns glm::vec4 representing camera position
 */
glm::vec4 Camera::getCameraPosition() const { return glm::vec4(m_pos, 1); }

/**
 * @brief Gets the near plane of this camera frustum
 * @returns float representing camera near plane
 */
float Camera::getNearPlane() const { return m_near; }

/**
 * @brief Gets the far plane of this camera frustum
 * @returns float representing camera far plane
 */
float Camera::getFarPlane() const { return m_far; }

/**
 * @brief Convert the pixel (i, j) to normalized image space coordinates
 * @return std::tuple of converted coords
 */
std::tuple<float, float> Camera::normalizePixel(int i, int j) const {
  float x = ((j + 0.5) / m_width) - 0.5;
  float y = ((m_height - 1 - i + 0.5) / m_height) - 0.5;
  return std::tuple<float, float>(x, y);
}

/**
 * @brief Given a output image coordinate (i, j) compute the direction vector d
 * in world space
 * @param i x coordinate
 * @param j y coordinate
 */
glm::vec4 Camera::getRayDir(int i, int j) const {
  auto normCoords = normalizePixel(i, j);
  // (Ux, Vy, -k) in Camera Space
  glm::vec3 uvk(m_viewAngleWidth * std::get<0>(normCoords),
                m_viewAngleHeight * std::get<1>(normCoords), -m_far);
  // Convert to World Space
  return m_invView * glm::vec4(uvk, 0);
}

/**
 * @brief Given a vector representing the displacement from the current position
 * move the camera by that displacement and update the view matrix to reflect
 * the change
 * @param disp Amount to move
 */
void Camera::applyTranslation(glm::vec3 &disp) {
  // apply the displacement
  m_pos += disp;
  // update the view matrix
  setViewMatrix();
}

/**
 * @brief Handle the W key (move along the look vector)
 * @returns unit displacement with sensitivity applied
 */
glm::vec3 Camera::onWPressed() { return 0.75f * m_look; }

/**
 * @brief Handle the S key (move along the neg look vector)
 * @returns unit displacement with sensitivity applied
 */
glm::vec3 Camera::onSPressed() { return -0.75f * m_look; }

/**
 * @brief Handle the A key (move to the left)
 * @returns unit displacement with sensitivity applied
 */
glm::vec3 Camera::onAPressed() { return -0.75f * glm::cross(m_look, m_up); }

/**
 * @brief Handle the D key (move to the right)
 * @returns unit displacement with sensitivity applied
 */
glm::vec3 Camera::onDPressed() { return 0.75f * glm::cross(m_look, m_up); }

/**
 * @brief Handle the Space key (move along <0, 1, 0> in world space)
 * @returns unit displacement with sensitivity applied
 */
glm::vec3 Camera::onSpacePressed() { return glm::vec3(0, 1.f, 0); }

/**
 * @brief Handle the Control key (move along <0, -1, 0> in world space)
 * @returns unit displacement with sensitivity applied
 */
glm::vec3 Camera::onControlPressed() { return glm::vec3(0, -1.f, 0); }

/**
 * @brief Applies the rotation matrix to our look vector after which
 * we update the view matrix to reflect the change
 * @param rotationMat Rotation Matrix that you want to apply
 */
void Camera::applyRotation(glm::mat3 &rotationMat) {
  // apply the rotation matrix
  m_look = rotationMat * m_look;
  // update the view matrix
  setViewMatrix();
}

/**
 * @brief Handle the Mouse X movement which rotates the camera about
 * the axis defined by world space vector (0, 1, 0) by "angle".
 * We do a clock wise rotation
 * @param angle Amout which we want to rotate
 */
void Camera::rotateX(float deltaX) {
  float angle = 0.3 * 360.f * deltaX / m_width;
  float theta = glm::radians(angle);
  glm::mat3 rotationMat(cos(theta), 0, -1 * sin(theta), // r1
                        0, 1.f, 0,                      // r2
                        sin(theta), 0, cos(theta));     // r3
  applyRotation(rotationMat);
}

/**
 * @brief Handle the Mouse Y movement which rotates the camera about the
 * axis defined by world space vector that is perpendicular to look and up
 * vector of the camera
 */
void Camera::rotateY(float deltaY) {
  float angle = 0.3 * 360.f * deltaY / m_height;
  float theta = glm::radians(angle);
  glm::vec3 axis = glm::cross(m_look, m_up);
  // Rodrigue's Formula
  float sinTheta = glm::sin(theta);
  float cosTheta = glm::cos(theta);
  glm::mat3 K(0, -axis[2], axis[1],  // r1
              axis[2], 0, -axis[0],  // r2
              -axis[1], axis[0], 0); // r3
  glm::mat3 I(1, 0, 0, 0, 1, 0, 0, 0, 1);
  glm::mat3 rotationMat = I + (sinTheta * K) + (1 - cosTheta) * (K * K);
  applyRotation(rotationMat);
}
