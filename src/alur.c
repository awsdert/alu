#include <alu/alur.h>

alub_t alur_end_bit( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		
		return alup_end_bit( _NUM );
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
		alup_t _DST, _SRC;
		
		alup_init_register( alu, _DST, DST );
		alup_init_register( alu, _SRC, SRC );
		
		ret = alu->block.fault = alup_mov_int2int( _DST, _SRC );
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
		alup_t _DST, _SRC;
		
		alup_init_register( alu, _DST, DST );
		alup_init_register( alu, _SRC, SRC );
		
		return alup_mov_int2flt( _DST, _SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_flt2int( alu_t *alu, alur_t DST, alur_t SRC )
{	
	if ( alu )
	{
		alup_t _DST, _SRC;
		
		alup_init_register( alu, _DST, DST );
		alup_init_register( alu, _SRC, SRC );
		
		return alup_mov_flt2int( _DST, _SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int alur_mov_flt2flt( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{
		alup_t _DST, _SRC;
		
		alup_init_register( alu, _DST, DST );
		alup_init_register( alu, _SRC, SRC );
		
		return alup_mov_flt2flt( _DST, _SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int_t	alur_mov( alu_t *alu, alur_t DST, alur_t SRC )
{
	if ( alu )
	{
		alup_t _DST, _SRC;
		
		alup_init_register( alu, _DST, DST );
		alup_init_register( alu, _SRC, SRC );
		
		return alup_mov( _DST, _SRC );
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
			alup_t _NUM, _VAL;
			
			alup_init_register( alu, _NUM, NUM );
			alup_init_register( alu, _VAL, VAL );
			
			if ( a )
			{
				alur_neg( alu, NUM );
				alur_neg( alu, VAL );
			}
			
			ret = alup_cmp_int2int( _NUM, _VAL );
			
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
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		alup_not( _NUM );
		
		return 0;
	}
	
	return alu_err_null_ptr( "alu" );
}

int_t alur_inc( alu_t *alu, alur_t NUM )
{
	if ( alu )
	{
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		
		return alup_inc( _NUM );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_dec( alu_t *alu, alur_t NUM )
{
	alub_t n;
	void *part;
	
	NUM.node %= alu_used( alu );
	part = alur_data( alu, NUM );
	
	n = alub( part, NUM.from );
	
	for ( ; n.bit < NUM.upto; alub_inc(&n) )
	{
		bool is1 = !!(*(n.ptr) & n.mask);
		*(n.ptr) &= ~(n.mask);
		*(n.ptr) |= IFTRUE( !is1, n.mask );
		NUM.upto = IFTRUE( !is1, NUM.upto );
	}
	
	return EITHER( NUM.upto, EOVERFLOW, 0 );
}

int_t alur_match_exponents( alu_t *alu, uint_t num, uint_t val )
{
	if ( alu )
	{		
		alu->block.fault = alup_match_exponents
		(
			alu_data( alu, num )
			, alu_data( alu, val )
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
		int ret;
		//alub_t n, v;
		alup_t _NUM, _VAL;
		
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		cpy %= alu_used(alu);
		tmp %= alu_used(alu);
		
		ret = IFTRUE( !NUM.node || !VAL.node || !cpy || !tmp, EINVAL );
			
		if ( ret )
		{
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts( "NUM.node was 0!" );
			
			if ( !VAL.node ) alu_puts( "VAL.node was 0!" );
			
			if ( !cpy ) alu_puts( "cpy was 0!" );
			
			if ( !tmp ) alu_puts( "tmp was 0!" );
			
			return ret;
		}
		
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		if ( alur_floating( NUM ) || alur_floating( VAL ) )
		{
			alup_t _CPY, _TMP, _CMAN, _TMAN;
			size_t exp, cexp, texp;
			bool_t truncated = false;
			
			alup_init_floating( _CPY, alu_data(alu, cpy), alu_Nsize(alu) );
			alup_init_floating( _TMP, alu_data(alu, tmp), alu_Nsize(alu) );
			
			alup_init_mantissa( _CPY, _CMAN );
			alup_init_mantissa( _TMP, _TMAN );
			
			alup_mov( _CPY, _NUM );
			alup_mov( _TMP, _VAL );
			
			cexp = alup_get_exponent( _CPY );
			texp = alup_get_exponent( _TMP );
			
			ret = alup_match_exponents
			(
				_CPY.data
				, _TMP.data
				, alu_Nsize(alu)
			);
			
			truncated = (ret == ERANGE);
			
			exp = alup_get_exponent( _CPY );
			
			ret = alup__add_int2int( _CMAN, _TMAN );
			
			if ( cexp || texp )
			{
				if ( (cexp == texp) || ret == EOVERFLOW ) ++exp;
			}
			
			(void)alup_set_exponent( _CPY, exp );
			
			ret = alup_mov( _NUM, _CPY );
			
			return IFTRUE( truncated || ret == ERANGE, ERANGE );
		}
		return alup__add_int2int( _NUM, _VAL );
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
		
		switch (ret)
		{
		case 0: case ENODATA: case EOVERFLOW: case ERANGE:
			alu->block.fault = ret;
			ret = 0;
			break;
		default:
			alu_error(ret);
		}
	}
	else
	{
		alu_error(ret);
	}
	
	return ret;
}


int_t alur_sub( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{
		alup_t _NUM, _VAL;
		
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		return alup__sub_int2int( _NUM, _VAL );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__shl( alu_t *alu, alur_t NUM, size_t by )
{	
	if ( alu )
	{
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		
		return alup__shl( _NUM, by );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__shr( alu_t *alu, alur_t NUM, size_t by )
{	
	if ( alu )
	{
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		
		return alup__shr( _NUM, by );
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
		alup_t _VAL, _TMP;
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
		
		alup_init_register( alu, _VAL, VAL );
		alup_init_unsigned( _TMP, &by, sizeof(size_t) );
		
		by = NUM.upto - NUM.from;
		
		cmp = alup_cmp_int2int( _VAL, _TMP );
		
		if ( cmp < 0 )
		{
			alup_mov_int2int( _TMP, _VAL );
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
		alup_t _NUM, _VAL;
			
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		if ( alur_floating(VAL) || alur_floating(NUM) )
		{
			/* Not yet supported */
			alup_t _CPY, _TMP;
			
			alup_init_floating( _CPY, alu_data( alu, cpy ), alu_Nsize( alu ) );
			alup_init_floating( _TMP, alu_data( alu, tmp ), alu_Nsize( alu ) );
			
			if ( alur_floating(NUM) )
				alup_mov_flt2flt( _CPY, _NUM );
			else
				alup_mov_int2flt( _CPY, _NUM );
				
			if ( alur_floating(VAL) )
				alup_mov_flt2flt( _TMP, _VAL );
			else
				alup_mov_int2flt( _TMP, _VAL );
			
			return ENOSYS;
		}
		else
		{
			return alup__mul_int2int( _NUM, _VAL, alu_data( alu, cpy ) );
		}
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
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		
		return alup_neg( _NUM );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__div( alu_t *alu, alur_t NUM, alur_t VAL, uint_t rem, uint_t tmp )
{	
	if ( alu )
	{
		alup_t _NUM, _VAL;
		
		alup_init_register( alu, _NUM, NUM );
		alup_init_register( alu, _VAL, VAL );
		
		return alup__div
		(
			_NUM
			, _VAL
			, alu_data( alu, rem )
			, alu_data( alu, tmp )
		);
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
		int tmpret;
		alur_t REM;
		
		if ( NUM.info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, REM, nodes[0] );
		}
		else
		{
			alur_init_unsigned( alu, REM, nodes[0] );
			REM.info = NUM.info;
		}
		
		ret = alur__div( alu, NUM, VAL, nodes[0], nodes[1] );
		
		switch ( ret )
		{
		case 0: case ENODATA: case EOVERFLOW:
			tmpret = ret;
			ret = alur_mov( alu, NUM, REM );
			if ( ret == 0 )
				ret = tmpret;
			break;
		default:
			alu_error(ret);
		}
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
			alup_t _NUM;
		
			alup_init_register( alu, _NUM, NUM );
			
			alup__rol( _NUM, alu_data( alu, tmp ), by );
			return 0;
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__ror( alu_t *alu, alur_t NUM, uint_t tmp, size_t by )
{	
	if ( alu )
	{
		alup_t _NUM;
		
		alup_init_register( alu, _NUM, NUM );
		
		return alup__ror( _NUM, alu_data( alu, tmp ), by );
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
		alup_t _VAL, _TMP;
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
		
		alup_init_register( alu, _VAL, VAL );
		alup_init_unsigned( _TMP, &by, sizeof(size_t) );
		
		by = NUM.upto - NUM.from;
		
		cmp = alup_cmp_int2int( _VAL, _TMP );
		
		if ( cmp < 0 )
		{
			alup_mov_int2int( _TMP, _VAL );
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
			alup_t _NUM, _VAL;
			
			alup_init_register( alu, _NUM, NUM );
			alup_init_register( alu, _VAL, VAL );
			
			return alup_and( _NUM, _VAL );
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur__or( alu_t *alu, alur_t NUM, alur_t VAL )
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
			alup_t _NUM, _VAL;
			
			alup_init_register( alu, _NUM, NUM );
			alup_init_register( alu, _VAL, VAL );
			
			return alup__or( _NUM, _VAL );
		}
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_xor( alu_t *alu, alur_t NUM, alur_t VAL )
{	
	if ( alu )
	{	
		NUM.node %= alu_used( alu );
		VAL.node %= alu_used( alu );
		
		if ( !NUM.node || !VAL.node )
		{
			int_t ret = EINVAL;
			
			alu_error( ret );
			
			if ( !NUM.node ) alu_puts("NUM.node was 0!");
		
			if ( !VAL.node ) alu_puts("VAL.node was 0!");
			
			return ret;
		}
		else
		{
			alup_t _NUM, _VAL;
			
			alup_init_register( alu, _NUM, NUM );
			alup_init_register( alu, _VAL, VAL );
		
			return alup_xor( _NUM, _VAL );
		}
	}
	
	return alu_err_null_ptr("alu");
}

