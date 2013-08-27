/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:    
 *
 *        Version:  1.0
 *        Created:  17/08/13 18:38:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Vojtech Havlicek (Vh), vojta.havlicek@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include<boost/chrono.hpp>
#include<boost/thread.hpp>

#include<GL/glew.h>
#include<GL/glut.h>

#include<SOIL/SOIL.h>

#include<ctime>
#include<cmath>

#include<iostream>
#include<fstream>
#include<sstream>

using namespace std;

boost::chrono::steady_clock::time_point timer;

void update();
void tick();
void render();
void prepareProgram();
void prepareShaders();
void prepareTextures();

string loadSource(const char* path);

double accumulator;
float t;
float dt; // Maximum permited value for iteration step
int height;
int width;
int window_handle;
int counter;

GLuint program;
GLuint fragment;
GLuint vertex;
GLuint texture_A; // contains the initial data
GLuint texture_B;
GLuint to_render;
GLuint fb_A;      // first framebuffer
GLuint fb_B;      // second framebuffer
GLuint tex_uniform;
GLuint pixel_uniform;
GLuint dx_uniform;
GLuint dy_uniform;

int main(int argc, char *argv[])
{
    timer = boost::chrono::steady_clock::now();

    width  = 400;
    height = 400;

    t = 0;
    dt = 1.0f/60.0f;
    
    glutInit(&argc, argv);
    glutInitWindowPosition(200,200);
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    window_handle = glutCreateWindow("Conway's Game of life on GPU");
    glutDisplayFunc(update);
    glutIdleFunc(update);
    
    glEnable(GL_TEXTURE_2D);

    glewInit();
   
    prepareProgram();
    prepareTextures();    
    prepareShaders();

    int ret_val;
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &ret_val);
    cout << "compiled fragment? " << (ret_val == GL_TRUE) << '\n';
    
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &ret_val);
    cout << "compiled vertex? " << (ret_val == GL_TRUE) << '\n';
    
    glGenFramebuffers(1, &fb_A);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_A);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_A, 0);

    glGenFramebuffers(1, &fb_B); 
    glBindFramebuffer(GL_FRAMEBUFFER, fb_B);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_B, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glLinkProgram(program);
    glUseProgram(program);

    to_render = texture_A;
    tex_uniform = glGetUniformLocation(program, "tex");
    pixel_uniform = glGetUniformLocation(program, "pixel");

    glUniform2f(pixel_uniform, 1.0f/width, 1.0f/height);

    glutMainLoop();

    return 0;
}

void tick()
{
     glUseProgram(program); 
     glActiveTexture(GL_TEXTURE0);
                                                         
     glViewport(0,0,width,height);
     
     if(counter % 2 == 0)
     {
       glBindFramebuffer(GL_FRAMEBUFFER, fb_A);
       glBindTexture(GL_TEXTURE_2D, texture_B);
       to_render = texture_A;
     } 
     else
     {
       glBindFramebuffer(GL_FRAMEBUFFER, fb_B);
       glBindTexture(GL_TEXTURE_2D, texture_A);    
       to_render = texture_B;
     }
                                                         
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                                                         
     glViewport(0,0,width,height);
     glBegin(GL_QUADS);
         glTexCoord2f(0.0f,0.0f);
         glVertex2f(-1.0f,-1.0f);
                               
         glTexCoord2f(1.0f,0.0f);
         glVertex2f( 1.0f,-1.0f);
                               
         glTexCoord2f(1.0f ,1.0f);
         glVertex2f( 1.0f, 1.0f);
                               
         glTexCoord2f( 0.0f,1.0f);
         glVertex2f( -1.0f, 1.0f);
     glEnd();
                                                         
     glUseProgram(0); 
                                                         
     glBindFramebuffer(GL_FRAMEBUFFER, 0);

    counter ++;
}

void update()
{

    boost::chrono::steady_clock::time_point current = boost::chrono::steady_clock::now();
    double diff = (boost::chrono::duration<double> (current-timer)).count();
    
    if(diff > 0.1)
        diff = 0.1; // Avoid spiral of death ?

    timer = current;

    accumulator += diff;

    while(accumulator >= dt)
    {
        accumulator -= dt;
        t += dt;
        tick();
    }
    render();
}

void render()
{
    glBindTexture(GL_TEXTURE_2D, to_render);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0,0,width,height);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);
        glVertex3f(-1.0f,-1.0f,0.0f);
                              
        glTexCoord2f(1.0f,0.0f);
        glVertex3f( 1.0f,-1.0f,0);
                              
        glTexCoord2f(1.0f ,1.0f);
        glVertex3f( 1.0f, 1.0f ,0.0f);
                              
        glTexCoord2f( 0.0f,1.0f);
        glVertex3f( -1.0f, 1.0f,0.0f);
    glEnd();

    glutSwapBuffers();
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
}

void prepareShaders()
{
    string v_source = loadSource("vertex.vert");
    string f_source = loadSource("fragment.frag");

    const char* cv_source = v_source.c_str();
    const char* cf_source = f_source.c_str();

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment,1,&cf_source,NULL);
    glCompileShader(fragment);

    vertex   = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &cv_source, NULL);
    glCompileShader(vertex);

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
}

void prepareProgram()
{
    program = glCreateProgram();
}

void prepareTextures()
{
    texture_A = SOIL_load_OGL_texture("blank.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
    
    glBindTexture(GL_TEXTURE_2D, texture_A); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    texture_B = SOIL_load_OGL_texture("blank.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

    glBindTexture(GL_TEXTURE_2D, texture_B); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

string loadSource(const char* path)
{
    string line;
    ostringstream output;
    ifstream source(path);

    if (source.is_open()) 
    {    
        output << source.rdbuf();
        source.close();
    }
    
    return output.str();
}
