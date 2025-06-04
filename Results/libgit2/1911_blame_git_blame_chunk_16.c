static int blame_chunk(
		git_blame *blame,
		size_t tlno,
		size_t plno,
		size_t same,
		git_blame__origin *target,
		git_blame__origin *parent)
{
	git_blame__entry *e;

	for (e = blame->ent; e; e = e->next) {
		if (e->guilty || !same_suspect(e->suspect, target))
			continue;
		if (same <= e->s_lno)
			continue;
		if (tlno < e->s_lno + e->num_lines) {
			if (blame_overlap(blame, e, tlno, plno, same, parent) < 0)
				return -1;
		}
	}

	return 0;
}


// Source: blame_git.c
// Lines 305-327
