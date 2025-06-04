static int binary_cb(
	const git_diff_delta *delta,
	const git_diff_binary *binary,
	void *payload)
{
	struct diff_data *diff_data = payload;

	GIT_UNUSED(delta);

	git_str_encode_base85(&diff_data->old_binary_base85,
		binary->old_file.data, binary->old_file.datalen);
	diff_data->old_binary_inflatedlen = binary->old_file.inflatedlen;
	diff_data->old_binary_type = binary->old_file.type;

	git_str_encode_base85(&diff_data->new_binary_base85,
		binary->new_file.data, binary->new_file.datalen);
	diff_data->new_binary_inflatedlen = binary->new_file.inflatedlen;
	diff_data->new_binary_type = binary->new_file.type;

	return 0;
}


// Source: binary.c
// Lines 445-465
