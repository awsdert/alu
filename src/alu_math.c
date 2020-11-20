#include "alu.h"

bool_t alur_below0( alu_t *alu, alur_t REG )
{
	if ( alu )
	{	
		alup_init_register( alu, REG, 0 );
		
		return alup_below0( &(REG.alup) );
	}
	
	(void)alu_err_null_ptr("alu");
	return 0;
}

int_t alur_get_exponent( alu_t *alu, alur_t SRC, size_t *dst )
{
	if ( alu )
	{
		if ( dst )
		{	
			alup_init_register( alu, SRC, 0 );
			
			*dst = alup_get_exponent( &(SRC.alup) );
			
			return 0;
		}
		
		return alu_err_null_ptr("dst");
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_set_exponent( alu_t *alu, alur_t DST, size_t src )
{
	if ( alu )
	{
		alup_init_register( alu, DST, 0 );	
		return alup_set_exponent( &(DST.alup), src );
	}
	
	return alu_err_null_ptr("alu");
}

size_t alur_get_exponent_bias( alu_t *alu, alur_t SRC )
{
	if ( alu )
	{
		alup_init_register( alu, SRC, 0 );
		return alup_get_exponent( &(SRC.alup) );
	}
	
	(void)alu_err_null_ptr("alu");
	return 0;
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
		else if ( info & ALU_INFO__SIGN )
		{
			alur_init___signed( alu, NUM, num );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
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
		else if ( info & ALU_INFO__SIGN )
		{
			alur_init___signed( alu, NUM, num );
			alur_init___signed( alu, VAL, val );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			alur_init_unsigned( alu, VAL, val );
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
		else if ( info & ALU_INFO__SIGN )
		{
			alur_init___signed( alu, NUM, num );
			alur_init___signed( alu, VAL, val );
		}
		else
		{
			alur_init_unsigned( alu, NUM, num );
			alur_init_unsigned( alu, VAL, val );
		}
		
		return op4( alu, NUM, VAL, reg, tmp );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alur_set( alu_t *alu, alur_t NUM, bool fillwith )
{
	if ( alu )
	{
		alup_init_register( alu, NUM, 0 );
		return alup_set( &(NUM.alup), fillwith );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_set( alu_t *alu, uint_t num, bool fillwith )
{
	if ( alu )
	{
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alup_set( &(NUM.alup), fillwith );
	}
	return alu_err_null_ptr("alu");
}

int_t alur_get_raw
(
	alu_t *alu
	, alur_t SRC
	, void *dst
	, size_t bits
	, uint_t info
)
{
		
	if ( alu )
	{
		alup_t _DST;
		
		alup_init_register( alu, SRC, 0 );
		
		if ( info & ALU_INFO_FLOAT )
		{
			alup_init_floating( _DST, dst, bits );
		}
		else if ( info & ALU_INFO__SIGN )
		{
			alup_init___signed( _DST, dst, bits );
		}
		else
		{
			alup_init_unsigned( _DST, dst, bits );
		}
		
		return alup_mov( &_DST, &(SRC.alup) );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_get_raw( alu_t *alu, uint_t num, uintmax_t *raw )
{
	if ( alu )
	{
		alur_t NUM;
		alur_init_unsigned( alu, NUM, num );
		return alur_get_raw( alu, NUM, raw, bitsof(uintmax_t), 0 );
	}
	return alu_err_null_ptr("alu");
}

int_t alur_set_raw
(
	alu_t *alu
	, alur_t DST
	, void *src
	, size_t bits
	, uint_t info
)
{	
	if ( alu )
	{
		alup_t _SRC;
		
		alup_init_register( alu, DST, 0 );
		
		if ( info & ALU_INFO_FLOAT )
		{
			alup_init_floating( _SRC, src, bits );
		}
		else if ( info & ALU_INFO__SIGN )
		{
			alup_init___signed( _SRC, src, bits );
		}
		else
		{
			alup_init_unsigned( _SRC, src, bits );
		}
		
		return alup_mov( &(DST.alup), &_SRC );
	}
	
	return alu_err_null_ptr("alu");
}

int_t alu_set_raw( alu_t *alu, uint_t num, uintmax_t raw, uint_t info )
{
	alur_t NUM;
	alur_init_unsigned( alu, NUM, num );
	return alur_set_raw( alu, NUM, &raw, bitsof(uintmax_t), info );
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
		return alup_final_one( &(NUM.alup) );
	}
	
	(void)alu_err_null_ptr("alu");
	return n;
}

size_t alu_lowest_upto( alur_t NUM, alur_t VAL )
{		
	return NUM.alup.from + LOWEST(NUM.alup.leng,VAL.alup.leng);
}
