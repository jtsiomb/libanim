#ifndef LIBANIM_H_
#define LIBANIM_H_

#include "config.h"

#include <pthread.h>

#include <vmath/vector.h>
#include <vmath/quat.h>
#include <vmath/matrix.h>
#include "track.h"

enum {
	ANM_TRACK_POS_X,
	ANM_TRACK_POS_Y,
	ANM_TRACK_POS_Z,

	ANM_TRACK_ROT_X,
	ANM_TRACK_ROT_Y,
	ANM_TRACK_ROT_Z,
	ANM_TRACK_ROT_W,

	ANM_TRACK_SCL_X,
	ANM_TRACK_SCL_Y,
	ANM_TRACK_SCL_Z,

	ANM_NUM_TRACKS
};

struct anm_animation {
	char *name;
	struct anm_track tracks[ANM_NUM_TRACKS];
};

struct anm_node {
	char *name;

	int cur_anim[2];
	float cur_mix;

	struct anm_animation *animations;
	vec3_t pivot;

	/* matrix cache */
	struct mat_cache {
		mat4_t matrix, inv_matrix;
		anm_time_t time, inv_time;
		struct mat_cache *next;
	} *cache_list;
	pthread_key_t cache_key;
	pthread_mutex_t cache_list_lock;

	/* matrix calculated by anm_eval functions (no locking, meant as a pre-pass) */
	mat4_t matrix;

	struct anm_node *parent;
	struct anm_node *child;
	struct anm_node *next;
};

#ifdef __cplusplus
extern "C" {
#endif

int anm_init_animation(struct anm_animation *anim);
void anm_destroy_animation(struct anm_animation *anim);

void anm_set_animation_name(struct anm_animation *anim, const char *name);


/* node constructor and destructor */
int anm_init_node(struct anm_node *node);
void anm_destroy_node(struct anm_node *node);

/* recursively destroy an animation node tree */
void anm_destroy_node_tree(struct anm_node *tree);

/* helper functions to allocate/construct and destroy/free with
 * a single call. They call anm_init_node and anm_destroy_node
 * internally.
 */
struct anm_node *anm_create_node(void);
void anm_free_node(struct anm_node *node);

/* recursively destroy and free the nodes of a node tree */
void anm_free_node_tree(struct anm_node *tree);

int anm_set_node_name(struct anm_node *node, const char *name);
const char *anm_get_node_name(struct anm_node *node);

/* link and unlink nodes with parent/child relations */
void anm_link_node(struct anm_node *parent, struct anm_node *child);
int anm_unlink_node(struct anm_node *parent, struct anm_node *child);

void anm_set_pivot(struct anm_node *node, vec3_t pivot);
vec3_t anm_get_pivot(struct anm_node *node);

/* set active animation(s) */
int anm_use_node_animation(struct anm_node *node, int aidx);
int anm_use_node_animations(struct anm_node *node, int aidx, int bidx, float t);
/* recursive variants */
int anm_use_animation(struct anm_node *node, int aidx);
int anm_use_animations(struct anm_node *node, int aidx, int bidx, float t);

/* returns the requested current animation index, which can be 0 or 1 */
int anm_get_active_animation_index(struct anm_node *node, int which);
/* returns the requested current animation, which can be 0 or 1 */
struct anm_animation *anm_get_active_animation(struct anm_node *node, int which);
float anm_get_active_animation_mix(struct anm_node *node);

int anm_get_animation_count(struct anm_node *node);

/* add/remove an animation to the specified node */
int anm_add_node_animation(struct anm_node *node);
int anm_remove_node_animation(struct anm_node *node, int idx);

/* add/remove an animation to the specified node and all it's descendants */
int anm_add_animation(struct anm_node *node);
int anm_remove_animation(struct anm_node *node, int idx);

struct anm_animation *anm_get_animation(struct anm_node *node, int idx);
struct anm_animation *anm_get_animation_by_name(struct anm_node *node, const char *name);

int anm_find_animation(struct anm_node *node, const char *name);

/* set the interpolator for the (first) currently active animation */
void anm_set_interpolator(struct anm_node *node, enum anm_interpolator in);
/* set the extrapolator for the (first) currently active animation */
void anm_set_extrapolator(struct anm_node *node, enum anm_extrapolator ex);

/* set the name of the currently active animation of this node only */
void anm_set_node_active_animation_name(struct anm_node *node, const char *name);
/* recursively set the name of the currently active animation for this node
 * and all it's descendants */
void anm_set_active_animation_name(struct anm_node *node, const char *name);
/* get the name of the currently active animation of this node */
const char *anm_get_active_animation_name(struct anm_node *node);

void anm_set_position(struct anm_node *node, vec3_t pos, anm_time_t tm);
vec3_t anm_get_node_position(struct anm_node *node, anm_time_t tm);

void anm_set_rotation(struct anm_node *node, quat_t rot, anm_time_t tm);
quat_t anm_get_node_rotation(struct anm_node *node, anm_time_t tm);

void anm_set_scaling(struct anm_node *node, vec3_t scl, anm_time_t tm);
vec3_t anm_get_node_scaling(struct anm_node *node, anm_time_t tm);

/* these three return the full p/r/s taking hierarchy into account */
vec3_t anm_get_position(struct anm_node *node, anm_time_t tm);
quat_t anm_get_rotation(struct anm_node *node, anm_time_t tm);
vec3_t anm_get_scaling(struct anm_node *node, anm_time_t tm);

/* those return the start and end times of the whole tree */
anm_time_t anm_get_start_time(struct anm_node *node);
anm_time_t anm_get_end_time(struct anm_node *node);

/* these calculate the matrix and inverse matrix of this node alone */
void anm_get_node_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm);
void anm_get_node_inv_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm);

/* ---- top-down matrix calculation interface ---- */

/* calculate and set the matrix of this node */
void anm_eval_node(struct anm_node *node, anm_time_t tm);
/* calculate and set the matrix of this node and all its children recursively */
void anm_eval(struct anm_node *node, anm_time_t tm);


/* ---- bottom-up lazy matrix calculation interface ---- */

/* These calculate the matrix and inverse matrix of this node taking hierarchy
 * into account. The results are cached in thread-specific storage and returned
 * if there's no change in time or tracks from the last query...
 */
void anm_get_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm);
void anm_get_inv_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm);

#ifdef __cplusplus
}
#endif

#endif	/* LIBANIM_H_ */
