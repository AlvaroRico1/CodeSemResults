static int write_on_comment(git_config_parser *reader, const char *line, size_t line_len, void *data)
{
	struct write_data *write_data;

	GIT_UNUSED(reader);

	write_data = (struct write_data *)data;
	return write_line_to(&write_data->buffered_comment, line, line_len);
}


// Source: config_file.c
// Lines 1075-1083
