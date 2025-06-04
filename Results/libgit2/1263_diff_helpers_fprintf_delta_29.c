static void fprintf_delta(FILE *fp, const git_diff_delta *delta, float progress)
{
	char code = git_diff_status_char(delta->status);
	char old_suffix = diff_pick_suffix(delta->old_file.mode);
	char new_suffix = diff_pick_suffix(delta->new_file.mode);

	fprintf(fp, "%c\t%s", code, delta->old_file.path);

	if ((delta->old_file.path != delta->new_file.path &&
		 strcmp(delta->old_file.path, delta->new_file.path) != 0) ||
		(delta->old_file.mode != delta->new_file.mode &&
		 delta->old_file.mode != 0 && delta->new_file.mode != 0))
		fprintf(fp, "%c %s%c", old_suffix, delta->new_file.path, new_suffix);
	else if (old_suffix != ' ')
		fprintf(fp, "%c", old_suffix);

	fprintf(fp, "\t[%.2f]\n", progress);
}


// Source: diff_helpers.c
// Lines 32-49
