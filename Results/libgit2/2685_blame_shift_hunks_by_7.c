static void shift_hunks_by(git_vector *v, size_t start_line, int shift_by)
{
	size_t i;

	if (!git_vector_bsearch2(&i, v, hunk_byfinalline_search_cmp, &start_line)) {
		for (; i < v->length; i++) {
			git_blame_hunk *hunk = (git_blame_hunk*)v->contents[i];
			hunk->final_start_line_number += shift_by;
		}


// Source: blame.c
// Lines 113-121
