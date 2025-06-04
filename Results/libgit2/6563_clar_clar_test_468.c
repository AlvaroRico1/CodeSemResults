clar_test(int argc, char **argv)
{
	int errors;

	clar_test_init(argc, argv);
	errors = clar_test_run();
	clar_test_shutdown();

	return errors;
}


// Source: clar.c
// Lines 583-592
