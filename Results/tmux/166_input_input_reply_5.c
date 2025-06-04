input_reply(struct input_ctx *ictx, const char *fmt, ...)
{
	struct bufferevent	*bev = ictx->event;
	va_list			 ap;
	char			*reply;

	va_start(ap, fmt);
	xvasprintf(&reply, fmt, ap);
	va_end(ap);

	bufferevent_write(bev, reply, strlen(reply));
	free(reply);
}


// Source: input.c
// Lines 1073-1085
