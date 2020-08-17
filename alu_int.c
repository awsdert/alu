#include "alu.h"
int alu_int_prep1( alu_t *alu, alu_int_t num, int *reg )
{
	int ret = 0, _reg = -1;
	size_t size = num.mem.bytes.upto;
	alu_reg_t *REG;
	
	if ( !alu || !reg )
	{
		ret = EDESTADDRREQ;
		alu_error(ret);
		return ret;
	}
	
	ret = alu_get_reg( alu, &_reg, size );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	REG = alu->regv + _reg;
	REG->info |= ALU_REG_F__SIGN;
	
	(void)memcpy( REG->part, num.mem.block, size );
	
	*reg = _reg;
	return 0;
}

int alu_int_prep2(
	alu_t *alu,
	alu_int_t num, alu_int_t val,
	int *nreg, int *vreg )
{
	int ret = 0, _nreg = -1, _vreg = -1;
	
	if ( !nreg || !vreg )
	{
		ret = EDESTADDRREQ;
		alu_error( ret );
		return ret;
	}
	
	ret = alu_int_prep1( alu, num, &_nreg );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_int_prep1( alu, val, &_vreg );
	
	if ( ret != 0 )
	{
		alu_rem_reg( alu, _nreg );
		alu_error( ret );
		return ret;
	}
	
	/* Register size may have changed */
	(void)alu_reset_reg( alu, _nreg, 0 );
	
	*nreg = _nreg;
	*vreg = _vreg;
	return ret;
}

int alu_int_prep3(
	alu_t *alu,
	alu_int_t num, alu_int_t val, alu_int_t rem,
	int *nreg, int *vreg, int *rreg )
{
	int ret = 0, _nreg = -1, _vreg = -1, _rreg = -1;
	
	if ( !nreg || !vreg || !rreg )
	{
		ret = EDESTADDRREQ;
		alu_error( ret );
		return ret;
	}
	
	ret = alu_int_prep2( alu, num, val, &_nreg, &_vreg );
	
	if ( ret != 0 )
	{
		alu_error( ret );
		return ret;
	}
	
	ret = alu_int_prep1( alu, rem, &_rreg );
	
	if ( ret != 0 )
	{
		alu_rem_reg( alu, _vreg );
		alu_error( ret );
		return ret;
	}
	
	/* Register size may have changed */
	(void)alu_reset_reg( alu, _nreg, 0 );
	(void)alu_reset_reg( alu, _vreg, 0 );
	
	*nreg = _nreg;
	*vreg = _vreg;
	*rreg = _rreg;
	return ret;
}

int alu_int_and( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_and( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int__or( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu__or( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_xor( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_xor( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_shl( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_shl( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_shr( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_shr( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_rol( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_rol( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_ror( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_ror( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_inc( alu_t *alu, alu_int_t num )
{
	int ret = 0, _num = -1;
	
	ret = alu_int_prep1( alu, num, &_num );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	ret = alu_inc( alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_add( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_add( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_mul( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_mul( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_dec( alu_t *alu, alu_int_t num )
{
	int ret = 0, _num = -1;
	
	ret = alu_int_prep1( alu, num, &_num );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	ret = alu_dec( alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_sub( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_sub( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_divide( alu_t *alu, alu_int_t num, alu_int_t val, alu_int_t rem )
{
	int ret = 0, _num = -1, _val = -1, _rem = -1;
	
	ret = alu_int_prep3( alu, num, val, rem, &_num, &_val, &_rem );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_divide( alu, _num, _val, _rem );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	(void)alu_mov( alu, (uintptr_t)&rem, _rem );
	
	(void)alu_rem_reg( alu, _rem );
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_div( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_div( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}

int alu_int_rem( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret = 0, _num = -1, _val = -1;
	
	ret = alu_int_prep2( alu, num, val, &_num, &_val );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num, (uintptr_t)&num );
	(void)alu_mov( alu, _val, (uintptr_t)&val );
	ret = alu_rem( alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num );
	
	(void)alu_rem_reg( alu, _val );
	(void)alu_rem_reg( alu, _num );
	
	return ret;
}
