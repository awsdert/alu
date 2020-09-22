#include <check.h>
#include <alu.h>
#include <stdlib.h>

#define REG_COUNT 4

START_TEST( test_alu_create )
{
	int ret;
	alu_t _alu = {0}, *alu = &_alu;
	uint_t want = 32, nodes[REG_COUNT] = {0};
	
	puts( "Initialising ALU" );
	ret = alu_setup_reg( alu, want, 0, 0 );
	
	ck_assert( ret == 0 );
	ck_assert( alu_used( alu ) != 0 );
	ck_assert_msg
	(
		alu_upto( alu ) >= want
		, "Expected at least %u, Got %u"
		, want
		, alu_upto(alu)
	);
	ck_assert( alu_valid( alu ) != NULL );
	ck_assert( alu_size_perN( alu ) >= sizeof(uintmax_t) );
	
	puts( "Getting registers" );
	ret = alu_get_reg_nodes( alu, nodes, REG_COUNT, 0 );
	
	for ( want = 0; want < REG_COUNT; ++want )
	{
		ck_assert( nodes[want] != 0 );
	}
	
	alu_vec_release( alu, alu_size_perN(alu) );
}
END_TEST

/* Modified copy & paste of code given in check's guide */
Suite * alu_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("Money");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_alu_create);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	
	puts( "Running unit tests under 'check' test suite" );

	s = alu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
