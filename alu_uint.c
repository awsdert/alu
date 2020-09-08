#include "alu.h"
int alu_uint_cmp( alu_t *alu, alu_uint_t num, alu_uint_t val, int *cmp, size_t *bit )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 2, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	*cmp = alu_reg_cmp( *alu, _num, _val, bit );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_uint_neg( alu_t *alu, alu_uint_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	alu_reg_neg( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_uint_not( alu_t *alu, alu_uint_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	alu_reg_not( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_uint_and( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 2, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_and( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_uint__or( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 2, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg__or( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_uint_xor( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_xor( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_uint_shl( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_shl( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_shr( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_shr( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_rol( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_rol( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_ror( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_ror( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_inc( alu_t *alu, alu_uint_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	ret = alu_reg_inc( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_uint_add( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 2, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_add( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_uint_mul( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_mul( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_dec( alu_t *alu, alu_uint_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	ret = alu_reg_dec( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_uint_sub( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 2, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_sub( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_uint_divide( alu_t *alu, alu_uint_t num, alu_uint_t val, alu_uint_t rem )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _rem;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_rem = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_divide( *alu, _num, _val, _rem );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	(void)alu_mov( alu, (uintptr_t)&rem, _rem.node );
	
	(void)alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_div( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_div( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	(void)alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_uint_rem( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_register_t regv[3], _num, _val, _tmp;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	ret = alu_get_regv( alu, regv, 3, HIGHEST( n, v ) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num = regv[0];
	_val = regv[1];
	_tmp = regv[2];
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_rem( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _tmp.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_str2uint
(
	alu_t *alu,
	void *src,
	alu_uint_t dst,
	char32_t digsep,
	alu_func_rdChar32_t nextchar,
	long *nextpos,
	bool lowercase
)
{
	int ret = 0;
	alu_register_t _dst = {0}, *DST;
	
	ret = alu_get_reg( alu, &_dst, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	ret = alu_lit2reg(
		alu,
		src,
		_dst,
		digsep,
		nextchar,
		nextpos,
		lowercase
	);
	DST = alu->regv + _dst.node;
	DST->info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, (uintptr_t)&dst, _dst.node );
	
	(void)alu_rem_reg( *alu, _dst );
	
	return ret;
}

int alu_uint2str
(
	alu_t *alu,
	void *dst,
	alu_uint_t src,
	char32_t digsep,
	alu_func_wrChar32_t nextchar,
	alu_func_flipstr_t flipstr,
	size_t base,
	bool lowercase,
	bool noPfx,
	bool noSign
)
{
	int ret = 0;
	alu_register_t _src = {0}, *SRC;
	
	ret = alu_get_reg( alu, &_src, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	SRC = alu->regv + _src.node;
	SRC->info |= ALU_INFO__SIGN;
	
	ret = alu_reg2str(
		alu,
		dst,
		_src,
		digsep,
		nextchar,
		flipstr,
		base,
		lowercase,
		noPfx,
		noSign
	);
	
	(void)alu_mov( alu, (uintptr_t)&src, _src.node );
	
	alu_rem_reg( *alu, _src );
	
	return ret;
}
