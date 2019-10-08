//
//  Model.h
//
//  Created by James Asbury
//

#ifndef _MODEL_H
#define _MODEL_H

#include <iostream>
#include <glad/glad.h>

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"


class Model {
public:
    Model(const char *filename){
        load_meshes(filename);
    }
    
    virtual void draw(const std::shared_ptr<Program> shader, std::shared_ptr<MatrixStack> mStack)
    {
        for(unsigned int i = 0; i < meshes.size(); i++){
            glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(mStack->topMatrix()));
            meshes[i]->draw(shader);
        }
    }
    
protected:
    std::vector<std::shared_ptr<Shape>> meshes;
    
    void load_meshes(const char *filename){
        std::vector<tinyobj::shape_t> TOshapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string errStr;
        
        //load in the mesh and make the shape(s)
        bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, filename);
        if (!rc) {
            std::cerr << errStr << std::endl;
        } else {
            for(int i = 0; i < TOshapes.size(); ++i){
                std::shared_ptr<Shape> mesh = std::make_shared<Shape>();
                mesh->createShape(TOshapes[i]);
                mesh->measure();
                mesh->init();
                meshes.push_back(mesh);
            }
        }
    }
    
};

#endif /* _MODEL_H */
