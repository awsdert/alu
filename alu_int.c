#include "alu.h"
int alu_int_cmp( alu_t *alu, alu_int_t num, alu_int_t val, int *cmp, size_t *bit )
{
	int ret = 0;
	alu_register_t regv[2], _num, _val;
	size_t n = num.vec.mem.bytes.upto, v = val.vec.mem.bytes.upto;
	
	if ( cmp )
	{	
		ret = alu_get_regv( alu, regv, 2, HIGHEST( n, v ) );
		
		if ( ret != 0 )
		{
			alu_error(ret);
			return ret;
		}
		
		_num = regv[0];
		_val = regv[1];
		
		_num.info |= ALU_INFO__SIGN;
		_val.info |= ALU_INFO__SIGN;
		
		(void)alu_mov( alu, _num.node, (uintptr_t)&num );
		(void)alu_mov( alu, _val.node, (uintptr_t)&val );
		*cmp = alu_reg_cmp( *alu, _num, _val, bit );
		
		alu_rem_regv( *alu, regv, 2 );
		
		return 0;
	}
	
	return EDESTADDRREQ;
}

int alu_int_neg( alu_t *alu, alu_int_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	alu_reg_neg( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_int_not( alu_t *alu, alu_int_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	alu_reg_not( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_int_and( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_and( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_int__or( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg__or( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_int_xor( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_xor( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_int_shl( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_shl( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_shr( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_shr( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_rol( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_rol( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_ror( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	alu_reg_ror( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_inc( alu_t *alu, alu_int_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	ret = alu_reg_inc( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_int_add( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_add( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_int_mul( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	_tmp.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_mul( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_dec( alu_t *alu, alu_int_t num )
{
	int ret = 0;
	alu_register_t _num;
	
	ret = alu_get_reg( alu, &_num, num.vec.mem.bytes.upto );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	_num.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	ret = alu_reg_dec( *alu, _num );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_reg( *alu, _num );
	
	return ret;
}

int alu_int_sub( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_sub( *alu, _num, _val );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	alu_rem_regv( *alu, regv, 2 );
	
	return ret;
}

int alu_int_divide( alu_t *alu, alu_int_t num, alu_int_t val, alu_int_t rem )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	_rem.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_divide( *alu, _num, _val, _rem );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	(void)alu_mov( alu, (uintptr_t)&rem, _rem.node );
	
	(void)alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_div( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	_tmp.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_div( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _num.node );
	
	(void)alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_int_rem( alu_t *alu, alu_int_t num, alu_int_t val )
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
	
	_num.info |= ALU_INFO__SIGN;
	_val.info |= ALU_INFO__SIGN;
	_tmp.info |= ALU_INFO__SIGN;
	
	(void)alu_mov( alu, _num.node, (uintptr_t)&num );
	(void)alu_mov( alu, _val.node, (uintptr_t)&val );
	ret = alu_reg_rem( *alu, _num, _val, _tmp );
	(void)alu_mov( alu, (uintptr_t)&num, _tmp.node );
	
	alu_rem_regv( *alu, regv, 3 );
	
	return ret;
}

int alu_str2int( alu_t *alu, alu_src_t src, alu_int_t *dst, alu_base_t base )
{
	int ret = 0;
	alu_register_t tmp = {0}, *TMP;
	
	ret = alu_get_reg( alu, &tmp, 0 );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	ret = alu_str2reg( alu, src, tmp, base );
	tmp.node %= ALU_USED( *alu );
	TMP = alu->regv + tmp.node;
	tmp.info = (TMP->info |= ALU_INFO__SIGN);
	
	(void)alu_mov( alu, (uintptr_t)&dst, tmp.node );
	
	(void)alu_rem_reg( *alu, tmp );
	
	return ret;
}

int alu_int2str( alu_t *alu, alu_dst_t dst, alu_int_t src, alu_base_t base )
{
	int ret = 0;
	alu_register_t tmp = {0}, *TMP;
	
	ret = alu_get_reg( alu, &tmp, sizeof(size_t) );
	
	if ( ret != 0 )
	{
		alu_error(ret);
		return ret;
	}
	
	TMP = alu->regv + tmp.node;
	tmp.info = (TMP->info |= ALU_INFO__SIGN);
	(void)alu_mov( alu, tmp.node, (uintptr_t)&src );
	
	ret = alu_reg2str( alu, dst, tmp, base );
	
	alu_rem_reg( *alu, tmp );
	
	return ret;
}
