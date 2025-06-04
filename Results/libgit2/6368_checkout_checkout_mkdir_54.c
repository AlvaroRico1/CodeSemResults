static int checkout_mkdir(
	checkout_data *data,
	const char *path,
	const char *base,
	mode_t mode,
	unsigned int flags)
{
	struct git_futils_mkdir_options mkdir_opts = {0};
	int error;

	mkdir_opts.dir_map = data->mkdir_map;
	mkdir_opts.pool = &data->pool;

	error = git_futils_mkdir_relative(
		path, base, mode, flags, &mkdir_opts);

	data->perfdata.mkdir_calls += mkdir_opts.perfdata.mkdir_calls;
	data->perfdata.stat_calls += mkdir_opts.perfdata.stat_calls;
	data->perfdata.chmod_calls += mkdir_opts.perfdata.chmod_calls;

	return error;
}


// Source: checkout.c
// Lines 1412-1433
