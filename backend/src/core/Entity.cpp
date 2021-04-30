#include "Entity.h"

Entity::Entity() {
    current.scale = current.rotate = current.pos = {0};
    current.materials.clear();
    current.meshes.clear();
    start = current;
}

Entity::Entity(std::string file) {
    //current = load_model_string(file);
    //start = load_model_string(file);
}

void Entity::load(std::string file) {
    current = load_model_string(file);
    //start = current;
}

void Entity::draw(StaticShader& shader) {
    mat4 transform = create_transformation_matrix( current.pos, current.rotate, current.scale );
    shader.set_transform(transform);

    draw_model(&current);
}

void Entity::draw_vertices(BillboardShader& shader, Mesh* billboard, Texture circle, mat4 view, vec3 campos) {
    mat4 transform = create_transformation_matrix( current.pos, current.rotate, current.scale );

    //glDisable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    bind_texture(circle, 0);
    for(Mesh& mesh : current.meshes) {
        int i = 0;
        for(Vertex& vertex : mesh.vertices) {
            vec3 pos = (transform * V4(vertex.position.x, vertex.position.y, vertex.position.z, 1.0)).xyz;

            //move the circle a little towards the camera so it's not stuck in the mesh and you can see it clearly
            vec3 dir = normalize(campos - pos);
            pos = pos + (0.05*dir);

            //shader.set_transform(create_transformation_matrix(pos.x, pos.y, pos.z, 0, 0, 0, 1, 1,1 ));
            shader.set_transform(billboard_transform(pos.x, pos.y, pos.z, {0.10, 0.10, 0.10}, view));

            if(mesh.selected[i]) {
                shader.set_tint({1.0, 0.5, 0.2, 1.0});
            } else {
                shader.set_tint({0, 0, 0,1.0});
            }

            draw_billboard_unordered(billboard);
            i++;
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Entity::draw_vertices(PickingShader& shader, Mesh* billboard, Texture circle, mat4 view, vec3 campos) {
    mat4 transform = create_transformation_matrix( current.pos, current.rotate, current.scale );

    //glDisable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    bind_texture(circle, 0);
    u32 j = 0;
    for(Mesh& mesh : current.meshes) {
        u32 i = 0;
        for(Vertex& vertex : mesh.vertices) {
            vec3 pos = (transform * V4(vertex.position.x, vertex.position.y, vertex.position.z, 1.0)).xyz;

            //move the circle a little towards the camera so it's not stuck in the mesh and you can see it clearly
            vec3 dir = normalize(campos - pos);
            pos = pos + (0.05*dir);

            //shader.set_transform(create_transformation_matrix(pos.x, pos.y, pos.z, 0, 0, 0, 1, 1,1 ));
            shader.set_transform(billboard_transform(pos.x, pos.y, pos.z, {0.10, 0.10, 0.10}, view));
            shader.set_pickID(u32_to_rgba(j));

            draw_billboard_unordered(billboard);
            i++;
            j++;
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void Entity::set_vertex_ID_selected(int ID) {
    u32 j = 0;
    for(Mesh& mesh : current.meshes) {
        u32 i = 0;
        for (Vertex &vertex : mesh.vertices) {
            if(j == ID) {
                mesh.selected[i] = true;
            }
            i++;
            j++;
        }
    }
}

// Scale every vertex in every mesh in the entity by the factor passed in
void Entity::scale_entity(float factor) {
    for (Mesh& mesh : current.meshes) {
        for (Vertex& vertex : mesh.vertices){
            // Scale each vertex's position by the factor
            vertex.position.x *= factor;
            vertex.position.y *= factor;
            vertex.position.z *= factor;
        }
        // Update the VBO buffer to reflect changes in vertices
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.vertices.size(), &mesh.vertices[0], GL_STATIC_DRAW);
    }
}

bool Entity::is_mouse_over(vec3 o, vec3 d) {
    mat4 transform = create_transformation_matrix( current.pos, current.rotate, current.scale );

    for(Mesh m : current.meshes) {
        for(int i = 0; i < m.indices.size(); i+=3) {
            int index1 = m.indices[i+0];
            int index2 = m.indices[i+1];
            int index3 = m.indices[i+2];
            Vertex one = m.vertices[index1];
            Vertex two = m.vertices[index2];
            Vertex three = m.vertices[index3];

            vec3 collisionPoint;

            if(ray_tri_collision(
                    o, d,
                    (transform * V4(one.position.x, one.position.y, one.position.z, 1.0)).xyz,
                    (transform * V4(two.position.x, two.position.y, two.position.z, 1.0)).xyz,
                    (transform * V4(three.position.x, three.position.y, three.position.z, 1.0)).xyz,
                    &collisionPoint
            )
                    ) {
                return true;
            }
        }
    }

    return false;
}

void Entity::reset_selected_vertices() {
    //Reset selected
    for (Mesh& m : current.meshes) {
        for(int i = 0; i < m.selected.size(); ++i) {
            m.selected[i] = false;
        }
    }
}

void Entity::select(int xIn, int yIn, int x2, int y2, Camera camera, mat4 projection, Rect viewport) {
    mat4 transform = create_transformation_matrix( current.pos, current.rotate, current.scale );
    mat4 view = create_view_matrix(camera);

    reset_selected_vertices();

    for(Mesh& m : current.meshes) {
        int i = 0;
        for (Vertex& v : m.vertices) {
            //get world position of vertex sprite
            vec3 pos = (transform * V4(v.position.x, v.position.y, v.position.z, 1.0)).xyz;

            //get world position of the triangles that make up the 2d sprite showing where the vertices are
            mat4 billboardTransform = billboard_transform(pos.x, pos.y, pos.z, {0.10, 0.10, 0.10}, view);
            vec4 vertexOne = billboardTransform * V4(-0.5, -0.5, 0.0f, 1.0f);
            vec4 vertexTwo = billboardTransform * V4(0.5, -0.5, 0.0f, 1.0f);
            vec4 vertexThree = billboardTransform * V4(-0.5, 0.5, 0.0f, 1.0f);
            vec4 vertexFour = billboardTransform * V4(0.5, 0.5, 0.0f, 1.0f);

            //transform them onto the screen
            vec4 v1Next = vertexOne * view * projection;
            vec4 v2Next = vertexTwo * view * projection;
            vec4 v3Next = vertexThree * view * projection;
            vec4 v4Next = vertexFour * view * projection;

            //perspective division to account for f.o.v.
            vec3 v1Final = V3(v1Next.x / v1Next.w, v1Next.y / v1Next.w, v1Next.z / v1Next.w);
            vec3 v2Final = V3(v2Next.x / v2Next.w, v2Next.y / v2Next.w, v2Next.z / v2Next.w);
            vec3 v3Final = V3(v3Next.x / v3Next.w, v3Next.y / v3Next.w, v3Next.z / v3Next.w);
            vec3 v4Final = V3(v4Next.x / v4Next.w, v4Next.y / v4Next.w, v4Next.z / v4Next.w);

            //Now we're in normalized device space, which is -1.0f to 1.0f, convert to screen coordinates (0 to width), (0 to height)
            vec2 v1Screen{};
            v1Screen.x = (v1Final.x + 1.0f) * viewport.width / 2.0f;
            v1Screen.y = (v1Final.y + 1.0f) * viewport.height / 2.0f;

            vec2 v2Screen{};
            v2Screen.x = (v2Final.x + 1.0f) * viewport.width / 2.0f;
            v2Screen.y = (v2Final.y + 1.0f) * viewport.height / 2.0f;

            vec2 v3Screen{};
            v3Screen.x = (v3Final.x + 1.0f) * viewport.width / 2.0f;
            v3Screen.y = (v3Final.y + 1.0f) * viewport.height / 2.0f;

            vec2 v4Screen{};
            v4Screen.x = (v4Final.x + 1.0f) * viewport.width / 2.0f;
            v4Screen.y = (v4Final.y + 1.0f) * viewport.height / 2.0f;

            //Invert y coordinate
            v1Screen.y = viewport.height-v1Screen.y;
            v2Screen.y = viewport.height-v2Screen.y;
            v3Screen.y = viewport.height-v3Screen.y;
            v4Screen.y = viewport.height-v4Screen.y;

            if(i > 10 && i <= 11) {
                printf("vertexOne: %f %f\n", v1Screen.x, v1Screen.y);
                printf("vertexTwo: %f %f\n", v2Screen.x, v2Screen.y);
                printf("vertexThree: %f %f\n", v3Screen.x, v3Screen.y);
                printf("vertexFour: %f %f\n", v4Screen.x, v4Screen.y);
                m.selected[i] = true;
            }

            //now they are on screen, so check if they are in the rectangle
            if(v1Screen.x > xIn && v1Screen.y > yIn && v1Screen.x <= x2 && v1Screen.y <= y2) {
                //printf("vertexOne is inside the rectangle\n");
                m.selected[i] = true;
            }
            if(v2Screen.x > xIn && v2Screen.y > yIn && v2Screen.x <= x2 && v2Screen.y <= y2) {
                //printf("vertexTwo is inside the rectangle\n");
                m.selected[i] = true;
            }
            if(v3Screen.x > xIn && v3Screen.y > yIn && v3Screen.x <= x2 && v3Screen.y <= y2) {
                //printf("vertexThree is inside the rectangle\n");
                m.selected[i] = true;
            }
            if(v4Screen.x > xIn && v4Screen.y > yIn && v4Screen.x <= x2 && v4Screen.y <= y2) {
                //printf("vertexFour is inside the rectangle\n");
                m.selected[i] = true;
            }

            //the sprite is on a rectangle, made up of two triangles, check if the mouse selection box is over those triangles at any point.
           // bool firstTri = ray_tri_collision(o, d, vertexOne.xyz, vertexTwo.xyz, vertexThree.xyz, NULL);
            //bool secondTri = ray_tri_collision(o, d, vertexTwo.xyz, vertexThree.xyz, vertexFour.xyz, NULL);

            //if (firstTri || secondTri) {
            //    m.selected[i] = true;
            //}

            i++;
        }
    }

#if 0
    //raycast for each pixel in the selection box and check if vertices are selected.
    for(int x = xIn; x < x2; x++) {
        for(int y = yIn; y < y2; y++) {
            //raycast through this pixel
            vec3 o = {camera.x, camera.y, camera.z};
            vec3 d = raycast(projection, camera, {(float)x, (float)y}, viewport);

            for (Mesh& m : current.meshes) {
                int i = 0;
                for (Vertex& v : m.vertices) {
                    //get world position of vertex sprite
                    vec3 pos = (transform * V4(v.position.x, v.position.y, v.position.z, 1.0)).xyz;

                    //get world position of the triangles that make up the 2d sprite showing where the vertices are
                    mat4 billboardTransform = billboard_transform(pos.x, pos.y, pos.z, {0.10, 0.10, 0.10}, view);
                    vec4 vertexOne = billboardTransform * V4(-0.5, -0.5, 0.0f, 1.0f);
                    vec4 vertexTwo = billboardTransform * V4(0.5, -0.5, 0.0f, 1.0f);
                    vec4 vertexThree = billboardTransform * V4(-0.5, 0.5, 0.0f, 1.0f);
                    vec4 vertexFour = billboardTransform * V4(0.5, 0.5, 0.0f, 1.0f);

                    //the sprite is on a rectangle, made up of two triangles, check if the mouse selection box is over those triangles at any point.
                    bool firstTri = ray_tri_collision(o, d, vertexOne.xyz, vertexTwo.xyz, vertexThree.xyz, NULL);
                    bool secondTri = ray_tri_collision(o, d, vertexTwo.xyz, vertexThree.xyz, vertexFour.xyz, NULL);

                    if (firstTri || secondTri) {
                        m.selected[i] = true;
                    }

                    i++;
                }
            }
        }
    }
#endif
}

void Entity::set_position(vec3 pos) {
    current.pos = pos;
}

void Entity::set_rotation(vec3 rotate) {
    current.rotate = rotate;
}

void Entity::set_scale(vec3 scale) {
    current.scale = scale;
}

Entity::~Entity() {
    //dispose_model(&current);
    //dispose_model(&start);
}