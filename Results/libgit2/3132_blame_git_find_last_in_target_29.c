static bool find_last_in_target(size_t *out, git_blame *blame, git_blame__origin *target)
{
	git_blame__entry *e;
	size_t last_in_target = 0;
	bool found = false;

	*out = 0;

	for (e=blame->ent; e; e=e->next) {
		if (e->guilty || !same_suspect(e->suspect, target))
			continue;
		if (last_in_target < e->s_lno + e->num_lines) {
			found = true;
			last_in_target = e->s_lno + e->num_lines;
		}
	}

	*out = last_in_target;
	return found;
}


// Source: blame_git.c
// Lines 99-118
