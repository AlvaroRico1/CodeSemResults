void test_apply_both__rename_and_modify(void)
{
	git_diff *diff;

	struct merge_index_entry both_expected[] = {
		{ 0100644, "f51658077d85f2264fa179b4d0848268cb3475c3", 0, "asparagus.txt" },
		{ 0100644, "4b7c5650008b2e747fe1809eeb5a1dde0e80850a", 0, "bouilli.txt" },
		{ 0100644, "c4e6cca3ec6ae0148ed231f97257df8c311e015f", 0, "gravy.txt" },
		{ 0100644, "6fa10147f00fe1fab1d5e835529a9dad53db8552", 0, "notbeef.txt" },
		{ 0100644, "68af1fc7407fd9addf1701a87eb1c95c7494c598", 0, "oyster.txt" },
		{ 0100644, "94d2c01087f48213bd157222d54edfefd77c9bba", 0, "veal.txt" },
	};
	size_t both_expected_cnt = sizeof(both_expected) /
		sizeof(struct merge_index_entry);

	cl_git_pass(git_diff_from_buffer(&diff, DIFF_RENAME_AND_MODIFY_FILE,
		strlen(DIFF_RENAME_AND_MODIFY_FILE)));
	cl_git_pass(git_apply(repo, diff, GIT_APPLY_LOCATION_BOTH, NULL));

    validate_apply_index(repo, both_expected, both_expected_cnt);
	validate_apply_workdir(repo, both_expected, both_expected_cnt);

	git_diff_free(diff);
}


// Source: both.c
// Lines 428-451
