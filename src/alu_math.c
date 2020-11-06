#include "alu.h"
size_t alu_man_dig( size_t bits )
{
	return EITHER
	(
		bits < 6
		, 3
		, EITHER
		(
			bits < bitsof(float)
			, 5
			, EITHER
			(
				bits < bitsof(double)
				, FLT_MANT_DIG
				, EITHER
				(
					bits < bitsof(long double)
					, DBL_MANT_DIG
					, bits - 11
				)
			)
		)
	) - 1;
}

bool_t alur_below0( alu_t *alu, alur_t REG )
{
	if ( alu )
	{
		if ( REG.upto )
		{
			alub_t r = alub( (void*)alur_data( alu, REG ), REG.upto - 1 );
			
			return IFBOTH( alur___signed( REG ), *(r.ptr) & r.mask );
		}
		return 0;
	}
	
	(void)alu_err_null_ptr("alu");
	return 0;
}

int_t alur_get_exponent( alu_t *alu, alur_t SRC, size_t *dst )
{
	if ( dst )
	{
		alup_t _SRC;
		
		alup_init_register( alu, _SRC, SRC );
		
		*dst = alup_get_exponent( _SRC );
		
		return 0;
	}
	return alu_err_null_ptr("dst");
}

int_t alur_set_exponent( alu_t *alu, alur_t DST, size_t src )
{
	alup_t _DST;
		
	alup_init_register( alu, _DST, DST );
	
	return alup_set_exponent( _DST, src );
}

size_t alur_get_exponent_bias( alur_t SRC )
{
	alur_t EXP;
	size_t bias = UNIC_SIZE_C(~0);
	alur_init_exponent( SRC, EXP );
	bias <<= (EXP.upto - EXP.from) - 1;
	return ~bias;
}

int_t alu__op1
(
	alu_t *alu
	, uint_t num
	, uint_t info
	, func_alur_op1_t op1
)
{
	if ( alu )
	{
		alur_t NUM;
		
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, NUM, num );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			NUM.info = info;
		}
		
		return op1( alu, NUM );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu__op2
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t info
	, func_alur_op2_t op2
)
{
	if ( alu )
	{
		alur_t NUM, VAL;
	
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, NUM, num );
			alur_init_floating( alu, VAL, val );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			alur_init_unsigned( alu, VAL, val );
			NUM.info = info;
			VAL.info = info;
		}
		
		return op2( alu, NUM, VAL );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu__op4
(
	alu_t *alu
	, uint_t num
	, uint_t val
	, uint_t reg
	, uint_t tmp
	, uint_t info
	, func_alur_op4_t op4
)
{
	if ( alu )
	{
		alur_t NUM, VAL;
		
		if ( info & ALU_INFO_FLOAT )
		{
			alur_init_floating( alu, NUM, num );
			alur_init_floating( alu, VAL, val );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			alur_init_unsigned( alu, VAL, val );
			NUM.info = info;
			VAL.info = info;
		}
		
		return op4( alu, NUM, VAL, reg, tmp );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_set( alu_t *alu, alur_t NUM, bool fillwith )
{
	if ( alu )
	{
		alup_t _NUM;
		
		NUM.node %= alu_used( alu );
		
		if ( !NUM.node )
		{
			int_t ret = EDOM;
			alu_error( ret );
			return ret;
		}
		
		alup_init_register( alu, _NUM, NUM );
		alup_set( _NUM, fillwith );
		return 0;
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_set( alu_t *alu, uint_t num, bool fillwith )
{
	if ( alu )
	{
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_set( alu, NUM, fillwith );
	}
	return alu_err_null_ptr("alu");
}

int_t alur_get_raw
(
	alu_t *alu
	, alur_t SRC
	, void *dst
	, size_t size
	, uint_t info
)
{
		
	if ( alu )
	{
		alup_t _DST, _SRC;
		
		size = HIGHEST( size, 1 );
		
		alup_init_register( alu, _SRC, SRC );
		
		if ( info & ALU_INFO_FLOAT )
		{
			alup_init_floating( _DST, dst, size );
		}
		else
		{
			alup_init_unsigned( _DST, dst, size );
			_DST.info = info;
		}
		
		return alup_mov( _DST, _SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_get_raw( alu_t *alu, uint_t num, uintmax_t *raw )
{
	if ( alu )
	{
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_get_raw( alu, NUM, raw, sizeof(uintmax_t), 0 );
	}
	return alu_err_null_ptr("alu");
}

int_t alur_set_raw
(
	alu_t *alu
	, alur_t DST
	, void *src
	, size_t size
	, uint_t info
)
{	
	if ( alu )
	{
		alup_t _DST, _SRC;
		
		size = HIGHEST( size, 1 );
		
		alup_init_register( alu, _DST, DST );
		
		if ( info & ALU_INFO_FLOAT )
		{
			alup_init_floating( _SRC, src, size );
		}
		else
		{
			alup_init_unsigned( _SRC, src, size );
			_SRC.info = info;
		}
		
		return alup_mov( _DST, _SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_set_raw( alu_t *alu, uint_t num, uintmax_t raw, uint_t info )
{
	alur_t NUM;
	alur_init_unsigned( alu, NUM, num );
	return alur_set_raw( alu, NUM, &raw, sizeof(uintmax_t), info );
}

int_t	alu_mov( alu_t *alu, uint_t num, uint_t val )
{
	if ( alu )
	{
		alur_t NUM, VAL;
		
		alur_init_unsigned( alu, NUM, num );
		alur_init_unsigned( alu, VAL, val );
		
		return alur_mov( alu, NUM, VAL );
	}
	
	return alu_err_null_ptr("alu");
}

alub_t	alu_final_one( alu_t *alu, uint_t num )
{
	alub_t n = {0};
	
	if ( alu )
	{
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_final_one( alu, NUM );
	}
	
	(void)alu_err_null_ptr("alu");
	return n;
}

size_t alu_lowest_upto( alur_t NUM, alur_t VAL )
{
	size_t ndiff, vdiff;
	
	ndiff = (NUM.upto - NUM.from);
	vdiff = (VAL.upto - VAL.from);
		
	return NUM.from + LOWEST(ndiff,vdiff);
}
