#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include "anim.h"

#define DEG_TO_RAD(x)	((x) / 180.0 * M_PI)

struct body_part {
	float pos[3], pivot[3], sz[3], color[4];
} parts[] = {
	/* position			pivot			size			color */
	{{0, 1, 0},			{0, -1, 0},		{1.8, 2.8, 0.8},{1, 0, 1, 1}},		/* torso */

	{{-0.5, -2.5, 0},	{0, 1, 0},		{0.8, 2, 0.8},	{1, 0, 0, 1}},		/* left-upper leg */
	{{0.5, -2.5, 0},	{0, 1, 0},		{0.8, 2, 0.8},	{0, 1, 0, 1}},		/* right-upper leg */

	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{1, 0.5, 0.5, 1}},	/* left-lower leg */
	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{0.5, 1, 0.5, 1}},	/* right-lower leg */

	{{0, 2.6, 0},		{0, -0.5, 0},	{1.2, 1.2, 1.2},{0, 1, 1, 1}},		/* head */

	{{-1.3, 0.4, 0},	{0, 1, 0},		{0.8, 2, 0.8},	{0, 0, 1, 1}},		/* left-upper arm */
	{{1.3, 0.4, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{1, 1, 0, 1}},		/* right-upper arm */

	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{0.5, 0.5, 1, 1}},	/* left-lower arm */
	{{0, -2.1, 0},		{0, 1, 0},		{0.8, 2, 0.8},	{1, 1, 0.5, 1}},	/* right-lower arm */
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
static void set_walk_animation(int idx);
static void set_jump_animation(int idx);
void disp(void);
void idle(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);

float cam_theta = 200, cam_phi = 20, cam_dist = 15;
struct anm_node *root;

struct anm_node *nodes[NUM_NODES];

int cur_anim = 0, next_anim = 0;
unsigned int trans_start_tm;

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

	for(i=0; i<NUM_NODES; i++) {
		nodes[i] = anm_create_node();
		anm_set_pivot(nodes[i], parts[i].pivot[0], parts[i].pivot[1], parts[i].pivot[2]);
	}
	root = nodes[0];

	anm_link_node(nodes[NODE_TORSO], nodes[NODE_HEAD]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_LEFT_UPPER_LEG]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_RIGHT_UPPER_LEG]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_LEFT_UPPER_ARM]);
	anm_link_node(nodes[NODE_TORSO], nodes[NODE_RIGHT_UPPER_ARM]);
	anm_link_node(nodes[NODE_LEFT_UPPER_LEG], nodes[NODE_LEFT_LOWER_LEG]);
	anm_link_node(nodes[NODE_RIGHT_UPPER_LEG], nodes[NODE_RIGHT_LOWER_LEG]);
	anm_link_node(nodes[NODE_LEFT_UPPER_ARM], nodes[NODE_LEFT_LOWER_ARM]);
	anm_link_node(nodes[NODE_RIGHT_UPPER_ARM], nodes[NODE_RIGHT_LOWER_ARM]);

	set_walk_animation(0);

	anm_add_animation(root);	/* recursively add another animation slot to all nodes */
	set_jump_animation(1);

	anm_use_animation(root, cur_anim);

	return 0;
}

static void set_walk_animation(int idx)
{
	int i;

	anm_use_animation(root, idx);
	anm_set_active_animation_name(root, "walk");

	for(i=0; i<NUM_NODES; i++) {
		anm_set_position(nodes[i], parts[i].pos, 0);
		anm_set_extrapolator(nodes[i], ANM_EXTRAP_REPEAT);
	}

	/* upper leg animation */
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(-15), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(45), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(-15), 1, 0, 0, 2000);

	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(45), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(-15), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(45), 1, 0, 0, 2000);

	/* lower leg animation */
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(-90), 1, 0, 0, 500);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(-10), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 2000);

	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(-10), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(-90), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(-10), 1, 0, 0, 2000);

	/* head animation */
	anm_set_rotation_axis(nodes[NODE_HEAD], DEG_TO_RAD(-10), 0, 1, 0, 0);
	anm_set_rotation_axis(nodes[NODE_HEAD], DEG_TO_RAD(10), 0, 1, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_HEAD], DEG_TO_RAD(-10), 0, 1, 0, 2000);

	/* torso animation */
	anm_set_rotation_axis(nodes[NODE_TORSO], DEG_TO_RAD(-8), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_TORSO], DEG_TO_RAD(0), 1, 0, 0, 500);
	anm_set_rotation_axis(nodes[NODE_TORSO], DEG_TO_RAD(-8), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_TORSO], DEG_TO_RAD(0), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_TORSO], DEG_TO_RAD(-8), 1, 0, 0, 2000);

	/* upper arm animation */
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(30), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(-25), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(30), 1, 0, 0, 2000);

	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(-25), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(30), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(-25), 1, 0, 0, 2000);

	/* lower arm animation */
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_ARM], DEG_TO_RAD(40), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_ARM], DEG_TO_RAD(0), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_ARM], DEG_TO_RAD(0), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_ARM], DEG_TO_RAD(40), 1, 0, 0, 2000);

	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_ARM], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_ARM], DEG_TO_RAD(0), 1, 0, 0, 500);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_ARM], DEG_TO_RAD(40), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_ARM], DEG_TO_RAD(0), 1, 0, 0, 2000);
}

static void set_jump_animation(int idx)
{
	int i;
	float *vptr;

	anm_use_animation(root, idx);
	anm_set_active_animation_name(root, "jump");

	for(i=0; i<NUM_NODES; i++) {
		anm_set_position(nodes[i], parts[i].pos, 0);
		anm_set_extrapolator(nodes[i], ANM_EXTRAP_REPEAT);
	}

	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(40), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(0), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_LEG], DEG_TO_RAD(0), 1, 0, 0, 2000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(40), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(0), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_LEG], DEG_TO_RAD(0), 1, 0, 0, 2000);

	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(-80), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_LEFT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 2000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(-80), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_RIGHT_LOWER_LEG], DEG_TO_RAD(0), 1, 0, 0, 2000);

	vptr = parts[NODE_TORSO].pos;
	anm_set_position(nodes[NODE_TORSO], vptr, 0);
	anm_set_position3f(nodes[NODE_TORSO], vptr[0], vptr[1] - 1, vptr[2], 1000);
	anm_set_position3f(nodes[NODE_TORSO], vptr[0], vptr[1] + 2, vptr[2], 1500);
	anm_set_position(nodes[NODE_TORSO], vptr, 2000);

	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(-20), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(20), 1, 0, 0, 1200);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(170), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_LEFT_UPPER_ARM], DEG_TO_RAD(0), 1, 0, 0, 2000);

	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(0), 1, 0, 0, 0);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(-20), 1, 0, 0, 1000);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(20), 1, 0, 0, 1200);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(170), 1, 0, 0, 1500);
	anm_set_rotation_axis(nodes[NODE_RIGHT_UPPER_ARM], DEG_TO_RAD(0), 1, 0, 0, 2000);
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
		float xform[16];

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, parts[i].color);
		glColor4fv(parts[i].color);

		anm_get_matrix(nodes[i], xform, msec);

		glPushMatrix();
		glMultMatrixf(xform);

		glScalef(parts[i].sz[0], parts[i].sz[1], parts[i].sz[2]);
		glutSolidCube(1.0);

		glPopMatrix();
	}
	glPopMatrix();

	/* then render another one using simple top-down recursive evaluation */
	glPushMatrix();
	glTranslatef(2.5, 0, 0);

	anm_eval(nodes[NODE_TORSO], msec);	/* calculate all matrices recursively */

	for(i=0; i<NUM_NODES; i++) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, parts[i].color);
		glColor4fv(parts[i].color);

		glPushMatrix();
		glMultMatrixf(nodes[i]->matrix);

		glScalef(parts[i].sz[0], parts[i].sz[1], parts[i].sz[2]);
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

	case ' ':
		cur_anim = anm_get_active_animation_index(root, 0);
		next_anim = (cur_anim + 1) % 2;
		trans_start_tm = glutGet(GLUT_ELAPSED_TIME);
		anm_transition(root, next_anim, trans_start_tm, 1500);
		break;
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
