#include "alu.h"
int_t alu_int_cmp( alu_t *alu, alu_int_t num, alu_int_t val, size_t *bit )
{
	int ret;
	alu_reg_t _num, _val;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		
		return alu_reg_cmp( alu, _num, _val, bit );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_neg( alu_t *alu, alu_int_t num )
{
	alu_reg_t _num = {0};
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	
	return alu_reg_neg( alu, _num );
}

int_t alu_int_not( alu_t *alu, alu_int_t num )
{
	alu_reg_t _num = {0};
	
	alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
	
	return alu_reg_not( alu, _num );
}

int_t alu_int_and( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	alu_reg_t _num, _val;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		
		return alu_reg_and( alu, _num, _val );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int__or( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	alu_reg_t _num, _val;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		
		return alu_reg__or( alu, _num, _val );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_xor( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	alu_reg_t _num, _val;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		
		return alu_reg_xor( alu, _num, _val );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_shl( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
		
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		alu_reg_init( alu, _tmp, tmp, ALU_INFO__SIGN );
		
		ret = alu_reg_shl( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_int_shr( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
		
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		alu_reg_init( alu, _tmp, tmp, ALU_INFO__SIGN );
		
		ret = alu_reg_shr( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_int_rol( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
		
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		alu_reg_init( alu, _tmp, tmp, ALU_INFO__SIGN );
		
		ret = alu_reg_rol( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_int_ror( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
		
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		alu_reg_init( alu, _tmp, tmp, ALU_INFO__SIGN );
		
		ret = alu_reg_ror( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_int_inc( alu_t *alu, alu_int_t num )
{
	int ret;
	alu_reg_t _num;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		
		return alu_reg_inc( alu, _num );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_add( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	alu_reg_t _num, _val;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		
		return alu_reg_add( alu, _num, _val );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_mul( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	alu_reg_t _num, _val, _tmp;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
		
	if ( ret == 0 )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		alu_reg_init( alu, _tmp, tmp, ALU_INFO__SIGN );
		
		ret = alu_reg_mul( alu, _num, _val, _tmp );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_int_dec( alu_t *alu, alu_int_t num )
{
	int ret;
	alu_reg_t _num;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		
		return alu_reg_dec( alu, _num );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_sub( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	alu_reg_t _num, _val;
	
	if ( alu )
	{
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		
		return alu_reg_sub( alu, _num, _val );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_divide( alu_t *alu, alu_int_t num, alu_int_t val, alu_int_t rem )
{
	int ret;
	alu_reg_t _num, _val, _rem;
	
	if ( alu )
	{
		
		alu_reg_init( alu, _num, num, ALU_INFO__SIGN );
		alu_reg_init( alu, _val, val, ALU_INFO__SIGN );
		alu_reg_init( alu, _rem, rem, ALU_INFO__SIGN );
		
		return alu_reg_div( alu, _num, _val, _rem );
	}
	
	ret = EADDRNOTAVAIL;
	alu_error( ret );
	return ret;
}

int_t alu_int_div( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
		
	if ( ret == 0 )
	{
		ret = alu_int_divide( alu, num, val, tmp );
		
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alu_int_rem( alu_t *alu, alu_int_t num, alu_int_t val )
{
	int ret;
	uint_t tmp = -1;
	
	ret = alu_get_reg_node( alu, &tmp, 0 );
	
	if ( ret == 0 )
	{
		ret = alu_int_divide( alu, num, val, tmp );
		
		alu_copy( alu, num, tmp );
		alu_rem_reg_node( alu, &tmp );
		return ret;
	}

	alu_error( ret );
	return ret;
}

int_t alu_str2int( alu_t *alu, alu_src_t src, alu_int_t dst, alu_base_t base )
{
	alu_reg_t tmp = {0};
	
	alu_reg_init( alu, tmp, dst, ALU_INFO__SIGN );
	
	return alu_str2reg( alu, src, tmp, base );
}

int_t alu_int2str( alu_t *alu, alu_dst_t dst, alu_int_t src, alu_base_t base )
{
	int ret = 0;
	alu_reg_t tmp = {0};
	
	alu_reg_init( alu, tmp, src, ALU_INFO__SIGN );
	
	ret = alu_reg2str( alu, dst, tmp, base );
	
	if ( ret != 0 )
		alu_error(ret);
	
	return ret;
}
