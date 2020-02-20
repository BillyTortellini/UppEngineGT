#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__

#include "uppLib.hpp"
#include "../utils/tmpAlloc.hpp"
#include "../fileIO.hpp"
#include "../fileListener.hpp"

// Render includes
#include "openGLState.hpp"
#include "mesh.hpp"
#include "shaderProgram.hpp"
#include "autoShaderProgram.hpp"

// Next steps:
//  - Mesh creation in new file
//  - Phong shader
//  - renderer set camera
//  - Draw with mesh + transform + shaderProgram 
//  - Use renderer from inside game (Dynamically loaded)
//  - Debug rendering (2D, text, lines...)
//  - Texture creation from shader

//  DONE:
//  -----
//  - ShaderProgram set important uniforms automatically 
//  - ShaderProgram detects attributes and sets them automatically

// Todo:
// -----
// View Matrix              X
// Projection Matrix        X
// Orthographic Matrix      O
// Transform (With Quat)    O


#endif
