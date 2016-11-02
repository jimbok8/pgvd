#pragma once

#include <string>
#include <assert.h>
#include "../../GLUtilities/gl_utils.h"

using namespace std;
class ShaderProgram {
public: 
  GLuint program;

  /* attributes */
  GLuint position_id;
  GLuint offset_id;
  GLuint scale_id;
  GLuint color_id;

  /* uniforms */
  GLuint matrix_id;
  GLuint pointSize_id;
  GLuint color_uniform_id;

  ShaderProgram(const string vertShader, const string fragShader);
  void getAttributes();
  void getUniforms();
  void use();
};