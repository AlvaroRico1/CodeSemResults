static void show_parents(struct commit *commit, int abbrev, FILE *file)
{
	struct commit_list *p;
	for (p = commit->parents; p ; p = p->next) {
		struct commit *parent = p->item;
		fprintf(file, " %s", find_unique_abbrev(&parent->object.oid, abbrev));
	}


// Source: log-tree.c
// Lines 219-225
