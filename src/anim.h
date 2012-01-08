#ifndef LIBANIM_H_
#define LIBANIM_H_

#include "config.h"

#include <pthread.h>

#include <vmath/vmath.h>
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

struct anm_node {
	char *name;

	struct anm_track tracks[ANM_NUM_TRACKS];
	vec3_t pivot;

	/* matrix cache */
	struct mat_cache {
		mat4_t matrix, inv_matrix;
		anm_time_t time, inv_time;
		struct mat_cache *next;
	} *cache_list;
	pthread_key_t cache_key;
	pthread_mutex_t cache_list_lock;

	struct anm_node *parent;
	struct anm_node *child;
	struct anm_node *next;
};

#ifdef __cplusplus
extern "C" {
#endif

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

void anm_set_interpolator(struct anm_node *node, enum anm_interpolator in);
void anm_set_extrapolator(struct anm_node *node, enum anm_extrapolator ex);

/* link and unlink nodes with parent/child relations */
void anm_link_node(struct anm_node *parent, struct anm_node *child);
int anm_unlink_node(struct anm_node *parent, struct anm_node *child);

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

void anm_set_pivot(struct anm_node *node, vec3_t pivot);
vec3_t anm_get_pivot(struct anm_node *node);

void anm_get_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm);
void anm_get_inv_matrix(struct anm_node *node, mat4_t mat, anm_time_t tm);

/* those return the start and end times of the whole tree */
anm_time_t anm_get_start_time(struct anm_node *node);
anm_time_t anm_get_end_time(struct anm_node *node);

#ifdef __cplusplus
}
#endif

#endif	/* LIBANIM_H_ */
