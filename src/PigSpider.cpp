#include "PigSpider.h"
#include "Constants.h"


using namespace std;
using namespace glm;



PigSpider::PigSpider()
{
}


PigSpider::~PigSpider()
{
}

void PigSpider::initialize(shared_ptr<Shape> sphere, shared_ptr<Shape> pig_head)
{
    this->sphere = sphere;
    this->pig_head = pig_head;
}

void PigSpider::draw(shared_ptr<Program> prog, shared_ptr<MatrixStack> M)
{
    M->pushMatrix();
    M->translate(location);
    M->scale(size);
    
    drawPart(prog, M, bodyPosition, bodyScale); // body
    
    M->pushMatrix(); // head
    M->translate(headPosition);
    drawEyes(prog, M);
    //drawPart(prog, M, vec3(headRadius, headHeight, headRadius));
    drawHead(prog, M, vec3(headRadius, headHeight, headRadius), vec3(.25, .25, .25));
    M->popMatrix();
    
    drawLegs(prog, M);
    drawArms(prog, M);
    M->popMatrix();
}

void PigSpider::drawEyes(shared_ptr<Program> prog, shared_ptr<MatrixStack> M)
{
    M->pushMatrix();
    M->translate(headToEyePosition);
    drawPart(prog, M, vec3(-.19, -.1, 0), eyeSize);
    drawPart(prog, M, vec3(.19, -.1, 0), eyeSize); // top row

    M->popMatrix();
}

void PigSpider::drawLegs(shared_ptr<Program> prog, shared_ptr<MatrixStack> M)
{
    float adjustment = M_PI_4 / 2;
    float delta = -M_PI_4 / 2;
    vec3 rotations;
    M->pushMatrix();
    for (int i = 0; i < 3; ++i) {
        rotations = defaultLegRotation(i);
        drawLeg(prog, M, rotations, vec3(-1, -.4, 0)); // left leg
        drawLeg(prog, M, vec3(rotations.x, -rotations.y, -rotations.z), vec3(1, -.4, 0)); // right leg
    }
    M->popMatrix();
}

void PigSpider::drawArms(shared_ptr<Program> prog, shared_ptr<MatrixStack> M)
{
    float adjustment = M_PI_4 / 2;
    float delta = -M_PI_4 / 2;
    vec3 rotations;
    M->pushMatrix();
    for (int i = 0; i < 1; ++i) {
        rotations = defaultLegRotation(i);
        //drawArm(prog, M, rotations, vec3(-.7, .2, 0)); // left leg
        //drawArm(prog, M, vec3(rotations.x, -rotations.y, -rotations.z), vec3(.7, .2, 0)); // right leg
        drawLeg(prog, M, rotations, vec3(-1, .2, 0)); // left leg
        drawLeg(prog, M, vec3(rotations.x, -rotations.y, -rotations.z), vec3(1, .2, 0)); // right leg
    }
    M->popMatrix();
}

void PigSpider::drawLeg(shared_ptr<Program> prog, shared_ptr<MatrixStack> M,
                     vec3 rotations, vec3 translate)
{
    M->pushMatrix();
    M->rotate(rotations.y, YAXIS);
    M->rotate(rotations.x, XAXIS);
    M->rotate(rotations.z, ZAXIS);
    M->translate(translate);
    M->pushMatrix();
    M->rotate(M_PI_2, YAXIS);
    M->rotate(M_PI_2, XAXIS);
    drawPart(prog, M, vec3(0, translate.x, 1), sphereToLegScale);
    M->popMatrix();
    M->rotate(legBendAngle, YAXIS);
    drawPart(prog, M, sphereToLegScale);
    M->popMatrix();
}

void PigSpider::drawArm(shared_ptr<Program> prog, shared_ptr<MatrixStack> M,
                        vec3 rotations, vec3 translate)
{
    M->pushMatrix();
    M->rotate(rotations.y, YAXIS);
    M->rotate(rotations.x, XAXIS);
    M->rotate(rotations.z, ZAXIS);
    M->translate(translate);
    M->pushMatrix();
    M->rotate(M_PI_2, YAXIS);
    M->rotate(M_PI_2, XAXIS);
    //drawPart(prog, M, vec3(0, translate.x, 1), vec3(0.1, 0.1, .5));
    drawPart(prog, M, vec3(0, translate.x, 1), sphereToLegScale);
    M->popMatrix();
    M->rotate(legBendAngle, YAXIS);
    //drawPart(prog, M, vec3(0.1, 0.1, .5));
    drawPart(prog, M, sphereToLegScale);
    M->popMatrix();
}

void PigSpider::drawPart(shared_ptr<Program> prog, shared_ptr<MatrixStack> M, vec3 translation, vec3 scale)
{
    M->pushMatrix();
    M->translate(translation);
    drawPart(prog, M, scale);
    M->popMatrix();
}

void PigSpider::drawPart(shared_ptr<Program> prog, shared_ptr<MatrixStack> M, vec3 scale)
{
    M->pushMatrix();
    M->scale(scale);
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
    sphere->draw(prog);
    M->popMatrix();
}

void PigSpider::drawHead(shared_ptr<Program> prog, shared_ptr<MatrixStack> M, vec3 translation, vec3 scale)
{
    M->pushMatrix();
    M->rotate(-1.57, vec3(1, 0, 0));
    M->rotate(3.14, vec3(0, 0, 1));
    M->translate(vec3(0, .2, .3));
    M->pushMatrix();
    M->scale(vec3(.05, .05, .05));
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
    pig_head->draw(prog);
    M->popMatrix();
    M->popMatrix();
}

/*
 * Returns the default angles to rotate the legs in relation to each other
 */
vec3 PigSpider::defaultLegRotation(int legNum)
{
    float adjustment = M_PI / 4;
    float delta = -M_PI / 8;
    float leg = (legNum % 2 - 0.5) * 2;
    float timeAdjust = time + delta * legNum;
    float distanceAdjust = 4.0;
    
    // default rotation formula
    float xRot = 0;
    float yRot = (1.0 / 8.0 - adjustment * legNum) * leg / distanceAdjust;
    float zRot = -1.0 / distanceAdjust;
    return vec3(xRot, yRot, zRot);
}

/*
 * Returns the default angles to rotate the legs in relation to each other and time
 * Mainly just an example of moving the legs (Replace the use of defaultLegRotation to see)
 */
vec3 PigSpider::defaultLegAnimation(int legNum)
{
    float adjustment = M_PI / 8;
    float delta = -M_PI / 8;
    float leg = (legNum % 2 - 0.5) * 2;
    float timeAdjust = time + delta * legNum;
    float distanceAdjust = 4.0;
    
    // default rotation formula
    float xRot = 0;
    float yRot = (sin(timeAdjust) / 8 - adjustment * legNum) * leg / distanceAdjust;
    float zRot = -abs(cos(timeAdjust)) / distanceAdjust;
    return vec3(xRot, yRot, zRot);
}
