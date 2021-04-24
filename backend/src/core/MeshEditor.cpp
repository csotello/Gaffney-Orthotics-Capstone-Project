#include <iostream>
#include <fstream>
#include "MeshEditor.h"
#include "StairsString.h"
#include "assimp/Exporter.hpp"

MeshEditor::MeshEditor() {
    export_strlen = 0;
    shader.load();
    bshader.load();
    camera = {0};

    entities.emplace_back();
    entities.back().load(staircaseobjhardcoded, 0);
    entities.back().set_position( {4, 4, 4} );
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

void MeshEditor::set_camera(float zoom, float x, float y, float z, float yaw, float pitch, float roll){
    camera = {x, y, z, pitch, yaw, roll};
}

void MeshEditor::add_model(const char* str, int fileformat) {
    entities.emplace_back();
    entities.back().load(str, fileformat);
    printf("added model\n");
}

// Returns char* to either a valid .obj/.stl string or null
// Sets this->export_strlen to length of that string (not including null terminator)
//      which can be retrieved via get_export_strlen()
// Accepted fileformat parameters: ".obj", ".stl"
char* MeshEditor::export_model(const char* fileformat) {
    if (!strcmp(fileformat, ".obj")) {
        std::string modelData;
        //get the current model
        for (Entity& e : entities) {
            Model curr = e.get_current();
            for (Mesh& m : curr.meshes) {
                int j = 0;
                //loop through the model and stringify, starting with the vertices
                for (Vertex& v : m.vertices) {
                    float verticesX = m.vertices[j].position.x;
                    float verticesY = m.vertices[j].position.y;
                    float verticesZ = m.vertices[j].position.z;
                    std::string line = "v " + std::to_string(verticesX) + " " + std::to_string(verticesY) + " " + std::to_string(verticesZ) + "\n";
                    modelData.append(line);
                    j++;
                }

                j = 0;
                // Texture Coordinates
                for (Vertex& v : m.vertices) {
                    float texturesX = m.vertices[j].uv.x;
                    float texturesY = m.vertices[j].uv.y;
                    std::string line = "vt " + std::to_string(texturesX) + " " + std::to_string(texturesY) + "\n";
                    modelData.append(line);
                    j++;
                }
                j = 0;

                // Vertex Normals
                for (Vertex v : m.vertices) {
                    float normalsX = m.vertices[j].normal.x;
                    float normalsY = m.vertices[j].normal.y;
                    float normalsZ = m.vertices[j].normal.z;
                    std::string line = "vn " + std::to_string(normalsX) + " " + std::to_string(normalsY) + " " + std::to_string(normalsZ) + "\n";
                    modelData.append(line);
                    j++;
                }

                // Faces
                for (j = 0; j < m.indices.size(); j += 3) {
                    int face1 = m.indices[j] + 1;
                    int face2 = m.indices[j + 1] + 1;
                    int face3 = m.indices[j + 2] + 1;
                    std::string line = "f " + std::to_string(face1) + " " + std::to_string(face2) + " " + std::to_string(face3) + "\n";
                    modelData.append(line);
                }
            }
        }

        // Return the string containing the vertices and faces of the model
        char *temp = new char[modelData.size()];
        strcpy(temp, modelData.c_str());

        // Set export_strlen
        export_strlen = modelData.size();
        return temp; // yes memory leak, implement fix if you can find one
    } else {
        return nullptr;
    }
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

// Getter function to return the size (in bytes) of the export string
//      will return 0 if export_model has not been called yet.
//      Seems dumb, but necessary because we have to pass to front-end in a weird way.
//      Talk to Jacob if you feel like you have a better solution and I'll hear you out.
uint32_t MeshEditor::get_export_strlen() const {
    return export_strlen;
}

MeshEditor::~MeshEditor() {
    shader.dispose();
    bshader.dispose();
}