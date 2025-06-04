static int merge_assignments(void **old_raw, void *new_raw)
{
	git_attr_assignment **old = (git_attr_assignment **)old_raw;
	git_attr_assignment *new = (git_attr_assignment *)new_raw;

	GIT_REFCOUNT_DEC(*old, git_attr_assignment__free);
	*old = new;
	return GIT_EEXISTS;
}


// Source: attr_file.c
// Lines 864-872
