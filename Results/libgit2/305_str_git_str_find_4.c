GIT_INLINE(ssize_t) git_str_find(const git_str *str, char ch)
{
	void *found = memchr(str->ptr, ch, str->size);
	return found ? (ssize_t)((const char *)found - str->ptr) : -1;
}


// Source: str.h
// Lines 203-207
