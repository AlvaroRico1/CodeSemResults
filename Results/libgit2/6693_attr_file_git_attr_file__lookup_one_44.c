int git_attr_file__lookup_one(
	git_attr_file *file,
	git_attr_path *path,
	const char *attr,
	const char **value)
{
	size_t i;
	git_attr_name name;
	git_attr_rule *rule;

	*value = NULL;

	name.name = attr;
	name.name_hash = git_attr_file__name_hash(attr);

	git_attr_file__foreach_matching_rule(file, path, i, rule) {
		size_t pos;

		if (!git_vector_bsearch(&pos, &rule->assigns, &name)) {
			*value = ((git_attr_assignment *)
					  git_vector_get(&rule->assigns, pos))->value;
			break;
		}
	}

	return 0;
}


// Source: attr_file.c
// Lines 408-434
