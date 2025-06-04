static void check_one_assign(
	git_attr_file *file,
	int rule_idx,
	int assign_idx,
	const char *pattern,
	const char *name,
	enum attr_expect_t expected,
	const char *expected_str)
{
	git_attr_rule *rule = get_rule(rule_idx);
	git_attr_assignment *assign = get_assign(rule, assign_idx);

	cl_assert_equal_s(pattern, rule->match.pattern);
	cl_assert(rule->assigns.length == 1);
	cl_assert_equal_s(name, assign->name);
	cl_assert(assign->name_hash == git_attr_file__name_hash(assign->name));

	attr_check_expected(expected, expected_str, assign->name, assign->value);
}


// Source: file.c
// Lines 98-116
