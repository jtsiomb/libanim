/* An animation track defines the values of a single scalar over time
 * and supports various interpolation and extrapolation modes.
 */
#ifndef LIBANIM_TRACK_H_
#define LIBANIM_TRACK_H_

#include <limits.h>
#include "config.h"

enum anm_interpolator {
	ANM_INTERP_STEP,
	ANM_INTERP_LINEAR,
	ANM_INTERP_CUBIC
};

enum anm_extrapolator {
	ANM_EXTRAP_EXTEND,	/* extend to infinity */
	ANM_EXTRAP_CLAMP,	/* clamp to last value */
	ANM_EXTRAP_REPEAT	/* repeat motion */
};

typedef long anm_time_t;
#define ANM_TIME_INVAL	LONG_MIN

#define ANM_SEC2TM(x)	((anm_time_t)((x) * 1000))
#define ANM_MSEC2TM(x)	((anm_time_t)(x))
#define ANM_TM2SEC(x)	((x) / 1000.0)
#define ANM_TM2MSEC(x)	(x)

struct anm_keyframe {
	anm_time_t time;
	float val;
};

struct anm_track {
	char *name;
	int count;
	struct anm_keyframe *keys;

	float def_val;

	enum anm_interpolator interp;
	enum anm_extrapolator extrap;
};

#ifdef __cplusplus
extern "C" {
#endif

/* track constructor and destructor */
int anm_init_track(struct anm_track *track);
void anm_destroy_track(struct anm_track *track);

/* helper functions that use anm_init_track and anm_destroy_track internally */
struct anm_track *anm_create_track(void);
void anm_free_track(struct anm_track *track);

int anm_set_track_name(struct anm_track *track, const char *name);
const char *anm_get_track_name(struct anm_track *track);

void anm_set_track_interpolator(struct anm_track *track, enum anm_interpolator in);
void anm_set_track_extrapolator(struct anm_track *track, enum anm_extrapolator ex);

anm_time_t anm_remap_time(struct anm_track *track, anm_time_t tm, anm_time_t start, anm_time_t end);

void anm_set_track_default(struct anm_track *track, float def);

/* set or update a keyframe */
int anm_set_keyframe(struct anm_track *track, struct anm_keyframe *key);

/* get the idx-th keyframe, returns null if it doesn't exist */
struct anm_keyframe *anm_get_keyframe(struct anm_track *track, int idx);

/* Finds the 0-based index of the intra-keyframe interval which corresponds
 * to the specified time. If the time falls exactly onto the N-th keyframe
 * the function returns N.
 *
 * Special cases:
 * - if the time is before the first keyframe -1 is returned.
 * - if the time is after the last keyframe, the index of the last keyframe
 *   is returned.
 */
int anm_get_key_interval(struct anm_track *track, anm_time_t tm);

int anm_set_value(struct anm_track *track, anm_time_t tm, float val);

/* evaluates and returns the value of the track for a particular time */
float anm_get_value(struct anm_track *track, anm_time_t tm);

#ifdef __cplusplus
}
#endif


#endif	/* LIBANIM_TRACK_H_ */
