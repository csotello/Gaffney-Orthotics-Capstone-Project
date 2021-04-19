#ifndef GAFFNEY_ORTHOTICS_CAPSTONE_PROJECT_ENTITY_H
#define GAFFNEY_ORTHOTICS_CAPSTONE_PROJECT_ENTITY_H

#include "backend/src/engine/maths.h"
#include "backend/src/engine/texture.h"
#include "backend/src/engine/shaders.h"
#include "backend/src/engine/render.h"

#define MAX_REVERT_COUNT 50

class Entity {
public:
    Entity();
    explicit Entity(std::string file);
    ~Entity();

    void load(std::string file);
    bool is_mouse_over(vec3 o, vec3 d);
    void draw(StaticShader& shader);
    void draw_vertices(BillboardShader& shader, Mesh* billboard, Texture circle, mat4 view, vec3 campos);
    void set_position(vec3 pos);
    void set_rotation(vec3 rotate);
    void set_scale(vec3 scale);
    void scale_entity(float factor);
    void select(int xIn, int yIn, int x2, int y2, Camera camera, mat4 projection, Rect viewport);
    Model get_current() {
        return current;
    }

private:
    Model current; //current model
    Model start; //the model before any changes were made
    Model previous[MAX_REVERT_COUNT]; //an array of the last MAX_REVERT_COUNT number of changes (for undoing)
};

#endif //GAFFNEY_ORTHOTICS_CAPSTONE_PROJECT_ENTITY_H
