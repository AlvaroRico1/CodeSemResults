generic_listxattr(struct dentry *dentry, char *buffer, size_t buffer_size)
{
	const struct xattr_handler *handler, **handlers = dentry->d_sb->s_xattr;
	unsigned int size = 0;

	if (!buffer) {
		for_each_xattr_handler(handlers, handler) {
			if (!handler->name ||
			    (handler->list && !handler->list(dentry)))
				continue;
			size += strlen(handler->name) + 1;
		}
	} else {
		char *buf = buffer;
		size_t len;

		for_each_xattr_handler(handlers, handler) {
			if (!handler->name ||
			    (handler->list && !handler->list(dentry)))
				continue;
			len = strlen(handler->name);
			if (len + 1 > buffer_size)
				return -ERANGE;
			memcpy(buf, handler->name, len + 1);
			buf += len + 1;
			buffer_size -= len + 1;
		}
		size = buf - buffer;
	}


// Source: xattr.c
// Lines 750-778
