#include "MeshEditor.h"
#include "StairsString.h"
#include "assimp/Exporter.hpp"

MeshEditor::MeshEditor() {
    shader.load();
    bshader.load();
    camera = {0};
    entities.emplace_back();
    entities.back().load(staircaseobjhardcoded);
    entities.back().set_position( {4, 4, 4} );
    //entities.emplace_back();
    //entities.back().load(staircaseobjhardcoded);
    //entities.back().set_position( {-3, -3, -3} );
    //entities.emplace_back();
    //entities.back().load(staircaseobjhardcoded);
    //entities.back().set_position( {-5, -5, -5} );
    //entities.emplace_back();
    //entities.back().load(staircaseobjhardcoded);
    //entities.back().set_position( {3, -3, -5} );
    //entities.emplace_back();
    //entities.back().load(staircaseobjhardcoded);
    //entities.back().set_position( {-3, -3, 3} );
    projection = perspective_projection(90, 16.0f / 9.0f, 0.01f, 3000.0f);
    move_cam_backwards(&camera, 10);

    billboard = create_billboard();

    //Create a circle texture to be used to show where vertices are on the mesh
    auto* pixels = new unsigned char[64*64*4];
    vec2 center = {32.0f, 32.0f};
    for(int x = 0; x < 64; ++x) {
        for(int y = 0; y < 64; ++y) {
            vec2 pos = {(float)x, (float)y};
            int index = (x+y*64)*4;

            vec2 diff = center - pos;
            f32 distance = length(diff);

            if(distance < 32) {
                pixels[index+0] = 255;
                pixels[index+1] = 255;
                pixels[index+2] = 255;
                pixels[index+3] = 255;
            } else {
                pixels[index+0] = 0;
                pixels[index+1] = 0;
                pixels[index+2] = 0;
                pixels[index+3] = 0;
            }
        }
    }
    circle = load_texture(pixels, 64, 64, GL_LINEAR);
    delete[] pixels;
}

void MeshEditor::run() {
    local float rotation = 0.0f;
    rotation+=0.2f;
    local float zoom = 5.0f;
    //zoom-=0.025f;

#if 0
    Rect viewport = {0, 0, 1000, 640};
    vec2 mouse;
    int x;
    int y;
    glfwGetMousePos(&x, &y);
    mouse.x = x;
    mouse.y = y;

    vec3 rayposition = {camera.x, camera.y, camera.z};
    vec3 raydirection = raycast(projection, camera, mouse, viewport);

    for(int i = 0; i < entities.size(); ++i) {
        if(entities[i].is_mouse_over(rayposition, raydirection)) {
            printf("mouse is over a mesh\n");
        } else {
            printf("mouse is not over a mesh\n");
        }
    }
#endif

    mat4 view = create_view_matrix(camera);

    shader.bind();
    shader.set_light_pos(14, 14, 14);
    shader.set_light_color(147.0f/255.0f, 108.0f/255.0f,95.0f/255.0f);
    shader.set_camera_pos(camera.x, camera.y, camera.z);
    shader.set_view(view);

    static int timer=0;
    timer++;

    for(Entity& e : entities) {
        e.draw(shader);
        e.set_rotation( {rotation, rotation, rotation} );
    }
    bshader.bind();
    bshader.set_view(view);
    for(Entity& e : entities) {
        e.draw_vertices(bshader, &billboard, circle, view, {camera.x, camera.y, camera.z});
    }
}

void MeshEditor::set_camera(float zoom, float x, float y, float z, float yaw, float pitch, float roll) {
    camera = {x, y, z, pitch, yaw, roll};
}

void MeshEditor::add_model(const char* str) {
    entities.emplace_back();
    entities.back().load(str);
    printf("added model\n");
}

char* MeshEditor::export_model(int ID, const char* fileformat) {
    Assimp::Exporter exporter;
    const aiScene *scene;
    aiMesh mesh;
    mesh.mNormals;
    mesh.mVertices;
    mesh.mFaces[0].mIndices;
    scene->mMeshes[0] = &mesh;
    const aiExportDataBlob* blob = exporter.ExportToBlob(scene, "obj", 0);
    return (char*)blob->data;
}

void MeshEditor::on_mouse_up(int x, int y, int x2, int y2) {
    for(Entity& e: entities) {
        e.select(x, y, x2, y2, camera, projection, {0, 0, 1000, 640});
    }
}

// Scale every vertex in every mesh in every entity by the factor passed in
void MeshEditor::scale_all_entities(float factor) {
    for(Entity& e: entities) {
        e.scale_entity(factor);
    }
}

MeshEditor::~MeshEditor() {
    shader.dispose();
    bshader.dispose();
}