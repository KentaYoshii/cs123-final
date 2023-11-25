#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "raymarch/raymarchscene.h"
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include <unordered_map>

class Realtime : public QOpenGLWidget {
public:
  Realtime(QWidget *parent = nullptr);
  void finish(); // Called on program exit
  void sceneChanged();
  void settingsChanged();
  void saveViewportImage(std::string filePath);

public slots:
  void tick(QTimerEvent *event); // Called once per tick of m_timer

protected:
  void initializeGL() override; // Called once at the start of the program
  void paintGL() override; // Called whenever the OpenGL context changes or by
                           // an update() request
  void resizeGL(int width,
                int height) override; // Called when window size changes

private:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

  // Tick Related Variables
  int m_timer; // Stores timer which attempts to run ~60 times per second
  QElapsedTimer m_elapsedTimer; // Stores timer which keeps track of actual time
                                // between frames
  float m_delta;

  // Input Related Variables
  bool m_mouseDown = false;   // Stores state of left mouse button
  glm::vec2 m_prev_mouse_pos; // Stores mouse position
  std::unordered_map<Qt::Key, bool>
      m_keyMap; // Stores whether keys are pressed or not

  // Device Correction Variables
  int m_devicePixelRatio;

  // ============ RAY MARCHER ==============

  // PRIVATE DATA

  // RayMarch scene
  RayMarchScene scene;

  // Shader
  // - raymarch shader
  GLuint m_rayMarchShader;

  // Textures
  // - default material texture
  GLuint m_defaultShapeTexture;

  // FBO
  // - application window FBO
  GLuint m_defaultFBO = 1;

  // Image Plane (aka [-1,1] blank canvas)
  GLuint m_imagePlaneVAO;
  GLuint m_imagePlaneVBO;

  // Toggelable Options
  // - gamma correction
  bool m_enableGammaCorrection;
  // - soft shadow
  bool m_enableSoftShadow;
  // - reflection
  bool m_enableReflection;
  // - refraction
  bool m_enableRefraction;
  // - ambient occulusion
  bool m_enableAmbientOcclusion;

  // PRIVATE METHODS

  // Performs raymarching using our raymarch shader
  void rayMarch();

  // Initializes the shaders with constant uniforms
  void initShader();
  // Initializes all the default variables used in shader
  void initDefaults();
  // Initializes [-1,1] blank canvas to be used for raymarching
  void initImagePlane();
  // Initializes each and every material texture used in the scene
  void initShapesTextures();

  // Sets the output FBO
  void setFBO(GLuint fbo);

  // Sets the uniforms for our screen-related stuff
  void configureScreenUniforms(GLuint shader);
  // Sets the uniforms for camera-related stuff
  void configureCameraUniforms(GLuint shader);
  // Sets the uniforms for each shape in the scene
  void configureShapesUniforms(GLuint shader);
  // Sets the uniforms for each light in the scene
  void configureLightsUniforms(GLuint shader);
  // Sets the uniforms for all the rendering options
  void configureSettingsUniforms(GLuint shader);
};
