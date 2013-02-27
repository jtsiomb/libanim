#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "anim.h"
#include "dynarr.h"

static void invalidate_cache(struct anm_node *node);

int anm_init_node(struct anm_node *node)
{
	int i, j;
	static const float defaults[] = {
		0.0f, 0.0f, 0.0f,		/* default position */
		0.0f, 0.0f, 0.0f, 1.0f,	/* default rotation quat */
		1.0f, 1.0f, 1.0f		/* default scale factor */
	};

	memset(node, 0, sizeof *node);

	/* initialize thread-local matrix cache */
	pthread_key_create(&node->cache_key, 0);

	for(i=0; i<ANM_NUM_TRACKS; i++) {
		if(anm_init_track(node->tracks + i) == -1) {
			for(j=0; j<i; j++) {
				anm_destroy_track(node->tracks + i);
			}
		}
		anm_set_track_default(node->tracks + i, defaults[i]);
	}
	return 0;
}

void anm_destroy_node(struct anm_node *node)
{
	int i;
	free(node->name);

	for(i=0; i<ANM_NUM_TRACKS; i++) {
		anm_destroy_track(node->tracks + i);
	}

	/* destroy thread-specific cache */
	pthread_key_delete(node->cache_key);

	while(node->cache_list) {
		struct mat_cache *tmp = node->cache_list;
		node->cache_list = tmp->next;
		free(tmp);
	}
}

void anm_destroy_node_tree(struct anm_node *tree)
{
	struct anm_node *c, *tmp;

	if(!tree) return;

	c = tree->child;
	while(c) {
		tmp = c;
		c = c->next;

		anm_destroy_node_tree(tmp);
	}
	anm_destroy_node(tree);
}

struct anm_node *anm_create_node(void)
{
	struct anm_node *n;

	if((n = malloc(sizeof *n))) {
		if(anm_init_node(n) == -1) {
			free(n);
			return 0;
		}
	}
	return n;
}

void anm_free_node(struct anm_node *node)
{
	anm_destroy_node(node);
	free(node);
}

void anm_free_node_tree(struct anm_node *tree)
{
	struct anm_node *c, *tmp;

	if(!tree) return;

	c = tree->child;
	while(c) {
		tmp = c;
		c = c->next;

		anm_free_node_tree(tmp);
	}

	anm_free_node(tree);
}

int anm_set_node_name(struct anm_node *node, const char *name)
{
	char *str;

	if(!(str = malloc(strlen(name) + 1))) {
		return -1;
	}
	strcpy(str, name);
	free(node->name);
	node->name = str;
	return 0;
}

const char *anm_get_node_name(struct anm_node *node)
{
	return node->name ? node->name : "";
}

void anm_set_interpolator(struct anm_node *node, enum anm_interpolator in)
{
	int i;

	for(i=0; i<ANM_NUM_TRACKS; i++) {
		anm_set_track_interpolator(node->tracks + i, in);
	}
	invalidate_cache(node);
}

void anm_set_extrapolator(struct anm_node *node, enum anm_extrapolator ex)
{
	int i;

	for(i=0; i<ANM_NUM_TRACKS; i++) {
		anm_set_track_extrapolator(node->tracks + i, ex);
	}
	invalidate_cache(node);
}

void anm_link_node(struct anm_node *p, struct anm_node *c)
{
	c->next = p->child;
	p->child = c;

	c->parent = p;
	invalidate_cache(c);
}

int anm_unlink_node(struct anm_node *p, struct anm_node *c)
{
	struct anm_node *iter;

	if(p->child == c) {
		p->child = c->next;
		c->next = 0;
		invalidate_cache(c);
		return 0;
	}

	iter = p->child;
	while(iter->next) {
		if(iter->next == c) {
			iter->next = c->next;
			c->next = 0;
			invalidate_cache(c);
			return 0;
		}
	}
	return -1;
}

void anm_set_position(struct anm_node *node, vec3_t pos, anm_time_t tm)
{
	anm_set_value(node->tracks + ANM_TRACK_POS_X, tm, pos.x);
	anm_set_value(node->tracks + ANM_TRACK_POS_Y, tm, pos.y);
	anm_set_value(node->tracks + ANM_TRACK_POS_Z, tm, pos.z);
	invalidate_cache(node);
}

vec3_t anm_get_node_position(struct anm_node *node, anm_time_t tm)
{
	vec3_t v;
	v.x = anm_get_value(node->tracks + ANM_TRACK_POS_X, tm);
	v.y = anm_get_value(node->tracks + ANM_TRACK_POS_Y, tm);
	v.z = anm_get_value(node->tracks + ANM_TRACK_POS_Z, tm);
	return v;
}

void anm_set_rotation(struct anm_node *node, quat_t rot, anm_time_t tm)
{
	anm_set_value(node->tracks + ANM_TRACK_ROT_X, tm, rot.x);
	anm_set_value(node->tracks + ANM_TRACK_ROT_Y, tm, rot.y);
	anm_set_value(node->tracks + ANM_TRACK_ROT_Z, tm, rot.z);
	anm_set_value(node->tracks + ANM_TRACK_ROT_W, tm, rot.w);
	invalidate_cache(node);
}

quat_t anm_get_node_rotation(struct anm_node *node, anm_time_t tm)
{
	int idx0, idx1, last_idx;
	anm_time_t tstart, tend;
	float t, dt;
	struct anm_track *track_x, *track_y, *track_z, *track_w;
	quat_t q, q1, q2;

	track_x = node->tracks + ANM_TRACK_ROT_X;
	track_y = node->tracks + ANM_TRACK_ROT_Y;
	track_z = node->tracks + ANM_TRACK_ROT_Z;
	track_w = node->tracks + ANM_TRACK_ROT_W;

	if(!track_x->count) {
		q.x = track_x->def_val;
		q.y = track_y->def_val;
		q.z = track_z->def_val;
		q.w = track_w->def_val;
		return q;
	}

	last_idx = track_x->count - 1;

	tstart = track_x->keys[0].time;
	tend = track_x->keys[last_idx].time;
	tm = anm_remap_time(track_x, tm, tstart, tend);

	idx0 = anm_get_key_interval(track_x, tm);
	assert(idx0 >= 0 && idx0 < track_x->count);
	idx1 = idx0 + 1;

	dt = (float)(track_x->keys[idx1].time - track_x->keys[idx0].time);
	t = (float)(tm - track_x->keys[idx0].time) / dt;

	q1.x = track_x->keys[idx0].val;
	q1.y = track_y->keys[idx0].val;
	q1.z = track_z->keys[idx0].val;
	q1.w = track_w->keys[idx0].val;

	q2.x = track_x->keys[idx1].val;
	q2.y = track_y->keys[idx1].val;
	q2.z = track_z->keys[idx1].val;
	q2.w = track_w->keys[idx1].val;

	return quat_slerp(q1, q2, t);
}

void anm_set_scaling(struct anm_node *node, vec3_t scl, anm_time_t tm)
{
	anm_set_value(node->tracks + ANM_TRACK_SCL_X, tm, scl.x);
	anm_set_value(node->tracks + ANM_TRACK_SCL_Y, tm, scl.y);
	anm_set_value(node->tracks + ANM_TRACK_SCL_Z, tm, scl.z);
	invalidate_cache(node);
}

vec3_t anm_get_node_scaling(struct anm_node *node, anm_time_t tm)
{
	vec3_t v;
	v.x = anm_get_value(node->tracks + ANM_TRACK_SCL_X, tm);
	v.y = anm_get_value(node->tracks + ANM_TRACK_SCL_Y, tm);
	v.z = anm_get_value(node->tracks + ANM_TRACK_SCL_Z, tm);
	return v;
}


vec3_t anm_get_position(struct anm_node *node, anm_time_t tm)
{
	mat4_t xform;
	vec3_t pos = {0.0, 0.0, 0.0};

	if(!node->parent) {
		return anm_get_node_position(node, tm);
	}

	anm_get_matrix(node, xform, tm);
	return v3_transform(pos, xform);
}

quat_t anm_get_rotation(struct anm_node *node, anm_time_t tm)
{
	quat_t rot, prot;
	rot = anm_get_node_rotation(node, tm);

	if(!node->parent) {
		return rot;
	}

	prot = anm_get_rotation(node->parent, tm);
	return quat_mul(prot, rot);
}

vec3_t anm_get_scaling(struct anm_node *node, anm_time_t tm)
{
	vec3_t s, ps;
	s = anm_get_node_scaling(node, tm);

	if(!node->parent) {
		return s;
	}

	ps = anm_get_scaling(node->parent, tm);
	return v3_mul(s, ps);
}

void anm_set_pivot(struct anm_node *node, vec3_t piv)
{
	node->pivot = piv;
}

vec3_t anm_get_pivot(struct anm_node *node)
{
	return node->pivot;
}

void anm_get_node_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm)
{
	mat4_t tmat, rmat, smat, pivmat, neg_pivmat;
	vec3_t pos, scale;
	quat_t rot;

	m4_identity(tmat);
	/*no need to m4_identity(rmat); quat_to_mat4 sets this properly */
	m4_identity(smat);
	m4_identity(pivmat);
	m4_identity(neg_pivmat);

	pos = anm_get_node_position(node, tm);
	rot = anm_get_node_rotation(node, tm);
	scale = anm_get_node_scaling(node, tm);

	m4_translate(pivmat, node->pivot.x, node->pivot.y, node->pivot.z);
	m4_translate(neg_pivmat, -node->pivot.x, -node->pivot.y, -node->pivot.z);

	m4_translate(tmat, pos.x, pos.y, pos.z);
	quat_to_mat4(rmat, rot);
	m4_scale(smat, scale.x, scale.y, scale.z);

	/* ok this would look nicer in C++ */
	m4_mult(mat, pivmat, tmat);
	m4_mult(mat, mat, rmat);
	m4_mult(mat, mat, smat);
	m4_mult(mat, mat, neg_pivmat);
}

void anm_get_node_inv_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm)
{
	mat4_t tmp;
	anm_get_node_matrix(node, tmp, tm);
	m4_inverse(mat, tmp);
}

void anm_get_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm)
{
	struct mat_cache *cache = pthread_getspecific(node->cache_key);
	if(!cache) {
		cache = malloc(sizeof *cache);
		assert(cache);

		pthread_mutex_lock(&node->cache_list_lock);
		cache->next = node->cache_list;
		node->cache_list = cache;
		pthread_mutex_unlock(&node->cache_list_lock);

		cache->time = ANM_TIME_INVAL;
		cache->inv_time = ANM_TIME_INVAL;
		pthread_setspecific(node->cache_key, cache);
	}

	if(cache->time != tm) {
		anm_get_node_matrix(node, cache->matrix, tm);

		if(node->parent) {
			mat4_t parent_mat;

			anm_get_matrix(node->parent, parent_mat, tm);
			m4_mult(cache->matrix, parent_mat, cache->matrix);
		}
		cache->time = tm;
	}
	m4_copy(mat, cache->matrix);
}

void anm_get_inv_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm)
{
	struct mat_cache *cache = pthread_getspecific(node->cache_key);
	if(!cache) {
		cache = malloc(sizeof *cache);
		assert(cache);

		pthread_mutex_lock(&node->cache_list_lock);
		cache->next = node->cache_list;
		node->cache_list = cache;
		pthread_mutex_unlock(&node->cache_list_lock);

		cache->inv_time = ANM_TIME_INVAL;
		cache->inv_time = ANM_TIME_INVAL;
		pthread_setspecific(node->cache_key, cache);
	}

	if(cache->inv_time != tm) {
		anm_get_matrix(node, mat, tm);
		m4_inverse(cache->inv_matrix, mat);
		cache->inv_time = tm;
	}
	m4_copy(mat, cache->inv_matrix);
}

anm_time_t anm_get_start_time(struct anm_node *node)
{
	int i;
	struct anm_node *c;
	anm_time_t res = LONG_MAX;

	for(i=0; i<ANM_NUM_TRACKS; i++) {
		if(node->tracks[i].count) {
			anm_time_t tm = node->tracks[i].keys[0].time;
			if(tm < res) {
				res = tm;
			}
		}
	}

	c = node->child;
	while(c) {
		anm_time_t tm = anm_get_start_time(c);
		if(tm < res) {
			res = tm;
		}
		c = c->next;
	}
	return res;
}

anm_time_t anm_get_end_time(struct anm_node *node)
{
	int i;
	struct anm_node *c;
	anm_time_t res = LONG_MIN;

	for(i=0; i<ANM_NUM_TRACKS; i++) {
		if(node->tracks[i].count) {
			anm_time_t tm = node->tracks[i].keys[node->tracks[i].count - 1].time;
			if(tm > res) {
				res = tm;
			}
		}
	}

	c = node->child;
	while(c) {
		anm_time_t tm = anm_get_end_time(c);
		if(tm > res) {
			res = tm;
		}
		c = c->next;
	}
	return res;
}

static void invalidate_cache(struct anm_node *node)
{
	struct mat_cache *cache = pthread_getspecific(node->cache_key);
	if(cache) {
	   cache->time = ANM_TIME_INVAL;
	}
}
