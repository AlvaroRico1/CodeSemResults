static void assert_examples(git_attr_file *file)
{
	git_attr_rule *rule;
	git_attr_assignment *assign;

	rule = get_rule(0);
	cl_assert_equal_s("*.java", rule->match.pattern);
	cl_assert(rule->assigns.length == 3);
	assign = git_attr_rule__lookup_assignment(rule, "diff");
	cl_assert_equal_s("diff", assign->name);
	cl_assert_equal_s("java", assign->value);
	assign = git_attr_rule__lookup_assignment(rule, "crlf");
	cl_assert_equal_s("crlf", assign->name);
	cl_assert(GIT_ATTR_IS_FALSE(assign->value));
	assign = git_attr_rule__lookup_assignment(rule, "myAttr");
	cl_assert_equal_s("myAttr", assign->name);
	cl_assert(GIT_ATTR_IS_TRUE(assign->value));
	assign = git_attr_rule__lookup_assignment(rule, "missing");
	cl_assert(assign == NULL);

	rule = get_rule(1);
	cl_assert_equal_s("NoMyAttr.java", rule->match.pattern);
	cl_assert(rule->assigns.length == 1);
	assign = get_assign(rule, 0);
	cl_assert_equal_s("myAttr", assign->name);
	cl_assert(GIT_ATTR_IS_UNSPECIFIED(assign->value));

	rule = get_rule(2);
	cl_assert_equal_s("README", rule->match.pattern);
	cl_assert(rule->assigns.length == 1);
	assign = get_assign(rule, 0);
	cl_assert_equal_s("caveat", assign->name);
	cl_assert_equal_s("unspecified", assign->value);
}


// Source: file.c
// Lines 184-217
