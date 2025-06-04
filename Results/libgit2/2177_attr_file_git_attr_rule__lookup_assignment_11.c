git_attr_assignment *git_attr_rule__lookup_assignment(
	git_attr_rule *rule, const char *name)
{
	size_t pos;
	git_attr_name key;
	key.name = name;
	key.name_hash = git_attr_file__name_hash(name);

	if (git_vector_bsearch(&pos, &rule->assigns, &key))
		return NULL;

	return git_vector_get(&rule->assigns, pos);
}


// Source: attr_file.c
// Lines 538-550
