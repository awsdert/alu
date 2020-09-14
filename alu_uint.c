#include "alu.h"
int_t alu_uint_cmp( alu_t *alu, alu_uint_t num, alu_uint_t val, size_t *bit )
{
	return alu_cmp( alu, num, val, bit );
}

int_t alu_uint_neg( alu_t *alu, alu_uint_t num )
{
	return alu_neg( alu, num );
}

int_t alu_uint_not( alu_t *alu, alu_uint_t num )
{
	return alu_not( alu, num );
}

int_t alu_uint_and( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	return alu_and( alu, num, val );
}

int_t alu_uint__or( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	return alu__or( alu, num, val );
}

int_t alu_uint_xor( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	return alu_xor( alu, num, val );
}

int_t alu_uint_shl( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_shl( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_uint_shr( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_shr( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_uint_rol( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_rol( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_uint_ror( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_ror( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_uint_inc( alu_t *alu, alu_uint_t num )
{
	return alu_inc( alu, num );
}

int_t alu_uint_add( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	return alu_add( alu, num, val );
}

int_t alu_uint_mul( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_mul( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_uint_dec( alu_t *alu, alu_uint_t num )
{
	return alu_dec( alu, num );
}

int_t alu_uint_sub( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	return alu_sub( alu, num, val );
}

int_t alu_uint_divide( alu_t *alu, alu_uint_t num, alu_uint_t val, alu_uint_t rem )
{
	return alu_divide( alu, num, val, rem );
}

int_t alu_uint_div( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_mul( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_uint_rem( alu_t *alu, alu_uint_t num, alu_uint_t val )
{
	int ret = 0;
	alu_uint_t tmp;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, 0 );
		alu_reg_init( alu, _val, val, 0 );
		alu_reg_init( alu, _tmp, tmp, 0 );
		
		ret = alu_reg_divide( alu, _num, _val, _tmp );
		alu_reg_copy( alu, _num, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
	}
	
	return ret;
}

int_t alu_str2uint( alu_t *alu, alu_src_t src, alu_uint_t dst, alu_base_t base )
{
	alu_reg_t _dst;
	
	alu_reg_init( alu, _dst, dst, 0 );
	
	return alu_str2reg( alu, src, _dst, base );
}

int_t alu_uint2str
(
	alu_t *alu
	, alu_dst_t dst
	, alu_uint_t src
	, alu_base_t base
)
{	
	alu_reg_t _src;
	
	alu_reg_init( alu, _src, src, 0 );
	
	return alu_reg2str( alu, dst, _src, base );
}
