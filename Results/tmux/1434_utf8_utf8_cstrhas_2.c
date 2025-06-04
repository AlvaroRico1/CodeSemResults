utf8_cstrhas(const char *s, const struct utf8_data *ud)
{
	struct utf8_data	*copy, *loop;
	int			 found = 0;

	copy = utf8_fromcstr(s);
	for (loop = copy; loop->size != 0; loop++) {
		if (loop->size != ud->size)
			continue;
		if (memcmp(loop->data, ud->data, loop->size) == 0) {
			found = 1;
			break;
		}
	}
	free(copy);

	return (found);
}


// Source: utf8.c
// Lines 584-601
