#define _CRT_SECURE_NO_WARNINGS
#include "Render.h"

#include <sstream>
#include <iostream>
#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <conio.h>
#include "MyOGL.h"
#include "Camera.h"
#include "Light.h"
#include "Primitives.h"
#include "GUItextRectangle.h"
#include <fstream>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <time.h>



#pragma comment(lib, "Winmm.lib")

using namespace std;
bool textureMode = true;
bool lightMode = true;
bool doLoadObject = true;
double score = 0;

bool Terrka = true;
bool Obyect = true;
bool Body = false;
bool Nachalo = false;
int Flag = 0;
long int Body_num;

//размер бошки
double razmer = 10; 

//���������� ��� ������
int jump = 0;
double jump_L = 0;
double jump_nu = PI/4;

//движ
double walk_x=0;
double walk_y=0;
double walk_z=0;

bool isDead;

double dist=-30;
double visota = 0;
bool updown = false;

struct Point {
	double x, y, z;
};

struct UVPoint {
	double u, v;
};

struct Edge {
	long int A, B, C;
};

UVPoint* body_uv_points = (UVPoint*)malloc(sizeof(UVPoint));
Edge* body_edges = (Edge*)malloc(sizeof(Edge));
Point* body_points = (Point*)malloc(sizeof(Point));

void LoadObj(const char* file_path, string part)
{
	long int edges_num;

	Edge* part_edges = (Edge*)malloc(sizeof(Edge));
	Point* part_points = (Point*)malloc(sizeof(Point));
	UVPoint* part_uv_points = (UVPoint*)malloc(sizeof(UVPoint));

	string j_str;
	const char* outp;
	
	char stroka[50] = "none";
	FILE* model;
	model = fopen(file_path, "r");
	if (model != 0)
	{
		long int j = 0;
		long int uv = 0;
		long int k = 0;
		while (fgets(stroka, 50, model) != NULL )
		{
			int i = 0;
			while (i < 50)	
			{
				if (stroka[0] == 'v' && stroka[1] == 32)
				{
					j++;
					string x_str, y_str, z_str;
					double x, y, z;
					i=i+2;
					while (stroka[i] != 32)
					{
						x_str = x_str + stroka[i];
						i++;
					}
					x = std::stod(x_str);
					i = i++;

					while (stroka[i] != 32)
					{
						y_str = y_str + stroka[i];
						i++;
					}

					y = std::stod(y_str);
					i = i + 1;

					while (stroka[i] != 32 && i<50)
					{
						z_str = z_str + stroka[i];
						i++;
					}
					z = std::stod(z_str);

					size_t size = _msize(part_points);
					part_points = (Point*)realloc(part_points, size + sizeof(Point));
					part_points[j].x = x * razmer;
					part_points[j].y = y * razmer;
					part_points[j].z = z * razmer;
				}

				if (stroka[0] == 'v' && stroka[1] == 't')
				{
					uv++;
					string u_str;
					string v_str;
					double u;
					double v;
					i = i + 3;

					while (stroka[i] != 32)
					{
						u_str = u_str + stroka[i];
						i++;
					}

					u = std::stod(u_str);
					i = i + 1;

					while (stroka[i] != 32 && i < 50)
					{
						v_str = v_str + stroka[i];
						i++;
					}

					v = std::stod(v_str);
					size_t size = _msize(part_uv_points);
					part_uv_points = (UVPoint*)realloc(part_uv_points, size + sizeof(UVPoint));
					part_uv_points[uv].u = u;
					part_uv_points[uv].v = v;

				}

				if (stroka[0] == 'f')
				{
					k++;

					string A_str, B_str, C_str;
					double A, B, C;
					i = i + 2;
					while (stroka[i] != 32)
					{
						if (stroka[i] == 47)
						{
							while (stroka[i] != 32)
								i++;
						}
						else
						{
							A_str = A_str + stroka[i];
							i++;
						}
					}

					A = std::stod(A_str);
					i = i + 1;
					while (stroka[i] != 32)
					{
						if (stroka[i] == 47)
						{
							while (stroka[i] != 32)
								i++;
						}
						else
						{
							B_str = B_str + stroka[i];
							i++;
						}


					}
					B = std::stod(B_str);

					i = i + 1;

					while (stroka[i] != 32  && i < 50)
					{
						if (stroka[i] == 47)
						{
							while (stroka[i] != 32)

								i++;

						}
						else
						{
							C_str = C_str + stroka[i];
							i++;
						}

					}
					C = std::stod(C_str);

					size_t size = _msize(part_edges);
					part_edges = (Edge*)realloc(part_edges, size + sizeof(Edge));
					part_edges[k].A = A;
					part_edges[k].B = B;
					part_edges[k].C = C;
				}

				i = 60;
			}
		}

		edges_num = k;
		fclose(model);
	}

	if (part == "body")
	{
		Body = true;
		Body_num = edges_num;
		body_edges = part_edges;
		body_points = part_points;
		body_uv_points = part_uv_points;
	}
}


int GetRandomNumber(int min, int max)
{
	srand(time(NULL));

	int num = min + rand() % (max - min + 1);

	return num;
}

class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   

class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1.};

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  

int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{

		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 32 && jump!=1)
	{

		jump=1;
		Nachalo = true;
		Flag++;
		if (Flag == 1) {
			mciSendString("close MP3", NULL, 0, NULL);
			mciSendString("open \"THEROCK.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
			mciSendString("play mp3 from 0", NULL, 0, NULL);
		}		
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}

GLUquadricObj *quadObj = gluNewQuadric();;
GLuint tex;

void initRender(OpenGL *ogl)
{

if (doLoadObject == true)
{
	LoadObj("Objects\\rock.obj", "body");
}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);

	RGBTRIPLE *texarray;

	char *texCharArray;
	int texW, texH;

	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	free(texCharArray);
	free(texarray);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	glEnable(GL_NORMALIZE);

	glEnable(GL_LINE_SMOOTH); 

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);



	light.pos = Vector3(-1.5, -4, 4);
	camera.camDist = 40.8;
	camera.fi1 = -2.18999;
	camera.fi2 = 0.16999;
	
	
	
}

void Dead()
{
	Nachalo = false;
	mciSendString("close MP3", NULL, 0, NULL);
	mciSendString("open \"vine-boom.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
	mciSendString("play mp3 from 0", NULL, 0, NULL);
	
	score = 0;
	dist = -30;
	walk_z = 0;
	visota = GetRandomNumber(0, 10);
	updown = GetRandomNumber(0, 1);
	Flag = 0;
	
}

void Render(OpenGL *ogl)
{
	if (Nachalo == true)
	{
		if (jump == 1)
		{
			jump_nu = jump_nu + PI / 20;
			jump_L = sin(jump_nu);
			walk_z = walk_z + jump_L;
			if (jump_nu > PI / 2)
			{
				jump = 0;
			}
		}
		else
			if (jump_L > 0)
			{
				jump_nu = jump_nu - PI ;
				jump_L = sin(jump_nu);
				walk_z = walk_z - 0.5;
			}
			else
			{
				jump_nu = PI/10 ;
				jump = 0;
				walk_z = walk_z - 0.5;
			}

		if (walk_z > 20)
			walk_z = 20;

		if (walk_z < -20)
		{
			Dead();
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	GLfloat amb[] = { 1, 1, 1, 1. };
	GLfloat dif[] = { 1, 1, 1, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  

	glTranslatef(walk_x, walk_y, walk_z);
	//glRotatef(walk_angle, 0, 0, 1);


	if (Body = true)
	for (int i = 1; i < Body_num; i++)
	{
		double P1[] = { body_points[body_edges[i].A].x,body_points[body_edges[i].A].y,body_points[body_edges[i].A].z };
		double P2[] = { body_points[body_edges[i].B].x,body_points[body_edges[i].B].y,body_points[body_edges[i].B].z };
		double P3[] = { body_points[body_edges[i].C].x,body_points[body_edges[i].C].y,body_points[body_edges[i].C].z };

		Point a, b;
		a.x = P1[0] - P2[0];
		a.y = P1[1] - P2[1];
		a.z = P1[2] - P2[2];

		b.x = P1[0] - P3[0];
		b.y = P1[1] - P3[1];
		b.z = P1[2] - P3[2];
		double n[] = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
		glBindTexture(GL_TEXTURE_2D, tex);
		glColor3d(0.95, 0.95, 0.95);
		glBegin(GL_TRIANGLES);
		glNormal3d(n[0], n[1], n[2]);
		glTexCoord2f(body_uv_points[body_edges[i].A].u, body_uv_points[body_edges[i].A].v);
		glVertex3f(P1[0], P1[1], P1[2]);
		glTexCoord2d(body_uv_points[body_edges[i].B].u, body_uv_points[body_edges[i].B].v);
		glVertex3f(P2[0], P2[1], P2[2]);
		glTexCoord2d(body_uv_points[body_edges[i].C].u, body_uv_points[body_edges[i].C].v);
		glVertex3f(P3[0], P3[1], P3[2]);
		glEnd();
	}
	glTranslatef(-walk_x, -walk_y, -walk_z);

	if (Obyect == true)
	{
		if (Nachalo == true)
		{
			dist = dist + (0.2+sqrt(score)/20);
		}
 
		if (dist-4 < 0 && dist + 4 > 0 && -visota < walk_z && updown==true)
		{
  			Dead();
		}	

		if (dist - 4 < 0 && dist + 4 > 0 && visota > walk_z && updown == false)
		{
			Dead();
		}

		if (dist > 2)
		{
			dist = -20;
			visota = GetRandomNumber(-10, 10);
			score++;
			updown= GetRandomNumber(0, 1);
		}
	}

	if (Obyect == true)
	{

		double EnA[] = { -2, dist - 2, -visota };
		double EnB[] = { -2, dist + 2, -visota };
		double EnC[] = { +2, dist + 2, -visota };
		double EnD[] = { +2, dist - 2, -visota };
		double EnA1[] = { -2, dist - 2, 20 };
		double EnB1[] = { -2, dist + 2, 20 };
		double EnC1[] = { +2, dist + 2, 20 };
		double EnD1[] = { +2, dist - 2, 20 };
		if (updown == false)
		{
			EnA[2] = visota;
			EnB[2] = visota;
			EnC[2] = visota;
			EnD[2] = visota;
			EnA1[2] = -20;
			EnB1[2] = -20;
			EnC1[2] = -20;
			EnD1[2] = -20;
		}
		glColor3d(0.6, 0.6, 0.6);
		glBegin(GL_QUADS);
		glNormal3d(0, 1, 0);
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(EnA);
		glTexCoord2d(0.5, 0.1);
		glVertex3dv(EnA1);
		glTexCoord2d(1, 0.1);
		glVertex3dv(EnB1);
		glTexCoord2d(1, 0.5);
		glVertex3dv(EnB);
		glEnd();

		glBegin(GL_QUADS);
		glNormal3d(1, 0, 0);
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(EnB);
		glTexCoord2d(0.5, 0.1);
		glVertex3dv(EnB1);
		glTexCoord2d(1, 0.1);
		glVertex3dv(EnC1);
		glTexCoord2d(1, 0.5);
		glVertex3dv(EnC);
		glEnd();

		glBegin(GL_QUADS);
		glNormal3d(0, -1, 0);
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(EnC);
		glTexCoord2d(0.5, 0.1);
		glVertex3dv(EnC1);
		glTexCoord2d(1, 0.1);
		glVertex3dv(EnD1);
		glTexCoord2d(1, 0.5);
		glVertex3dv(EnD);
		glEnd();

		glBegin(GL_QUADS);
		glNormal3d(-1, 0, 0);
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(EnD);
		glTexCoord2d(0.5, 0.1);
		glVertex3dv(EnD1);
		glTexCoord2d(1, 0.1);
		glVertex3dv(EnA1);
		glTexCoord2d(1, 0.5);
		glVertex3dv(EnA);
		glEnd();

		glBegin(GL_QUADS);
		glNormal3d(0, 0, 1);
		glTexCoord2d(0.5, 0.5);
		glVertex3dv(EnA);
		glTexCoord2d(1, 0.5);
		glVertex3dv(EnB);
		glTexCoord2d(1, 1);
		glVertex3dv(EnC);
		glTexCoord2d(0.5, 1);
		glVertex3dv(EnD);
		glEnd();

		glBegin(GL_QUADS);
		glNormal3d(0, 0, -1);
		glTexCoord2d(0.7, 0.7);
		glVertex3dv(EnA1);
		glTexCoord2d(0.7, 0.8);
		glVertex3dv(EnB1);
		glTexCoord2d(0.8, 0.8);
		glVertex3dv(EnC1);
		glTexCoord2d(0.8, 0.7);
		glVertex3dv(EnD1);
		glEnd();
	}

	if (Terrka == true)
	{
		double TA[] = { -5,-30,-20 };
		double TB[] = { -5,30,-20 };
		double TC[] = { 5,30,-20 };
		double TD[] = { 5,-30,-20 };
		glBegin(GL_QUADS);
		glNormal3d(0, 0, 1);
		glTexCoord2d(0.5, 0);
		glVertex3dv(TA);
		glTexCoord2d(0.5, 0.09);
		glVertex3dv(TB);
		glTexCoord2d(1, 0.09);
		glVertex3dv(TC);
		glTexCoord2d(1, 0);
		glVertex3dv(TD);
		glEnd();
	}

	
	glMatrixMode(GL_PROJECTION);	
	                               
	glPushMatrix();  		    
	glLoadIdentity();	
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	

	glMatrixMode(GL_MODELVIEW);		
	glPushMatrix();			 
	glLoadIdentity();	

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;	
	rec.setSize(400, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);
	

	std::stringstream ss;
	ss << "Счет: " <<score<< std::endl;
	ss << "z: " << walk_z << std::endl;
	ss << "Скам: " << camera.camDist << " " << camera.fi1 << " " << camera.fi2 << std::endl;
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}