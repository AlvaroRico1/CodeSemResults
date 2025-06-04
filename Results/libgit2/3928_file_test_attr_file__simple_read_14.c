void test_attr_file__simple_read(void)
{
	git_attr_file *file;
	git_attr_assignment *assign;
	git_attr_rule *rule;

	cl_git_pass(git_attr_file__load_standalone(&file, cl_fixture("attr/attr0")));

	cl_assert_equal_s(cl_fixture("attr/attr0"), file->entry->path);
	cl_assert(file->rules.length == 1);

	rule = get_rule(0);
	cl_assert(rule != NULL);
	cl_assert_equal_s("*", rule->match.pattern);
	cl_assert(rule->match.length == 1);
	cl_assert((rule->match.flags & GIT_ATTR_FNMATCH_HASWILD) != 0);

	cl_assert(rule->assigns.length == 1);
	assign = get_assign(rule, 0);
	cl_assert(assign != NULL);
	cl_assert_equal_s("binary", assign->name);
	cl_assert(GIT_ATTR_IS_TRUE(assign->value));

	git_attr_file__free(file);
}


// Source: file.c
// Lines 8-32
