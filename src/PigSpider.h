#pragma once

#ifndef PIGSPIDER_H
#define PIGSPIDER_H

#include <iostream>
#include <glad/glad.h>
#include <time.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Model.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "stb_image.h"


using namespace std;
using namespace glm;

class PigSpider
{
public:
    PigSpider();
    ~PigSpider();
    
    shared_ptr<Shape> sphere;
    shared_ptr<Shape> pig_head;
    
    float time; // frame time for animation
    
    /* * * * * * * * * * * * * * * * * * * * * *
     *                   NOTE                  *
     *                   (pos)    (neg)        *
     *         x-axis is right and left        *
     *         y-axis is up and down           *
     *         z-axis is back and front        *
     *                                         *
     * * * * * * * * * * * * * * * * * * * * * */
    
    
    float size = 0.1;
    vec3 location = vec3(0);
    
    // ----------- body variables -------------- //
    vec3 bodyPosition = vec3(0, 0, 0);
    vec3 bodyScale = vec3(.6, 0.7, .6);
    
    // ----------- head variables -------------- //
    vec3 headPosition = vec3(0, 0, -0.2);
    float headRadius = 0.7; // radius of head from top-down view
    float headHeight = headRadius / 2; // distance from center of head to top
    
    // ----------- eye variables ------------- //
    float eyeOffset = 0.1; // offset from center horizontal line of spider head to center of eyes
    vec3 headToEyePosition = vec3(0, 1.1, -headRadius + .5);
    vec3 eyeSize = vec3(.1, .13, .1);
    
    // ----------- leg variables ------------- //
    vec3 sphereToLegScale = vec3(0.05, 0.05, 1);
    float legBendAngle = M_PI_2; // angle of leg joint
    vec3 legOrigin = vec3(-1, 0, 0); // center point where left legs come out of
    
    // ----------- arm variables ------------- //
    vec3 armOrigin = vec3(-1, 1, 0);
    
    // ----------- Functions -------------- //
    void initialize(shared_ptr<Shape> sphere, shared_ptr<Shape> pig_head);
    void draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> M);
    void drawHead(shared_ptr<Program> prog, shared_ptr<MatrixStack> M, vec3 translation, vec3 scale);
    void drawEyes(shared_ptr<Program> prog, shared_ptr<MatrixStack> M);
    void drawMouth(shared_ptr<Program> prog, shared_ptr<MatrixStack> M);
    void drawLegs(shared_ptr<Program> prog, shared_ptr<MatrixStack> M);
    void drawArms(shared_ptr<Program> prog, shared_ptr<MatrixStack> M);
    void drawLeg(shared_ptr<Program> prog, shared_ptr<MatrixStack> M,
                 vec3 rotations, vec3 translate);
    void drawArm(shared_ptr<Program> prog, shared_ptr<MatrixStack> M,
                 vec3 rotations, vec3 translate);
    void drawPart(shared_ptr<Program> prog, shared_ptr<MatrixStack> M, vec3 translation, vec3 scale);
    void drawPart(shared_ptr<Program> prog, shared_ptr<MatrixStack> M, vec3 scale);
    
    vec3 defaultLegRotation(int legNum);
    vec3 defaultLegAnimation(int legNum);
};

#endif
