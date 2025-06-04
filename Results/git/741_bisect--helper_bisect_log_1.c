static enum bisect_error bisect_log(void)
{
	int fd, status;
	const char* filename = git_path_bisect_log();

	if (is_empty_or_missing_file(filename))
		return error(_("We are not bisecting."));

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return BISECT_FAILED;

	status = copy_fd(fd, STDOUT_FILENO);
	close(fd);
	return status ? BISECT_FAILED : BISECT_OK;
}


// Source: bisect--helper.c
// Lines 930-945
