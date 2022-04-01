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

#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files
#include "VideoMux.h"      //Functions for saving videos
#include "DebugCallback.h"
#include "Constants.hpp"
#include "BVH.h"
#include "SceneObject.h"

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
//glm::vec3 object_1_pos;
//glm::vec3 object_2_pos;
//glm::vec3 object_3_pos;
//glm::vec3 object_1_velocity;
//glm::vec3 object_2_velocity;
//glm::vec3 object_3_velocity;
vector<SceneObject> objects;  // aabb, position, velocity

float rotAngle = 0.0f;
float mScale = 1.0f;
float aspect = 1.0f;
bool recording = false;


//This structure mirrors the uniform block declared in the shader
struct SceneUniforms
{
    glm::mat4 PV;	//camera projection * view matrix
    glm::vec4 eye_w;	//world-space eye position
} SceneData;

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
GLuint scene_ubo = -1;
GLuint light_ubo = -1;
GLuint material_ubo = -1;
GLuint model_matrix_buffer = -1;

namespace UboBinding
{
    //These values come from the binding value specified in the shader block layout
    int scene = 0;
    int light = 1;
    int material = 2;
}

//Locations for the uniforms which are not in uniform blocks
namespace UniformLocs
{
    int M = 0; //model matrix
    int delta_time = 1;
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

void updatePosition(glm::vec3& curr_pos, glm::vec3& curr_velocity)
{
    // bounding detection
    if (curr_pos.x < -0.8 || curr_pos.x > 0.8)
    {
        if (curr_pos.x < -0.8)
            curr_pos.x = -0.8;
        else
            curr_pos.x = 0.8;

        curr_velocity.x = -curr_velocity.x;
    }

    if (curr_pos.y < -0.8 || curr_pos.y > 0.8)
    {
        if (curr_pos.y < -0.8)
            curr_pos.y = -0.8;
        else
            curr_pos.y = 0.8;

        curr_velocity.y = -curr_velocity.y;
    }

    if (curr_pos.z < -4.0 || curr_pos.z > -1.0)
    {
        if (curr_pos.z < -4.0)
            curr_pos.z = -4.0;
        else
            curr_pos.z = -1.0;

        curr_velocity.z = -curr_velocity.z;
    }

    curr_pos += curr_velocity * delta_time;

    //std::cout << "curr pos: " << curr_pos.x << ", " << curr_pos.y << ", " << curr_pos.z << std::endl;
}


void processSceneData()
{
    aiVector3D mBbMin = mesh_data.mBbMin;
    aiVector3D mBbMax = mesh_data.mBbMax;
    AABB aabb(mBbMin.x, mBbMin.y, mBbMin.z, mBbMax.x, mBbMax.y, mBbMax.z);


    for (unsigned int i = 0; i < INSTANCE_NUM; i++)
    {
        // here i euqals to instance id
        glm::vec3 _position = glm::vec3(random(-0.5, 0.5), random(-0.5, 0.5), random(-2.0, -1.0));
        glm::vec3 _velocity = glm::vec3(random(-0.5, 0.2), random(-0.5, 0.4), random(-0.5, -0.4));

        SceneObject sceneObject(i, _position, _velocity, aabb);
        std::cout << sceneObject.pos.x << ", " << sceneObject.pos.y << std::endl;
        std::cout << sceneObject.aabb.maxX << ", " << sceneObject.aabb.minX << std::endl;

        objects.push_back(sceneObject);
    }
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

    ImGui::SliderFloat("View angle", &rotAngle, -glm::pi<float>(), +glm::pi<float>());
    ImGui::SliderFloat("Scale", &mScale, -10.0f, +10.0f);

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

    SceneData.eye_w = glm::vec4(0.0f, 0.0f, 3.0f, 1.0f);
    glm::mat4 M = glm::rotate(rotAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(mScale * mesh_data.mScaleFactor));
    //M = glm::translate(M, glm::vec3(object_1_pos));
    glm::mat4 V = glm::lookAt(glm::vec3(SceneData.eye_w), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 P = glm::perspective(glm::pi<float>() / 4.0f, aspect, 0.1f, 100.0f);
    SceneData.PV = P * V;

    glUseProgram(shader_program);

    glBindTexture(0, texture_id);

    // Set uniforms
    glUniformMatrix4fv(UniformLocs::M, 1, false, glm::value_ptr(M));

    glBindBuffer(GL_UNIFORM_BUFFER, scene_ubo); //Bind the OpenGL UBO before we update the data.
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneData), &SceneData); //Upload the new uniform values.
    glBindBuffer(GL_UNIFORM_BUFFER, 0); //unbind the ubo

     // Update model matrix instance attribute
    vector<glm::mat4> model_matrix_data;
    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.f), objects[i].pos);
        model_matrix_data.push_back(trans);
    }
    /*std::cout << "curr pos: " << object_1_pos.x << ", " << object_1_pos.y << ", " << object_1_pos.z << std::endl;
    std::cout << "-----" << std::endl;*/

    glBindBuffer(GL_ARRAY_BUFFER, model_matrix_buffer);
    //glBufferData(GL_ARRAY_BUFFER, INSTANCE_NUM * sizeof(glm::mat4), model_matrix_data, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, INSTANCE_NUM * sizeof(glm::mat4), model_matrix_data.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(mesh_data.mVao);

    //glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);
    glDrawElementsInstanced(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, (void*)0, 3);
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

    // update position;
    updatePosition(objects[0].pos, objects[0].velocity);
    updatePosition(objects[1].pos, objects[1].velocity);
    updatePosition(objects[2].pos, objects[2].velocity);
}

GLuint create_model_matrix_buffer()
{
    vector<glm::mat4> model_matrix_data;
    /*glm::mat4 model_matrix_data[INSTANCE_NUM] = {
        glm::translate(glm::mat4(1.f), objects[0].pos),
        glm::translate(glm::mat4(1.f), objects[1].pos),
        glm::translate(glm::mat4(1.f), objects[2].pos)
    };*/

    for (int i = 0; i < INSTANCE_NUM; i++)
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.f), objects[i].pos);
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

    texture_id = LoadTexture(texture_name);

    // Create instanced vertex attributes
    glBindVertexArray(mesh_data.mVao);

    // randomize the position and velocity data
    /*object_1_pos = glm::vec3(random(-0.5, 0.5), random(-0.5, 0.5), random(-2.0, -1.0));
    object_2_pos = glm::vec3(random(-0.4, 0.5), random(-0.5, 0.5), random(-2.0, -1.0));
    object_3_pos = glm::vec3(random(-0.1, 0.5), random(-0.5, 0.5), random(-2.0, -1.0));

    object_1_velocity = glm::vec3(random(-0.5, 0.2), random(-0.5, 0.4), random(-0.5, -0.4));
    object_2_velocity = glm::vec3(random(-0.4, 0.5), random(-0.5, -0.3), random(0.4, 0.7));
    object_3_velocity = glm::vec3(random(-0.4, 0.3), random(0.3, 0.7), random(-0.5, 0.1));*/

    // create model matrix attribute
    model_matrix_buffer = create_model_matrix_buffer();
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

    //Create and initialize uniform buffers
    glGenBuffers(1, &scene_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, scene_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUniforms), nullptr, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
    glBindBufferBase(GL_UNIFORM_BUFFER, UboBinding::scene, scene_ubo); //Associate this uniform buffer with the uniform block in the shader that has the same binding.

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