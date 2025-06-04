static int clear_commit_marks_1(git_commit_list **plist,
		git_commit_list_node *commit, unsigned int mark)
{
	while (commit) {
		unsigned int i;

		if (!(mark & commit->flags))
			return 0;

		commit->flags &= ~mark;

		for (i = 1; i < commit->out_degree; i++) {
			git_commit_list_node *p = commit->parents[i];
			if (git_commit_list_insert(p, plist) == NULL)
				return -1;
		}


// Source: merge.c
// Lines 329-344
