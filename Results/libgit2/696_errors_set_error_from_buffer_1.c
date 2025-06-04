static void set_error_from_buffer(int error_class)
{
	git_error *error = &GIT_THREADSTATE->error_t;
	git_str *buf = &GIT_THREADSTATE->error_buf;

	error->message = buf->ptr;
	error->klass = error_class;

	GIT_THREADSTATE->last_error = error;
}


// Source: errors.c
// Lines 29-38
