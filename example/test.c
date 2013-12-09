#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <vmath/vmath.h>
#include "anim.h"

struct body_part {
	vec3_t pos, pivot, sz, color;
} parts[] = {
	/* position			pivot			size			color */
	{{0, 1, 0},			{0, -1, 0},		{1.8, 2.8, 0.8},{1, 0, 1}},		/* torso */

	{{-0.5, -2.5, 0},	{0, 1, 0},		{0.8, 2, 0.8},	{1, 0, 0}},		/* left-upper leg */
	{{0.5, -2.5, 0},	{0, 1, 0},		{0.8, 2, 0.8},	{0, 1, 0}},		/* right-upper leg */

	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{1, 0.5, 0.5}},	/* left-lower leg */
	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{0.5, 1, 0.5}},	/* right-lower leg */

	{{0, 2.6, 0},		{0, -0.5, 0},	{1.2, 1.2, 1.2},{0, 1, 1}},	/* head */

	{{-1.3, 0.4, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{0, 0, 1}},	/* left-upper arm */
	{{1.3, 0.4, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{1, 1, 0}},		/* right-upper arm */

	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{0.5, 0.5, 1}},	/* left-lower arm */
	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{1, 1, 0.5}},	/* right-lower arm */
};

enum {
	NODE_TORSO,
	NODE_LEFT_UPPER_LEG,
	NODE_RIGHT_UPPER_LEG,
	NODE_LEFT_LOWER_LEG,
	NODE_RIGHT_LOWER_LEG,
	NODE_HEAD,
	NODE_LEFT_UPPER_ARM,
	NODE_RIGHT_UPPER_ARM,
	NODE_LEFT_LOWER_ARM,
	NODE_RIGHT_LOWER_ARM,

	NUM_NODES
};

int init(void);
void disp(void);
void idle(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);

float cam_theta = 200, cam_phi = 20, cam_dist = 15;
struct anm_node *root;

struct anm_node *nodes[NUM_NODES];

int main(int argc, char **argv)
{
	glutInitWindowSize(800, 600);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("libanim example");

	glutDisplayFunc(disp);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	if(init() == -1) {
		return 1;
	}
	glutMainLoop();
	return 0;
}

int init(void)
{
	int i;

	glPointSize(3);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	root = nodes[0];

	for(i=0; i<NUM_NODES; i++) {
		nodes[i] = anm_create_node();

		anm_set_pivot(nodes[i], parts[i].pivot);
		anm_set_position(nodes[i], parts[i].pos, 0);
		anm_set_extrapolator(nodes[i], ANM_EXTRAP_REPEAT);
	}

	anm_link_node(nodes[NODE_TORSO], nodes[NODE_HEAD]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_LEFT_UPPER_LEG]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_RIGHT_UPPER_LEG]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_LEFT_UPPER_ARM]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_RIGHT_UPPER_ARM]);
	anm_link_node(nodes[NODE_LEFT_UPPER_LEG], nodes[NODE_LEFT_LOWER_LEG]);
	anm_link_node(nodes[NODE_RIGHT_UPPER_LEG], nodes[NODE_RIGHT_LOWER_LEG]);
	anm_link_node(nodes[NODE_LEFT_UPPER_ARM], nodes[NODE_LEFT_LOWER_ARM]);
	anm_link_node(nodes[NODE_RIGHT_UPPER_ARM], nodes[NODE_RIGHT_LOWER_ARM]);

	/* upper leg animation */
	anm_set_rotation(nodes[NODE_LEFT_UPPER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-15), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_LEFT_UPPER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(45), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_LEFT_UPPER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-15), 1, 0, 0), 2000);

	anm_set_rotation(nodes[NODE_RIGHT_UPPER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(45), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_RIGHT_UPPER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-15), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_RIGHT_UPPER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(45), 1, 0, 0), 2000);

	/* lower leg animation */
	anm_set_rotation(nodes[NODE_LEFT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_LEFT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-90), 1, 0, 0), 500);
	anm_set_rotation(nodes[NODE_LEFT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-10), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_LEFT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 2000);

	anm_set_rotation(nodes[NODE_RIGHT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-10), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_RIGHT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_RIGHT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-90), 1, 0, 0), 1500);
	anm_set_rotation(nodes[NODE_RIGHT_LOWER_LEG], quat_rotate(quat_identity(), DEG_TO_RAD(-10), 1, 0, 0), 2000);

	/* head animation */
	anm_set_rotation(nodes[NODE_HEAD], quat_rotate(quat_identity(), DEG_TO_RAD(-10), 0, 1, 0), 0);
	anm_set_rotation(nodes[NODE_HEAD], quat_rotate(quat_identity(), DEG_TO_RAD(10), 0, 1, 0), 1000);
	anm_set_rotation(nodes[NODE_HEAD], quat_rotate(quat_identity(), DEG_TO_RAD(-10), 0, 1, 0), 2000);

	/* torso animation */
	anm_set_rotation(nodes[NODE_TORSO], quat_rotate(quat_identity(), DEG_TO_RAD(-8), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_TORSO], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 500);
	anm_set_rotation(nodes[NODE_TORSO], quat_rotate(quat_identity(), DEG_TO_RAD(-8), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_TORSO], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 1500);
	anm_set_rotation(nodes[NODE_TORSO], quat_rotate(quat_identity(), DEG_TO_RAD(-8), 1, 0, 0), 2000);

	/* upper arm animation */
	anm_set_rotation(nodes[NODE_LEFT_UPPER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(30), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_LEFT_UPPER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(-25), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_LEFT_UPPER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(30), 1, 0, 0), 2000);

	anm_set_rotation(nodes[NODE_RIGHT_UPPER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(-25), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_RIGHT_UPPER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(30), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_RIGHT_UPPER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(-25), 1, 0, 0), 2000);

	/* lower arm animation */
	anm_set_rotation(nodes[NODE_LEFT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(40), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_LEFT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_LEFT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 1500);
	anm_set_rotation(nodes[NODE_LEFT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(40), 1, 0, 0), 2000);

	anm_set_rotation(nodes[NODE_RIGHT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 0);
	anm_set_rotation(nodes[NODE_RIGHT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 500);
	anm_set_rotation(nodes[NODE_RIGHT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(40), 1, 0, 0), 1000);
	anm_set_rotation(nodes[NODE_RIGHT_LOWER_ARM], quat_rotate(quat_identity(), DEG_TO_RAD(0), 1, 0, 0), 2000);

	return 0;
}

void disp(void)
{
	int i;
	float lpos[] = {-1, 1, 1.5, 0};
	unsigned int msec = glutGet(GLUT_ELAPSED_TIME);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	/* first render a character with bottom-up lazy matrix calculation */
	glPushMatrix();
	glTranslatef(-2.5, 0, 0);

	for(i=0; i<NUM_NODES; i++) {
		float color[4] = {0, 0, 0, 1};
		mat4_t xform, xform_transp;

		color[0] = parts[i].color.x;
		color[1] = parts[i].color.y;
		color[2] = parts[i].color.z;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glColor4fv(color);

		anm_get_matrix(nodes[i], xform, msec);
		m4_transpose(xform_transp, xform);

		glPushMatrix();
		glMultMatrixf((float*)xform_transp);

		glScalef(parts[i].sz.x, parts[i].sz.y, parts[i].sz.z);
		glutSolidCube(1.0);

		glPopMatrix();
	}
	glPopMatrix();

	/* then render another one using simple top-down recursive evaluation */
	glPushMatrix();
	glTranslatef(2.5, 0, 0);

	anm_eval(nodes[NODE_TORSO], msec);	/* calculate all matrices recursively */

	for(i=0; i<NUM_NODES; i++) {
		float color[4] = {0, 0, 0, 1};
		mat4_t xform_transp;

		color[0] = parts[i].color.x;
		color[1] = parts[i].color.y;
		color[2] = parts[i].color.z;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glColor4fv(color);

		m4_transpose(xform_transp, nodes[i]->matrix);

		glPushMatrix();
		glMultMatrixf((float*)xform_transp);

		glScalef(parts[i].sz.x, parts[i].sz.y, parts[i].sz.z);
		glutSolidCube(1.0);

		glPopMatrix();
	}
	glPopMatrix();

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

void idle(void)
{
	glutPostRedisplay();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)x / (float)y, 0.5, 500.0);
}

void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);
	}
}

int bnstate[64];
int prev_x, prev_y;

void mouse(int bn, int state, int x, int y)
{
	int idx = bn - GLUT_LEFT_BUTTON;
	int down = state == GLUT_DOWN ? 1 : 0;

	bnstate[idx] = down;

	prev_x = x;
	prev_y = y;
}

void motion(int x, int y)
{
	int dx = x - prev_x;
	int dy = y - prev_y;
	prev_x = x;
	prev_y = y;

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) {
			cam_phi = -90;
		}
		if(cam_phi > 90) {
			cam_phi = 90;
		}
		glutPostRedisplay();
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0.0) {
			cam_dist = 0.0;
		}
		glutPostRedisplay();
	}
}
