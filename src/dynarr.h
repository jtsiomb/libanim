/*
libanim - hierarchical keyframe animation library
Copyright (C) 2012-2014 John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DYNARR_H_
#define DYNARR_H_

void *anm_dynarr_alloc(int elem, int szelem);
void anm_dynarr_free(void *da);
void *anm_dynarr_resize(void *da, int elem);

int anm_dynarr_empty(void *da);
int anm_dynarr_size(void *da);

/* stack semantics */
void *anm_dynarr_push(void *da, void *item);
void *anm_dynarr_pop(void *da);


#endif	/* DYNARR_H_ */
