#define _USE_MATH_DEFINES
#pragma once
#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <boost/asio.hpp>
#include "Client.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include "Window.h"
#include <time.h>
#include "Object.h"
#include "VAO.h"
#include "glslprogram.h"
#include "Cube.h"
#include "ShaderController.h"
#include "Ground.h"
#include <Qt/QtGui/QImage> 
#include <Qt/QtOpenGL/QGLWidget>
#include "SkyBox.h"
#include "Structures.h"
#include "Sphere.h"
#include "TextureScreen.h"
#include "Camera.h"
#include "Scene.h"
#include "Mesh.h"
#include "Texture.h"

#include "ConfigSettings.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <sys/types.h>
#include <sys/stat.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;
using glm::quat;
using namespace std;

std::vector<Object*> draw_list;
std::vector<Object*> player_list;
std::vector<Object*> stationary_list;
std::vector<Texture*> texture_list;

Mesh* m_pMesh;
Mesh* m_pMesh2;

int Window::width  = 800;   // set window width in pixels here
int Window::height = 600;   // set window height in pixels here
float nearClip = 0.1;
float farClip = 1000.0;
float fov = 55.0;

vec3 EyePoint = vec3(0,0,3);
vec3 CenterPoint = vec3(0,0,0);
mat4 Projection;
mat4 View = glm::lookAt(EyePoint,CenterPoint, vec3(0,1,0));
mat4 MVP;

ShaderController sdrCtl;

int oldX,oldY,mouseDown,mouseButton;

Light light[1];
Fog fog;
vec3 fogColor = vec3(0.9,0.9,0.9);

Scene* scene;

Cube* cube;
Ground* ground;
SkyBox* skybox;
Sphere* sphere;
TextureScreen* texScreen;
int texScreenWidth = 512;
int texScreenHeight = 512;

Camera* cam;
float cam_sp = 0.1;

GLuint fboHandle;

void keyboard(unsigned char key, int, int);
void keyUp (unsigned char key, int x, int y);
void trackballScale( int width, int height, int fromX, int fromY, int toX, int toY );
void trackballRotation( int width, int height, int fromX, int fromY, int toX, int toY );
void mouseFunc(int button, int state, int x, int y);
void motionFunc(int x, int y);
void passiveMotionFunc(int x, int y);

void updateShaders();
void setupShaders();
void initialize(int argc, char *argv[]);
void loadTextures();

int pID = 0;
int counter = 0;
int keyState = 0;
boost::array<mat4, 4> m;

//Initialize some vectors
std::vector <pair<string, mat4>>* stateVec = new vector<pair<string, mat4>>;
std::vector <pair<string, mat4>>* recvVec = new vector<pair<string, mat4>>;

boost::asio::io_service io_service;
tcp_client* cli;

int playerID = -1;

void Window::idleCallback(void)
{
	//print fps
	static time_t timer = clock();
	if(clock()-timer>=CLOCKS_PER_SEC){
		cout<<"FPS: "<<counter<<endl;
		timer = clock();
		counter=0;
	}
	counter++;

	cam->preRotate(glm::rotate(mat4(1.0), cam->getPendingRote(), vec3(1, 0, 0)));
	if ((cam->getCamM()*vec4(0, 1, 0, 0))[1] < 0){
		cam->setPreRot(glm::rotate(mat4(1.0), -90.0f, vec3(1, 0, 0)));
	}
	cam->setPendingRot(0);

	static time_t tick = clock();
	float diff = (float)(clock() - tick)/CLOCKS_PER_SEC;
	tick = clock();
	//scene->simulate(diff, 1.0 / 100);

	//vector<mat4> playerMs = scene->getPlayerMats();
	//for (int i = 0; i < player_list.size(); i++){
	//	player_list[i]->setModelM(playerMs[i]);
	//}

	View = cam->getViewM();
	updateShaders();

    displayCallback();  
}
void Window::reshapeCallback(int w, int h)
{
  width = w;
  height = h;
  glViewport(0, 0, w, h);  // set new view port size
  Projection = glm::perspective(fov, (float)w/h, nearClip, farClip);
  updateShaders();
}
void Window::displayCallback(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mat4 viewM = cam->getViewM();

	//m_pMesh->Render();
	sdrCtl.getShader("basic_model")->setUniform("projMatrix", Projection);
	sdrCtl.getShader("basic_model")->setUniform("viewMatrix", viewM);
	sdrCtl.getShader("basic_model")->setUniform("modelMatrix", glm::translate(vec3(0.0, 2.0, 0.0))*glm::rotate(mat4(1.0), 90.0f, vec3(-1.0, 0, 0))*glm::scale(vec3(0.1, 0.1, 0.1)));
	sdrCtl.getShader("basic_model")->setUniform("texUnit", 0);
	sdrCtl.getShader("basic_model")->use();
	m_pMesh2->Render();

	// doing stuff with each rectangle in the list:
	for (int i = 0; i < draw_list.size(); ++i)
	{
		Object* r = draw_list[i];
		//r->draw();
		string temp = r->getType();
		if ((std::string)temp == (std::string)"Cube"){
			Cube* c = (Cube*)r;
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ModelMatrix", c->getModelM());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("MVP", Projection*viewM*c->getModelM());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Kd", c->getKd());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Ka", c->getKa());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Ks", c->getKs());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Shininess", c->getShininess());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.ReflectFactor", c->getReflectFactor());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Eta", c->getEta());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("CubeMapTex", c->getCubeMapUnit());
			sdrCtl.getShader("basic_reflect_refract")->use();
			c->draw();
		}
		else if ((std::string)temp == (std::string)"Skybox"){
			SkyBox* s = (SkyBox*)r;
			sdrCtl.getShader("basic_skybox")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("basic_skybox")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("basic_skybox")->setUniform("CubeMapTex", skybox->getTexUnit());
			sdrCtl.getShader("basic_skybox")->use();
			s->draw();
		}
		else if ((std::string)temp == (std::string)"Ground"){
			Ground* g = (Ground*)r;
			sdrCtl.getShader("grid_ground")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("grid_ground")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("grid_ground")->setUniform("ModelMatrix", mat4(1.0));
			sdrCtl.getShader("grid_ground")->setUniform("MVP", Projection*viewM);
			sdrCtl.getShader("grid_ground")->setUniform("color", vec3(0.8));
			sdrCtl.getShader("grid_ground")->setUniform("width", (float)0.02);
			sdrCtl.getShader("grid_ground")->use();
			g->draw();
		}
	}

	for (int i = 0; i < player_list.size(); ++i)
	{
		Object* r = player_list[i];
		//r->draw();
		string temp = r->getType();
		if ((std::string)temp == (std::string)"Cube"){
			Cube* c = (Cube*)r;
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ModelMatrix", c->getModelM());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("MVP", Projection*viewM*c->getModelM());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Kd", c->getKd());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Ka", c->getKa());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Ks", c->getKs());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Shininess", c->getShininess());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.ReflectFactor", c->getReflectFactor());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Eta", c->getEta());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("CubeMapTex", c->getCubeMapUnit());
			sdrCtl.getShader("basic_reflect_refract")->use();
			c->draw();
		}
		else if ((std::string)temp == (std::string)"Skybox"){
			SkyBox* s = (SkyBox*)r;
			sdrCtl.getShader("basic_skybox")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("basic_skybox")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("basic_skybox")->setUniform("CubeMapTex", skybox->getTexUnit());
			sdrCtl.getShader("basic_skybox")->use();
			s->draw();
		}
		else if ((std::string)temp == (std::string)"Ground"){
			Ground* g = (Ground*)r;
			sdrCtl.getShader("grid_ground")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("grid_ground")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("grid_ground")->setUniform("ModelMatrix", mat4(1.0));
			sdrCtl.getShader("grid_ground")->setUniform("MVP", Projection*viewM);
			sdrCtl.getShader("grid_ground")->setUniform("color", vec3(0.8));
			sdrCtl.getShader("grid_ground")->setUniform("width", (float)0.02);
			sdrCtl.getShader("grid_ground")->use();
			g->draw();
		}
	}

	for (int i = 0; i < stationary_list.size(); ++i)
	{
		Object* r = stationary_list[i];
		//r->draw();
		string temp = r->getType();
		if ((std::string)temp == (std::string)"Cube"){
			Cube* c = (Cube*)r;
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("basic_reflect_refract")->setUniform("ModelMatrix", c->getModelM());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("MVP", Projection*viewM*c->getModelM());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Kd", c->getKd());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Ka", c->getKa());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Ks", c->getKs());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Shininess", c->getShininess());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.ReflectFactor", c->getReflectFactor());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("material.Eta", c->getEta());
			sdrCtl.getShader("basic_reflect_refract")->setUniform("CubeMapTex", c->getCubeMapUnit());
			sdrCtl.getShader("basic_reflect_refract")->use();
			c->draw();
		}
		else if ((std::string)temp == (std::string)"Skybox"){
			SkyBox* s = (SkyBox*)r;
			sdrCtl.getShader("basic_skybox")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("basic_skybox")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("basic_skybox")->setUniform("CubeMapTex", skybox->getTexUnit());
			sdrCtl.getShader("basic_skybox")->use();
			s->draw();
		}
		else if ((std::string)temp == (std::string)"Ground"){
			Ground* g = (Ground*)r;
			sdrCtl.getShader("grid_ground")->setUniform("ViewMatrix", viewM);
			sdrCtl.getShader("grid_ground")->setUniform("ProjectionMatrix", Projection);
			sdrCtl.getShader("grid_ground")->setUniform("ModelMatrix", mat4(1.0));
			sdrCtl.getShader("grid_ground")->setUniform("MVP", Projection*viewM);
			sdrCtl.getShader("grid_ground")->setUniform("color", vec3(0.8));
			sdrCtl.getShader("grid_ground")->setUniform("width", (float)0.02);
			sdrCtl.getShader("grid_ground")->use();
			g->draw();
		}
	}
	
	glFlush();  
	glutSwapBuffers();
}

void server_update(int value){
	//Read position vector from server
	recvVec = cli->read();
	io_service.poll();
	std::cout << "recvVec string:" << recvVec->front().first << std::endl;
	m[0] = (*recvVec)[0].second;
	m[1] = (*recvVec)[1].second;
	m[2] = (*recvVec)[2].second;
	m[3] = (*recvVec)[3].second;

	// Print out matrix contents
	/*
	cout << (m[0])[0][0] << (m[0])[0][1] << (m[0])[0][2] << (m[0])[0][3] << endl;
	cout << (m[0])[1][0] << (m[0])[1][1] << (m[0])[1][2] << (m[0])[1][3] << endl;
	cout << (m[0])[2][0] << (m[0])[2][1] << (m[0])[2][2] << (m[0])[2][3] << endl;
	cout << (m[0])[3][0] << (m[0])[3][1] << (m[0])[3][2] << (m[0])[3][3] << endl;
	*/
	player_list[0]->setModelM(m[0]);
	player_list[1]->setModelM(m[1]);
	player_list[2]->setModelM(m[2]);
	player_list[3]->setModelM(m[3]);

	//Have to reset timer after
	glutTimerFunc(15, server_update, 0);
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);      	      	      // initialize GLUT
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);   // open an OpenGL context with double buffering, RGB colors, and depth buffering
  glutInitWindowSize(Window::width, Window::height);      // set initial window size
  glutCreateWindow("CSE 125");    	      // open window and set window title
  glutFullScreen();

  glEnable(GL_DEPTH_TEST);            	      // enable depth buffering
  glClear(GL_DEPTH_BUFFER_BIT);       	      // clear depth buffer
  glClearColor(fogColor[0],fogColor[1],fogColor[2],0.0);   	      // set clear color to black
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // set polygon drawing mode to fill front and back of each polygon
  glDisable(GL_CULL_FACE);     // disable back face culling to render both sides of polygons
  glShadeModel(GL_SMOOTH);             	      // set shading to smooth
  glMatrixMode(GL_PROJECTION); 
  
  // Generate material properties:
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  
  //for white face problem when scaled down
  glEnable(GL_NORMALIZE);

  //more realistic lighting
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  // Install callback functions:
  glutDisplayFunc(Window::displayCallback);
  glutReshapeFunc(Window::reshapeCallback);
  glutIdleFunc(Window::idleCallback);

  glutMouseFunc(mouseFunc);
  glutMotionFunc(motionFunc);
  glutPassiveMotionFunc(passiveMotionFunc);

  //Timer for network
  glutTimerFunc(15, server_update, 0);

  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyUp);

  glutSetCursor(GLUT_CURSOR_NONE);

  initialize(argc, argv);

  glutMainLoop();

  delete cube;

  return 0;
}

void keyboard(unsigned char key, int, int){
	if (key == 'a'){
		keyState = keyState | 1;
	}
	if (key == 'd'){
		keyState = keyState | 1 << 1;
	}
	if (key == 'w'){
		keyState = keyState | 1 << 2;
	}
	if (key == 's'){
		keyState = keyState | 1 << 3;
	}
	if (key == 27){
		exit(0);
	}
	if (key == ' '){
		keyState = keyState | 1 << 4;
	}
	//Send key int to server as matrix with all values being keyState
	stateVec->front() = std::make_pair(std::to_string(pID), mat4((float)keyState));
	cli->write(*stateVec);
	io_service.poll();
}
void keyUp (unsigned char key, int x, int y) {  
	if (key == 'a'){
		keyState = keyState & ~1;
	}
	if (key == 'd'){
		keyState = keyState & ~(1 << 1);
	}
	if (key == 'w'){
		keyState = keyState & ~(1 << 2);
	}
	if (key == 's'){
		keyState = keyState & ~(1 << 3);
	}
	if (key == ' '){
		keyState = keyState & ~(1 << 4);
	}
	stateVec->front() = std::make_pair(std::to_string(pID), mat4((float)keyState));
	cli->write(*stateVec);
	io_service.poll();
}
void mouseFunc(int button, int state, int x, int y)
{
	oldX=x;
	oldY=y;
	mouseDown= (state==GLUT_DOWN);
	mouseButton = button;
}
void motionFunc(int x, int y)
{
	passiveMotionFunc(x, y);
}
void passiveMotionFunc(int x, int y){
	static int lastX = 0;
	static int lastY = 0;

	float dx = x - lastX;
	float dy = lastY - y;
	lastX = x;
	lastY = y;

	if (fabs(dx) < 250 && fabs(dy) < 250){
		cam->pushRot(cam_sp*dy);
		//Update camera position in vector and send
		stateVec->back() = std::make_pair(std::to_string(pID), mat4((float)dx));
		cli->write(*stateVec);
		io_service.poll();
	}

	if (abs(Window::width / 2 - lastX)>25 || abs(Window::height / 2 - lastY)>25){
		lastX = Window::width / 2;
		lastY = Window::height / 2;
		glutWarpPointer(Window::width / 2, Window::height / 2);
	}
}

void updateShaders(){

	sdrCtl.setUniform("basic_reflect_refract","light[0].type",light[0].type);
	sdrCtl.setUniform("basic_reflect_refract","light[0].pos",light[0].pos);
	sdrCtl.setUniform("basic_reflect_refract","light[0].specular",light[0].specular);
	sdrCtl.setUniform("basic_reflect_refract","light[0].diffuse",light[0].diffuse);
	sdrCtl.setUniform("basic_reflect_refract","light[0].ambient",light[0].ambient);
	sdrCtl.setUniform("basic_reflect_refract","light[0].dir",light[0].dir);
	sdrCtl.setUniform("basic_reflect_refract","light[0].spotCutOff",light[0].spotCutOff);

}
void setupShaders()
{
	sdrCtl.createVFShader("basic_skybox","Shaders/basic_skybox.vert","Shaders/basic_skybox.frag");

	sdrCtl.createVFShader("basic_reflect_refract","Shaders/basic_reflect_refract.vert","Shaders/basic_reflect_refract.frag");

	sdrCtl.createVFShader("grid_ground", "Shaders/grid_ground.vert", "Shaders/grid_ground.frag");

	sdrCtl.createVFShader("basic_model", "Shaders/basic_model.vert", "Shaders/basic_model.frag");

	updateShaders();
}
void initialize(int argc, char *argv[])
{
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		fprintf(stderr, "Error initializing GLEW: %s\n", 
			glewGetErrorString(err) );
	}

	loadTextures();

	// fbo texture
	GLuint renderTex;
	glGenTextures(1, &renderTex);
	glActiveTexture(GL_TEXTURE6); // Use texture unit 0
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texScreenWidth,texScreenHeight,0,GL_RGBA,
		GL_UNSIGNED_BYTE,NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		GL_LINEAR);

	glGenFramebuffers(1,&fboHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, renderTex, 0);

	GLuint depthBuf;
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
		texScreenWidth,texScreenHeight);
	// Bind the depth buffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, depthBuf);

	GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBufs);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	light[0].type=0;
	light[0].pos = vec4(10,10,0,1);
	light[0].specular = vec3(1.0,1.0,1.0);
	light[0].diffuse = vec3(1.0, 1.0, 1.0);
	light[0].ambient = vec3(1.0, 1.0, 1.0);
	light[0].dir = vec4(0,-1,0,1);
	light[0].spotCutOff = cos(10.0/180*M_PI);

	//fog.maxDist=4;
	//fog.minDist=3;
	//fog.color = fogColor;

	setupShaders();

	cube = new Cube();
	//cube->setShader(sdrCtl.getShader("basic_phong"));
	cube->setKd(vec3(0.8, 0.0, 0.0));
	cube->setKa(vec3(0.3, 0.0, 0.0));
	cube->setKs(vec3(0.4, 0.0, 0.0));
	cube->setShininess(100);
	cube->setReflectFactor(vec2(0.2, 0.5));
	cube->setEta(0.5);
	cube->setCubeMapUnit(3);
	cube->setSpeed(5);
	cube->postTrans(glm::translate(vec3(0, 0.5, 0)));
	cube->setAABB(AABB(vec3(-0.5,-0.5,-0.5),vec3(0.5,0.5,0.5)));

	//Name and type
	cube->setType("Cube");
	cube->setName("Player Cube");
	//Add Cube to the draw list
	player_list.push_back(cube);
	//scene->addPlayer(cube);

	//TEST
	Cube* cube2 = new Cube();
	//cube->setShader(sdrCtl.getShader("basic_phong"));
	cube2->setKd(vec3(0.8, 0.0, 0.0));
	cube2->setKa(vec3(0.3, 0.0, 0.0));
	cube2->setKs(vec3(0.4, 0.0, 0.0));
	cube2->setShininess(100);
	cube2->setReflectFactor(vec2(0.2, 0.5));
	cube2->setEta(0.5);
	cube2->setCubeMapUnit(3);
	cube2->setSpeed(5);
	cube2->postTrans(glm::translate(vec3(3, 0.5, 0)));
	cube2->setAABB(AABB(vec3(-0.5, -0.5, -0.5), vec3(0.5, 0.5, 0.5)));

	//Name and type
	cube2->setType("Cube");
	cube2->setName("Test Cube");
	//Add Cube to the draw list
	player_list.push_back(cube2);
	//scene->addStationary(cube2);

	Cube* cube3 = new Cube();
	//cube->setShader(sdrCtl.getShader("basic_phong"));
	cube3->setKd(vec3(0.8, 0.0, 0.0));
	cube3->setKa(vec3(0.3, 0.0, 0.0));
	cube3->setKs(vec3(0.4, 0.0, 0.0));
	cube3->setShininess(100);
	cube3->setReflectFactor(vec2(0.2, 0.5));
	cube3->setEta(0.5);
	cube3->setCubeMapUnit(3);
	cube3->setSpeed(5);
	cube3->postTrans(glm::translate(vec3(-3, 0.5, 0)));
	cube3->setAABB(AABB(vec3(-0.5, -0.5, -0.5), vec3(0.5, 0.5, 0.5)));

	//Name and type
	cube3->setType("Cube");
	cube3->setName("Test Cube2");
	//Add Cube to the draw list
	player_list.push_back(cube3);
	//scene->addStationary(cube3);

	Cube* cube4 = new Cube();
	//cube->setShader(sdrCtl.getShader("basic_phong"));
	cube4->setKd(vec3(0.8, 0.0, 0.0));
	cube4->setKa(vec3(0.3, 0.0, 0.0));
	cube4->setKs(vec3(0.4, 0.0, 0.0));
	cube4->setShininess(100);
	cube4->setReflectFactor(vec2(0.2, 0.5));
	cube4->setEta(0.5);
	cube4->setCubeMapUnit(3);
	cube4->setSpeed(5);
	cube4->postTrans(glm::translate(vec3(0, 0.5, -3)));
	cube4->setAABB(AABB(vec3(-0.5, -0.5, -0.5), vec3(0.5, 0.5, 0.5)));

	//Name and type
	cube4->setType("Cube");
	cube4->setName("Test Cube3");
	//Add Cube to the draw list
	player_list.push_back(cube4);
	//scene->addStationary(cube4);

	Cube* cube5 = new Cube();
	//cube->setShader(sdrCtl.getShader("basic_phong"));
	cube5->setKd(vec3(0.8, 0.0, 0.0));
	cube5->setKa(vec3(0.3, 0.0, 0.0));
	cube5->setKs(vec3(0.4, 0.0, 0.0));
	cube5->setShininess(100);
	cube5->setReflectFactor(vec2(0.2, 0.5));
	cube5->setEta(0.5);
	cube5->setCubeMapUnit(3);
	cube5->setSpeed(5);
	cube5->postTrans(glm::translate(vec3(0, 0.5, 3)));
	cube5->setAABB(AABB(vec3(-0.5, -0.5, -0.5), vec3(0.5, 0.5, 0.5)));

	//Name and type
	cube5->setType("Cube");
	cube5->setName("Test Cube4");
	//Add Cube to the draw list
	stationary_list.push_back(cube5);
	//scene->addStationary(cube5);

	//Name and type
	//cam->setType("Camera");
	//cam->setName("Player Camera");
	//Add Cam to the draw list
	//draw_list.push_back(cam);

	ground = new Ground();
	ground->setKd(vec3(0.5,0.0,0.0));
	ground->setKa(vec3(0.3,0.0,0.0));
	ground->setKs(vec3(1.0,0.5,0.5));
	ground->setShininess(100);
	ground->setAABB(AABB(vec3(-10.0, 0.0, -10.0), vec3(10.0, 0.0, 10.0)));

	skybox = new SkyBox(-50, 50, -50, 50, -50, 50);
	//skybox->setShader(sdrCtl.getShader("basic_skybox"));
	skybox->setTexUnit(3);

	//Name and type
	skybox->setType("Skybox");
	skybox->setName("Skybox");
	//Add skybox to the draw list
	draw_list.push_back(skybox);

	//Name and type
	ground->setType("Ground");
	ground->setName("Ground");
	//Add Ground to the draw list
	stationary_list.push_back(ground);
	//scene->addStationary(ground);

	m_pMesh = new Mesh();
	m_pMesh->LoadMesh("Model/cube.dae");

	m_pMesh2 = new Mesh();
	m_pMesh2->LoadMesh("Model/monky2014_12.dae");

	recvVec->push_back(std::make_pair("initRecvPos_c", mat4(0.0f)));
	recvVec->push_back(std::make_pair("initRecvPos_c", mat4(0.0f)));
	recvVec->push_back(std::make_pair("initRecvPos_c", mat4(0.0f)));
	recvVec->push_back(std::make_pair("initRecvPos_c", mat4(0.0f)));
	stateVec->push_back(std::make_pair("initKey_c", mat4(0.0f)));
	stateVec->push_back(std::make_pair("initCam_c", mat4(0.0f)));

	try
	{
		cli = new tcp_client(io_service, "localhost", "13");
		io_service.run_one();
		io_service.run_one();
		pID = cli->pID();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	cam = new Camera();
	cam->attach(player_list[pID]);
	cam->postTrans(glm::translate(vec3(0, 1, 4)));

}
void loadTextures(){
	//cloth texture
	Texture* testTexture0 = new Texture(GL_TEXTURE_2D, "img/cloth.jpg", "JPG");
	testTexture0->Bind(GL_TEXTURE0);
	testTexture0->Load();
	texture_list.push_back(testTexture0);

	//moss texture
	Texture* testTexture1 = new Texture(GL_TEXTURE_2D, "img/moss.png", "PNG");
	testTexture1->Bind(GL_TEXTURE1);
	testTexture1->Load();
	texture_list.push_back(testTexture1);

	//cube map
	Texture* testTexture2 = new Texture(GL_TEXTURE_CUBE_MAP, "img/stormydays", "JPG");
	testTexture2->Bind(GL_TEXTURE2);
	testTexture2->LoadCube("img/stormydays", ".jpg");
	texture_list.push_back(testTexture2);

	//cube map
	Texture* testTexture3 = new Texture(GL_TEXTURE_CUBE_MAP, "img/deep_space_blue/space", "PNG");
	testTexture3->Bind(GL_TEXTURE3);
	testTexture3->LoadCube("img/deep_space_blue/space", ".png");
	texture_list.push_back(testTexture3);

	//cube map
	Texture* testTexture4 = new Texture(GL_TEXTURE_CUBE_MAP, "img/lake_pe1/lake_pe1", "JPG");
	testTexture4->Bind(GL_TEXTURE4);
	testTexture4->LoadCube("img/lake_pe1/lake_pe1", ".jpg");
	texture_list.push_back(testTexture4);

	//cube map
	Texture* testTexture5 = new Texture(GL_TEXTURE_CUBE_MAP, "img/lake_pe100/lake_pe100", "JPG");
	testTexture5->Bind(GL_TEXTURE5);
	testTexture5->LoadCube("img/lake_pe100/lake_pe100", ".jpg");
	texture_list.push_back(testTexture5);
}