#include <alu/alur.h>

alub_t alur_final_one( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{	
		alup_init_register( alu, NUM, 0 );
		
		return alup_final_one( &(NUM.alup) );
	}
	else
	{
		alub_t n = {0};
		
		(void)alu_err_null_ptr("alu");
		
		return n;
	}
}

int_t alur_mov_int2int( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{
		int_t ret;
		
		alup_init_register( alu, DST, 0 );
		alup_init_register( alu, SRC, 0 );
		
		ret = alu->block.fault = alup_mov_int2int( &(DST.alup), &(SRC.alup) );
		if ( ret == 0 || ret == EOVERFLOW )
			return 0;
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_int2flt( alu_t *alu, alur_t DST, alur_t SRC )
{	
	if ( alu )
	{	
		alup_init_register( alu, DST, 0 );
		alup_init_register( alu, SRC, 0 );
		
		return alup_mov_int2flt( &(DST.alup), &(SRC.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_flt2int( alu_t *alu, alur_t DST, alur_t SRC )
{	
	if ( alu )
	{	
		alup_init_register( alu, DST, 0 );
		alup_init_register( alu, SRC, 0 );
		
		return alup_mov_flt2int( &(DST.alup), &(SRC.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_flt2flt( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{	
		alup_init_register( alu, DST, 0 );
		alup_init_register( alu, SRC, 0 );
		
		return alup_mov_flt2flt( &(DST.alup), &(SRC.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t	alur_mov( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{	
		alup_init_register( alu, DST, 0 );
		alup_init_register( alu, SRC, 0 );
		
		return alup_mov( &(DST.alup), &(SRC.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_cmp(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{	
	if ( alu )
	{
		int ret, a, b;
		
		NUM.node %= alu_used(alu);
		VAL.node %= alu_used(alu);
			
		if ( !NUM.node || !VAL.node )
		{
			alu_error( EDOM );
			
			if ( !NUM.node ) alu_puts( "NUM.node was 0" );
			
			if ( !VAL.node ) alu_puts( "VAL.node was 0" );
			
			a = !!(NUM.node);
			b = !!(VAL.node);
			
			return a - b;
		}
		
		/* Check sign is same */
		
		a = alur_below0( alu, NUM );
		b = alur_below0( alu, VAL );
		
		ret = -(a - b);
		
		if ( ret )
			return ret;
			
		if ( alur_floating( NUM ) )
		{
			if ( alur_floating( VAL ) )
			{
				alur_t NEXP, VEXP, NMAN, VMAN;
				
				alur_init_exponent( NUM, NEXP );
				alur_init_exponent( VAL, VEXP );
				
				ret = alur_cmp( alu, NEXP, VEXP );
				
				if ( ret )
					return ret;
				
				alur_init_mantissa( NUM, NMAN );
				alur_init_mantissa( VAL, VMAN );
				
				return alur_cmp( alu, NMAN, VMAN );
			}
			
			alu->block.fault = EINVAL;
			alu_error( EINVAL );
			return 0;
		}
		else
		{	
			alup_init_register( alu, NUM, 0 );
			alup_init_register( alu, VAL, 0 );
			
			if ( a )
			{
				alur_neg( alu, NUM );
				alur_neg( alu, VAL );
			}
			
			ret = alup_cmp_int2int( &(NUM.alup), &(VAL.alup) );
			
			if ( a )
			{
				alur_neg( alu, NUM );
				alur_neg( alu, VAL );
				/* Example: -3 vs -2 became 3 vs 2,
				 * 3 vs 2 returns 1 but -3 vs -2 should
				 * return -1 so we flip the result to
				 * match */
				return -ret;
			}
			
			return ret;
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_not( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{
		alup_not( &(NUM.alup) );
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alur_inc( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{	
		alup_init_register( alu, NUM, 0 );
		return alup_inc( &(NUM.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_dec( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{
		alup_init_register( alu, NUM, 0 );
		return alup_dec( &(NUM.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_match_exponents( alu_t *alu, uint_t num, uint_t val )
{
	if ( alu )
	{		
		alu->block.fault = alup_match_exponents
		(
			alu_Ndata( alu, num )
			, alu_Ndata( alu, val )
			, alu_Nsize( alu )
		);
		
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__add(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t cpy
	, uint_t tmp
)
{	
	if ( alu )
	{	
		int_t ret;
		
		alup_init_register( alu, NUM, 0 );
		alup_init_register( alu, VAL, 0 );
		
		ret = alup__add
		(
			&(NUM.alup)
			, &(VAL.alup)
			, alu_Ndata(alu,cpy)
			, alu_Ndata(alu,tmp)
		);
		
		alu->block.fault = ret;
		
		switch ( ret )
		{
			case 0: case ENODATA: case EOVERFLOW: case ERANGE:
				ret = 0;
				break;
			default:
				alu_error( ret );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_add(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	uint_t nodes[2];
	int_t ret = alur_get_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alur__add( alu, NUM, VAL, nodes[0], nodes[1] );
		alur_rem_nodes( alu, nodes, 2 );
	}
	else
	{
		alu_error(ret);
	}
	
	return ret;
}

int_t alur__sub( alu_t *alu, alur_t NUM, alur_t VAL, uint_t cpy, uint_t tmp )
{
	if ( alu )
	{
		int_t ret;
		
		alup_init_register( alu, NUM, 0 );
		alup_init_register( alu, VAL, 0 );
		
		ret = alup__sub
		(
			&(NUM.alup)
			, &(VAL.alup)
			, alu_Ndata(alu, cpy)
			, alu_Ndata(alu, tmp)
		);
		
		alu->block.fault = ret;
		
		switch ( ret )
		{
			case 0: case ENODATA: case EOVERFLOW: case ERANGE:
				ret = 0;
				break;
			default:
				alu_error( ret );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_sub( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	uint_t nodes[2];
	int_t ret = alur_get_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{	
		ret = alur__sub( alu, NUM, VAL, nodes[0], nodes[1] );
		alur_rem_nodes( alu, nodes, 2 );
	}
	else
	{
		alu_error(ret);
	}
	
	return ret;
}

int_t alur__shl( alu_t *alu, alur_t NUM, size_t by )
{	
	if ( alu )
	{
		alup_init_register( alu, NUM, 0 );	
		return alup__shl( &(NUM.alup), by );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__shr( alu_t *alu, alur_t NUM, size_t by )
{	
	if ( alu )
	{
		alup_init_register( alu, NUM, 0 );
		return alup__shr( &(NUM.alup), by );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__shift
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, func_alur__shift_t _shift
)
{
	int_t ret;
	
	if ( alur_floating( VAL ) )
	{
		ret = EINVAL;
		alu_error(ret);
		alu_puts( "Cannot shift by a float");
		return ret;
	}
	
	if ( alu )
	{
		int cmp;
		alup_t _TMP;
		size_t by;
		
		alup_init_register( alu, VAL, 0 );
		alup_init_unsigned( _TMP, &by, bitsof(size_t) );
		
		by = NUM.alup.leng;
		
		cmp = alup_cmp_int2int( &(VAL.alup), &_TMP );
		
		if ( cmp < 0 )
		{
			alup_mov_int2int( &_TMP, &(VAL.alup) );
			ret = _shift( alu, NUM, by );
		}
		else
		{
			ret = _shift( alu, NUM, -1 );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}


int_t alur__mul
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t cpy
	, uint_t tmp
)
{
	if ( alu )
	{		
		int_t ret;
		
		alup_init_register( alu, NUM, 0 );
		alup_init_register( alu, VAL, 0 );
		
		ret = alup__mul
		(
			&(NUM.alup)
			, &(VAL.alup)
			, alu_Ndata( alu, cpy )
			, alu_Ndata( alu, tmp )
		);
		
		alu->block.fault = ret;
		
		switch ( ret )
		{
			case 0: case ENODATA: case EOVERFLOW: case ERANGE:
				ret = 0;
				break;
			default:
				alu_error( ret );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_mul
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	int ret;
	uint_t nodes[2];
	
	ret = alur_get_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alur__mul( alu, NUM, VAL, nodes[0], nodes[1] );
		alur_rem_nodes( alu, nodes, 2 );
		
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alur_neg( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{	
		alup_init_register( alu, NUM, 0 );
		return alup_neg( &(NUM.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__div( alu_t *alu, alur_t NUM, alur_t VAL, uint_t rem, uint_t tmp )
{	
	if ( alu )
	{	
		int_t ret;
		
		alup_init_register( alu, NUM, 0 );
		alup_init_register( alu, VAL, 0 );
		
		ret = alup__div
		(
			&(NUM.alup)
			, &(VAL.alup)
			, alu_Ndata( alu, rem )
			, alu_Ndata( alu, tmp )
		);
		
		alu->block.fault = ret;
		
		switch ( ret )
		{
			case 0: case ENODATA: case EOVERFLOW: case ERANGE:
				ret = 0;
				break;
			default:
				alu_error( ret );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_div
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	int ret = 0;
	uint_t nodes[2] = {0};
	
	ret = alur_get_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{	
		ret = alur__div( alu, NUM, VAL, nodes[0], nodes[1] );
		alur_rem_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alur_rem
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
)
{
	int_t ret;
	uint_t nodes[2];
	
	ret = alur_get_nodes( alu, nodes, 2, 0 );
	
	if ( ret == 0 )
	{
		ret = alur__div( alu, NUM, VAL, nodes[0], nodes[1] );
		
		if ( ret == 0 )
		{
			alur_t REM = NUM;
			
			REM.node = nodes[0];
			alup_init_register( alu, REM, 0 );
			ret = alur_mov( alu, NUM, REM );
			
			if ( ret != 0 )
			{
				alu_error(ret);
				alu->block.fault = ret;
			}
		}
		
		return ret;
		
		alur_rem_nodes( alu, nodes, 2 );
		return ret;
	}
	
	alu_error( ret );
	return ret;
}

int_t alur__rol( alu_t *alu, alur_t NUM, uint_t tmp, size_t by )
{
	if ( alu )
	{
		NUM.node %= alu_used( alu );
		tmp %= alu_used( alu );
		
		if ( !NUM.node || !tmp )
		{
			int_t ret = EDOM;
			
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts("NUM.node was 0!");
			
			if ( !tmp ) alu_puts("tmp was 0!");
			
			return ret;
		}
		else
		{
			alup_init_register( alu, NUM, 0 );
			
			alup__rol( &(NUM.alup), alu_Ndata( alu, tmp ), by );
			return 0;
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__ror( alu_t *alu, alur_t NUM, uint_t tmp, size_t by )
{	
	if ( alu )
	{
		alup_init_register( alu, NUM, 0 );	
		return alup__ror( &(NUM.alup), alu_Ndata( alu, tmp ), by );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__rotate
(
	alu_t *alu
	, alur_t NUM
	, alur_t VAL
	, uint_t tmp
	, func_alur__rotate_t _rotate
)
{
	int_t ret;
	
	if ( alur_floating( VAL ) )
	{
		ret = EINVAL;
		alu_error(ret);
		alu_puts("Cannot rotate by a float");
		return ret;
	}
	
	if ( alu )
	{
		int cmp;
		alup_t _TMP;
		size_t by;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		ret = IFTRUE( !NUM.node || !VAL.node, EINVAL );
		
		if ( ret )
		{
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts("NUM.node was 0!");
			
			if ( !VAL.node ) alu_puts("VAL.node was 0!");
			
			return ret;
		}
		
		alup_init_register( alu, VAL, 0 );
		alup_init_unsigned( _TMP, &by, bitsof(size_t) );
		
		by = NUM.alup.leng;
		
		cmp = alup_cmp_int2int( &(VAL.alup), &_TMP );
		
		if ( cmp < 0 )
		{
			alup_mov_int2int( &_TMP, &(VAL.alup) );
			ret = _rotate( alu, NUM, tmp, by );
		}
		else
		{
			ret = _rotate( alu, NUM, tmp, -1 );
		}
		
		return ret;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_and( alu_t *alu, alur_t NUM, alur_t VAL )
{
	if ( alu )
	{	
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		if ( !NUM.node || !VAL.node )
		{
			int_t ret = EDOM;
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts("!NUM.node was 0!");
			
			if ( !VAL.node ) alu_puts("!VAL.node was 0!");
			
			return ret;
		}
		else
		{
			alup_init_register( alu, NUM, 0 );
			alup_init_register( alu, VAL, 0 );
			
			return alup_and( &(NUM.alup), &(VAL.alup) );
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__or( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{
		alup_init_register( alu, NUM, 0 );
		alup_init_register( alu, VAL, 0 );
		return alup__or( &(NUM.alup), &(VAL.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_xor( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{	
		alup_init_register( alu, NUM, 0 );
		alup_init_register( alu, VAL, 0 );
		return alup_xor( &(NUM.alup), &(VAL.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

