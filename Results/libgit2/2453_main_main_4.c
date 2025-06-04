int __cdecl main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int res;
	char *at_exit_cmd;

	clar_test_init(argc, argv);

	res = git_libgit2_init();
	if (res < 0) {
		const git_error *err = git_error_last();
		const char *msg = err ? err->message : "unknown failure";
		fprintf(stderr, "failed to init libgit2: %s\n", msg);
		return res;
	}


// Source: main.c
// Lines 9-25
