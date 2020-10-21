#ifndef INC_ALUM_H
# define INC_ALUM_H

# include "_.h"

typedef struct alum {
	int fault;
	void *block;
	size_t given, taken;
} alum_t;

/** @brief malloc/realloc/free wrapper
 * @param mem Where you want allocations to go
 * @param want How many bytes you want (0 to free)
 * @param dir Which direction to restrain allocation to
 * 	positive value = expansion only -
 * 		will not shrink memory if wanted amount is smaller than current,
 * 		will return success in that case. Will also clear any memory,
 * 		between that wanted memory end point and the end point of
 * 		current memory
 * 	negative value = shrinkage only -
 * 		will fail if wanted amount is larger than current
 * 	0 = normal realloc behaviour
 * @return 0 on success, errno.h value on failure
 * **/
void* alum( alum_t *mem, size_t want, int_t dir );
# define alum_expand( MEM, WANT ) alum( MEM, WANT, 1 )
# define alum_shrink( MEM, WANT ) alum( MEM, WANT, -1 )
# define alum_release( MEM ) (void)alum( MEM, 0, -1 )

#endif
