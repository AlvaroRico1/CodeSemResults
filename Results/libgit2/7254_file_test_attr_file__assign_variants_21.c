void test_attr_file__assign_variants(void)
{
	git_attr_file *file;
	git_attr_rule *rule;
	git_attr_assignment *assign;

	cl_git_pass(git_attr_file__load_standalone(&file, cl_fixture("attr/attr2")));

	cl_assert_equal_s(cl_fixture("attr/attr2"), file->entry->path);
	cl_assert(file->rules.length == 11);

	check_one_assign(file, 0, 0, "pat0", "simple", EXPECT_TRUE, NULL);
	check_one_assign(file, 1, 0, "pat1", "neg", EXPECT_FALSE, NULL);
	check_one_assign(file, 2, 0, "*", "notundef", EXPECT_TRUE, NULL);
	check_one_assign(file, 3, 0, "pat2", "notundef", EXPECT_UNDEFINED, NULL);
	check_one_assign(file, 4, 0, "pat3", "assigned", EXPECT_STRING, "test-value");
	check_one_assign(file, 5, 0, "pat4", "rule-with-more-chars", EXPECT_STRING, "value-with-more-chars");
	check_one_assign(file, 6, 0, "pat5", "empty", EXPECT_TRUE, NULL);
	check_one_assign(file, 7, 0, "pat6", "negempty", EXPECT_FALSE, NULL);

	rule = get_rule(8);
	cl_assert_equal_s("pat7", rule->match.pattern);
	cl_assert(rule->assigns.length == 5);
	/* assignments will be sorted by hash value, so we have to do
	 * lookups by search instead of by position
	 */
	assign = git_attr_rule__lookup_assignment(rule, "multiple");
	cl_assert(assign);
	cl_assert_equal_s("multiple", assign->name);
	cl_assert(GIT_ATTR_IS_TRUE(assign->value));
	assign = git_attr_rule__lookup_assignment(rule, "single");
	cl_assert(assign);
	cl_assert_equal_s("single", assign->name);
	cl_assert(GIT_ATTR_IS_FALSE(assign->value));
	assign = git_attr_rule__lookup_assignment(rule, "values");
	cl_assert(assign);
	cl_assert_equal_s("values", assign->name);
	cl_assert_equal_s("1", assign->value);
	assign = git_attr_rule__lookup_assignment(rule, "also");
	cl_assert(assign);
	cl_assert_equal_s("also", assign->name);
	cl_assert_equal_s("a-really-long-value/*", assign->value);
	assign = git_attr_rule__lookup_assignment(rule, "happy");
	cl_assert(assign);
	cl_assert_equal_s("happy", assign->name);
	cl_assert_equal_s("yes!", assign->value);
	assign = git_attr_rule__lookup_assignment(rule, "other");
	cl_assert(!assign);

	rule = get_rule(9);
	cl_assert_equal_s("pat8", rule->match.pattern);
	cl_assert(rule->assigns.length == 2);
	assign = git_attr_rule__lookup_assignment(rule, "again");
	cl_assert(assign);
	cl_assert_equal_s("again", assign->name);
	cl_assert(GIT_ATTR_IS_TRUE(assign->value));
	assign = git_attr_rule__lookup_assignment(rule, "another");
	cl_assert(assign);
	cl_assert_equal_s("another", assign->name);
	cl_assert_equal_s("12321", assign->value);

	check_one_assign(file, 10, 0, "pat9", "at-eof", EXPECT_FALSE, NULL);

	git_attr_file__free(file);
}


// Source: file.c
// Lines 118-182
