#include <windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>
#include <math.h>

#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files
#include "VideoMux.h"      //Functions for saving videos
#include "DebugCallback.h"
#include "Constants.hpp"
#include "BVH.h"
#include "SceneObject.h"
#include "Camera.h"

int window_width = 1024;
int window_height = 1024;
const char* const window_title = "300 Thoudsand";

static const std::string vertex_shader("template_vs.glsl");
static const std::string fragment_shader("template_fs.glsl");
GLuint shader_program = -1;

static const std::string mesh_name = "Amago0.obj";
static const std::string texture_name = "AmagoT.bmp";

// Time
float prev_time = 0.f;
float delta_time = 0.f;

// Texture
GLuint texture_id = -1;

// Mesh data
MeshData mesh_data;
vector<SceneObject> objects;  // aabb, position, velocity
BVH* bvh;
AABB* defaultAABB;
glm::mat4 M = mat4(1.f);

// Camera
Camera* camera;

// Attribute less
GLuint attribless_arena_vao = -1;

float yAngle = 0.0f;
float xAngle = 0.0f;
float mScale = 1.0f;
float aspect = 1.0f;
float fov = glm::pi<float>() / 4.0;
float camPos[3] = { 0.0, 0.5, 3.0 };
bool recording = false;
bool enableAABB = false;
bool enableBVH = false;
bool enableDynamic = false;


//This structure mirrors the uniform block declared in the shader
//struct SceneUniforms
//{
//    glm::mat4 PV;	//camera projection * view matrix
//    glm::vec4 eye_w;	//world-space eye position
//} SceneData;

struct LightUniforms
{
    glm::vec4 La = glm::vec4(0.5f, 0.5f, 0.55f, 1.0f);	//ambient light color
    glm::vec4 Ld = glm::vec4(0.5f, 0.5f, 0.25f, 1.0f);	//diffuse light color
    glm::vec4 Ls = glm::vec4(0.3f);	//specular light color
    glm::vec4 light_w = glm::vec4(1.2f, 0.2, 1.0f, 1.0f); //world-space light position

} LightData;

struct MaterialUniforms
{
    glm::vec4 ka = glm::vec4(1.0f);	//ambient material color
    glm::vec4 kd = glm::vec4(1.0f);	//diffuse material color
    glm::vec4 ks = glm::vec4(1.0f);	//specular material color
    float shininess = 20.0f;         //specular exponent
} MaterialData;

//IDs for the buffer objects holding the uniform block data
//GLuint scene_ubo = -1;
GLuint light_ubo = -1;
GLuint material_ubo = -1;
GLuint model_matrix_buffer = -1;
GLuint aabbVAOs[INSTANCE_NUM] = { -1 };
GLuint aabbVBOs[INSTANCE_NUM] = { -1 };
GLuint bvhVAOs[INSTANCE_NUM - 1] = { -1 };

namespace UboBinding
{
    //These values come from the binding value specified in the shader block layout
    //int scene = 0;
    int light = 1;
    int material = 2;
}

//Locations for the uniforms which are not in uniform blocks
namespace UniformLocs
{
    int M = 0; //model matrix
    int delta_time = 1;
    int type = 2;
}

namespace AttribLocs
{
    int model_matrix = 3; // 3 4 5 6
    int model_velocity = 7;
}

float random(float min, float max)
{
    //float randNum = (float)(rand() / (float)(RAND_MAX / abs(max - min)));
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_real_distribution<float> distribution(min, max);
    return min + distribution(generator);
}

void collisionDetection()
{
    //for (int i = 0; i < INSTANCE_NUM; i++)
    //{
    //    vector<int> collisions = bvh->CollisionDetection(objects[i].aabb, i);

    //    for (int j = 0; j < collisions.size(); j++)
    //    {
    //        int k = collisions[j];
    //        AABB aabb_1 = objects[i].aabb;
    //        AABB aabb_2 = objects[k].aabb;

    //        // difference of distance in x and z
    //        glm::vec3 deltaArea = aabb_1.intersection(aabb_2);

    //        // update position
    //        // calculate the first collision axis
    //        glm::vec3 velocity_delta = objects[i].velocity + objects[k].velocity;
    //        float delta_time_x = deltaArea.x / velocity_delta.x;
    //        float delta_time_z = deltaArea.z / velocity_delta.z;
    //        if (delta_time_x <= delta_time_z)
    //        {
    //            // x direction is the hit normal
    //            objects[i].currPos.x -= objects[i].velocity.x * delta_time;
    //            objects[k].currPos.x -= objects[k].velocity.x * delta_time;

    //            // update velocity
    //            objects[i].velocity.x = -objects[i].velocity.x;
    //            objects[k].velocity.x = -objects[k].velocity.x;
    //        }
    //        else
    //        {
    //            // z direction is the hit normal
    //            objects[i].currPos.z -= objects[i].velocity.z * delta_time;
    //            objects[k].currPos.z -= objects[k].velocity.z * delta_time;

    //            // update velocity
    //            objects[i].velocity.z = -objects[i].velocity.z;
    //            objects[k].velocity.z = -objects[k].velocity.z;
    //        }


    //        // update bounding box
    //        glm::vec3 scale = glm::vec3(mScale * mesh_data.mScaleFactor);
    //        objects[i].aabb.update(objects[i].currPos, scale);
    //        objects[k].aabb.update(objects[k].currPos, scale);
    //    }
    //}

    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        for (int j = i + 1; j < INSTANCE_NUM; j++) 
        {
            /*AABB aabb_1 = objects[i].aabb.update(objects[i].currPos);
            AABB aabb_2 = objects[j].aabb.update(objects[j].currPos);*/
            AABB aabb_1 = objects[i].aabb;
            AABB aabb_2 = objects[j].aabb;

            if (aabb_1.overlap(aabb_2))
            {
                // difference of distance in x and z
                glm::vec3 deltaArea = aabb_1.intersection(aabb_2);

                // update position
                // calculate the first collision axis
                glm::vec3 velocity_delta = objects[i].velocity + objects[j].velocity;
                float delta_time_x = deltaArea.x / velocity_delta.x;
                float delta_time_z = deltaArea.z / velocity_delta.z;
                if (delta_time_x <= delta_time_z)
                {
                    // x direction is the hit normal
                    objects[i].currPos.x -= objects[i].velocity.x * delta_time;
                    objects[j].currPos.x -= objects[j].velocity.x * delta_time;

                    // update velocity
                    objects[i].velocity.x = -objects[i].velocity.x;
                    objects[j].velocity.x = -objects[j].velocity.x;
                }
                else
                {
                    // z direction is the hit normal
                    objects[i].currPos.z -= objects[i].velocity.z * delta_time;
                    objects[j].currPos.z -= objects[j].velocity.z * delta_time;

                    // update velocity
                    objects[i].velocity.z = -objects[i].velocity.z;
                    objects[j].velocity.z = -objects[j].velocity.z;
                }


                // update bounding box
                glm::vec3 scale = glm::vec3(mScale * mesh_data.mScaleFactor);
                objects[i].aabb.update(objects[i].currPos, scale);
                objects[j].aabb.update(objects[j].currPos, scale);

            }
        }
    }


}

void updatePosition(glm::vec3& curr_pos, glm::vec3& prev_pos, glm::vec3& curr_velocity)
{
    // bounding detection
    if (curr_pos.x < -15.0 || curr_pos.x > 15.0)
    {
        if (curr_pos.x < -15.0)
            curr_pos.x = -15.0;
        else
            curr_pos.x = 15.0;

        curr_velocity.x = -curr_velocity.x;
    }

    /*if (curr_pos.y < -2.0 || curr_pos.y > 2.0)
    {
        if (curr_pos.y < -2.0)
            curr_pos.y = -2.0;
        else
            curr_pos.y = 2.0;

        curr_velocity.y = -curr_velocity.y;
    }*/

    if (curr_pos.z < -15.0 || curr_pos.z > 15.0)
    {
        if (curr_pos.z < -15.0)
            curr_pos.z = -15.0;
        else
            curr_pos.z = 15.0;

        curr_velocity.z = -curr_velocity.z;
    }

    prev_pos = glm::vec3(curr_pos.x, curr_pos.y, curr_pos.z);
    curr_pos += curr_velocity * delta_time;

}

void updatePositions()
{
    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        updatePosition(objects[i].currPos, objects[i].prevPos, objects[i].velocity);

        // update bounding box
        glm::vec3 deltaPos = objects[i].currPos - objects[i].prevPos;
        glm::vec3 scale = glm::vec3(mScale * mesh_data.mScaleFactor);
        objects[i].aabb.update(objects[i].currPos, scale);
    }
}

void updateBoundingBox()
{
    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        // update the aabb box vertex data
        //AABB aabb(mesh_data.mBbMin.x, mesh_data.mBbMin.y, mesh_data.mBbMin.z, mesh_data.mBbMax.x, mesh_data.mBbMax.y, mesh_data.mBbMax.z);
        vector<glm::vec3> vertices = BVH::generateAABBvertices(objects[i].aabb);

        // update aabb vbo
        glBindBuffer(GL_ARRAY_BUFFER, aabbVBOs[i]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 36 * sizeof(glm::vec3), vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // update bvh (!! not work because the aabb is not update)
    /*delete bvh;
    bvh = new BVH(objects);*/
}

void processSceneData()
{
    int length = ceil(sqrt(INSTANCE_NUM));

    float unit_area = (float)width_range / (float)length;  // x=y
    std::cout << length << ", " << unit_area << std::endl;

    
    for (unsigned int i = 0; i < INSTANCE_NUM; i++)
    {
        int z = i / length;
        int x = i % length;
        float x_0 = width_start_from + x * unit_area;
        float z_0 = depth_start_from + z * unit_area;

        glm::vec3 _position = glm::vec3(
            /*random(-2.0, 2.0),
            0.0,
            random(-3.0, -1.0)*/
            x_0,
            0.0,
            z_0
        );
        glm::vec3 _velocity = glm::vec3(random(-0.5, 0.2), 0.0, random(-0.5, -0.4));

        // update aabb
        glm::vec3 scale = glm::vec3(mScale * mesh_data.mScaleFactor);
        AABB aabb(mesh_data.mBbMin.x, mesh_data.mBbMin.y, mesh_data.mBbMin.z, mesh_data.mBbMax.x, mesh_data.mBbMax.y, mesh_data.mBbMax.z);
        aabb.update(_position, scale);

        SceneObject sceneObject(i, _position, glm::vec3(0, 0, 0), _velocity, aabb);
        /*std::cout << sceneObject.pos.x << ", " << sceneObject.pos.y << std::endl;
        std::cout << sceneObject.aabb.maxX << ", " << sceneObject.aabb.minX << std::endl;*/

        objects.push_back(sceneObject);
    }
}

void initBVH()
{
    bvh = new BVH(objects);
    bvh->traverseBVH(bvh->getRootIndex());
}

void initCamera()
{
    camera = new Camera(fov, aspect);
    camera->lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    camera->perspective(fov, aspect, 0.1f, 100.f);
}

void draw_gui(GLFWwindow* window)
{
    //Begin ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //Draw Gui
    ImGui::Begin("Debug window");
    if (ImGui::Button("Quit"))
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const int filename_len = 256;
    static char video_filename[filename_len] = "capture.mp4";

    ImGui::InputText("Video filename", video_filename, filename_len);
    ImGui::SameLine();
    if (recording == false)
    {
        if (ImGui::Button("Start Recording"))
        {
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            recording = true;
            start_encoding(video_filename, w, h); //Uses ffmpeg
        }

    }
    else
    {
        if (ImGui::Button("Stop Recording"))
        {
            recording = false;
            finish_encoding(); //Uses ffmpeg
        }
    }

   /* ImGui::SliderFloat("Y View angle", &yAngle, -glm::pi<float>(), +glm::pi<float>());
    ImGui::SliderFloat("X View angle", &xAngle, -glm::pi<float>(), +glm::pi<float>());*/
    ImGui::SliderFloat3("Cam Pos", camPos, -20.f, 20.f);
    ImGui::SliderFloat("Scale", &mScale, -1.0f, +1.0f);
    ImGui::Checkbox("AABB", &enableAABB);
    ImGui::Checkbox("Dynamic", &enableDynamic);
    ImGui::Checkbox("BVH", &enableBVH);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    //static bool show_test = false;
    //ImGui::ShowDemoWindow(&show_test);

    //End ImGui Frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
    //Clear the screen to the color previously specified in the glClearColor(...) call.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    M = glm::scale(glm::vec3(mScale * mesh_data.mScaleFactor));

    glUseProgram(shader_program);

    glBindTexture(0, texture_id);

    // update camera
    camera->lookAt(glm::vec3(camPos[0], camPos[1], camPos[2]), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    camera->update();

    // Set uniforms
    glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(M));
    glUniform1i(UniformLocs::type, 1);

    // Update model matrix instance attribute
    vector<glm::mat4> model_matrix_data;
    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.f), objects[i].currPos);
        model_matrix_data.push_back(trans);
    }
   
    // update object positions
    glBindBuffer(GL_ARRAY_BUFFER, model_matrix_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, INSTANCE_NUM * sizeof(glm::mat4), model_matrix_data.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(mesh_data.mVao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, (void*)0, INSTANCE_NUM);
    glBindVertexArray(0);

    // draw bounding box
    if (enableAABB)
    {
        for (int i = 0; i < INSTANCE_NUM; i++)
        {
            glBindVertexArray(aabbVAOs[i]);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniform1i(UniformLocs::type, 2);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
    }

    // draw bvh
    if (enableBVH)
    {
        bvh->drawBVH();
    }

    // draw arena plane
    M = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -0.2, 0.0)) * glm::scale(mat4(1.0), glm::vec3(20.0));
    glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(M));
    glBindVertexArray(attribless_arena_vao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform1i(UniformLocs::type, 3);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    draw_gui(window);

    if (recording == true)
    {
        glFinish();
        glReadBuffer(GL_BACK);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        read_frame_to_encode(&rgb, &pixels, w, h);
        encode_frame(rgb);
    }

    /* Swap front and back buffers */
    glfwSwapBuffers(window);
}

void idle()
{
    float curr_time = static_cast<float>(glfwGetTime());
    delta_time = curr_time - prev_time;
    prev_time = curr_time;

    //Pass time_sec value to the shaders
    glUniform1f(UniformLocs::delta_time, delta_time);

    if (enableDynamic)
    {
        // update position;
        updatePositions();

        // collision detection
        collisionDetection();

        // update bounding box
        updateBoundingBox();
    }
    
}

GLuint create_model_matrix_buffer()
{
    vector<glm::mat4> model_matrix_data;

    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.f), objects[i].currPos);
        model_matrix_data.push_back(trans);
    }

    GLuint model_matrix_buffer;
    glGenBuffers(1, &model_matrix_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, model_matrix_buffer);
    glBufferData(GL_ARRAY_BUFFER, INSTANCE_NUM * sizeof(glm::mat4), model_matrix_data.data(), GL_DYNAMIC_DRAW);

    return model_matrix_buffer;
}

void reload_shader()
{
   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());

   if (new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
   }
   else
   {
      glClearColor(0.521, 0.576, 0.596, 0.0f);

      if (shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;
   }
}

//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

   if(action == GLFW_PRESS)
   {
      switch(key)
      {
         case 'r':
         case 'R':
            reload_shader();     
         break;

         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
         break;     
      }
   }
}

//This function gets called when the mouse moves over the window.
void mouse_cursor(GLFWwindow* window, double x, double y)
{
    //std::cout << "cursor pos: " << x << ", " << y << std::endl;
}

//This function gets called when a mouse button is pressed.
void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
}

void resize(GLFWwindow* window, int width, int height)
{
    //Set viewport to cover entire framebuffer
    glViewport(0, 0, width, height);
    //Set aspect ratio used in view matrix calculation
    aspect = float(width) / float(height);
}

//Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
    glewInit();

#ifdef _DEBUG
    RegisterCallback();
#endif

    //Print out information about the OpenGL version supported by the graphics driver.	
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    glEnable(GL_DEPTH_TEST);

    reload_shader();
    mesh_data = LoadMesh(mesh_name);
    processSceneData();
    initBVH();
    initCamera();

    // create attribute less vao for arena plane
    glGenVertexArrays(1, &attribless_arena_vao);

    // load texture
    texture_id = LoadTexture(texture_name);

    // create model matrix attribute
    model_matrix_buffer = create_model_matrix_buffer();

    // init aabb box
    glGenVertexArrays(INSTANCE_NUM, aabbVAOs);
    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        glBindVertexArray(aabbVAOs[i]);
        aabbVBOs[i] = BVH::createAABBVbo(objects[i].aabb);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindVertexArray(0);
    }  


    //glBindBuffer(GL_ARRAY_BUFFER, model_matrix_buffer);
    //// bounding model matrix to shader
    //for (int i = 0; i < 4; i++)
    //{
    //    glVertexAttribPointer(AttribLocs::model_matrix + i,
    //        4, GL_FLOAT, GL_FALSE,
    //        sizeof(glm::mat4),
    //        (void*)(sizeof(glm::vec4) * i));
    //    glEnableVertexAttribArray(AttribLocs::model_matrix + i);
    //    glVertexAttribDivisor(AttribLocs::model_matrix + i, 1);
    //}
    glBindVertexArray(0);

    // Create instanced vertex attributes
    glBindVertexArray(mesh_data.mVao);

    glBindBuffer(GL_ARRAY_BUFFER, model_matrix_buffer);
    // bounding model matrix to shader
    for (int i = 0; i < 4; i++)
    {
        glVertexAttribPointer(AttribLocs::model_matrix + i,
            4, GL_FLOAT, GL_FALSE,
            sizeof(glm::mat4),
            (void*)(sizeof(glm::vec4) * i));
        glEnableVertexAttribArray(AttribLocs::model_matrix + i);
        glVertexAttribDivisor(AttribLocs::model_matrix + i, 1);
    }

    glBindVertexArray(0);

    // bind aabbvbo to shader;

    //Create and initialize uniform buffers
    //glGenBuffers(1, &scene_ubo);
    //glBindBuffer(GL_UNIFORM_BUFFER, scene_ubo);
    //glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUniforms), nullptr, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
    //glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::scene, scene_ubo); //Associate this uniform buffer with the uniform block in the shader that has the same binding.

    glGenBuffers(1, &light_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUniforms), &LightData, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
    glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::light, light_ubo); //Associate this uniform buffer with the uniform block in the shader that has the same binding.

    glGenBuffers(1, &material_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, material_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialUniforms), &MaterialData, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
    glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::material, material_ubo); //Associate this uniform buffer with the uniform block in the shader that has the same binding.

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}



//C++ programs start executing in the main() function.
int main(int argc, char **argv)
{
   GLFWwindow* window;

   /* Initialize the library */
   if (!glfwInit())
   {
      return -1;
   }

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      return -1;
   }

   //Register callback functions with glfw. 
   glfwSetKeyCallback(window, keyboard);
   glfwSetCursorPosCallback(window, mouse_cursor);
   glfwSetMouseButtonCallback(window, mouse_button);
   glfwSetFramebufferSizeCallback(window, resize);

   /* Make the window's context current */
   glfwMakeContextCurrent(window);

   initOpenGL();
   
   //Init ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 150");

   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {
      idle();
      display(window);

      /* Poll for and process events */
      glfwPollEvents();
   }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

   glfwTerminate();
   return 0;
}