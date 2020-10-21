#ifndef INC_ALUV_H
# define INC_ALUV_H

# include "alum.h"

typedef struct aluv
{
	size_t Nsize;
	uint_t taken, given;
	alum_t block;
	void **ref;
} aluv_t;

typedef uint_t alu_int_t, alu_uint_t, alu_fpn_t;

void* aluv( aluv_t *vec, uint_t want, size_t Nsize, int dir );
# define aluv_expand( VEC, WANT, PERN ) aluv( VEC, WANT, PERN, 1 )
# define aluv_shrink( VEC, WANT, PERN ) aluv( VEC, WANT, PERN, -1 )
# define aluv_release( VEC, PERN ) (void)aluv( VEC, 0, PERN, -1 )

#endif
