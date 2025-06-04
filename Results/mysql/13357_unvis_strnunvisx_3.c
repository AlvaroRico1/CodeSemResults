strnunvisx(char *dst, size_t dlen, const char *src, int flag)
{
	char c;
	char t = '\0', *start = dst;
	int state = 0;

	_DIAGASSERT(src != NULL);
	_DIAGASSERT(dst != NULL);
#define CHECKSPACE() \
	do { \
		if (dlen-- == 0) { \
			errno = ENOSPC; \
			return -1; \
		} \
	} while (/*CONSTCOND*/0)

	while ((c = *src++) != '\0') {
 again:
		switch (unvis(&t, c, &state, flag)) {
		case UNVIS_VALID:
			CHECKSPACE();
			*dst++ = t;
			break;
		case UNVIS_VALIDPUSH:
			CHECKSPACE();
			*dst++ = t;
			goto again;
		case 0:
		case UNVIS_NOCHAR:
			break;
		case UNVIS_SYNBAD:
			errno = EINVAL;
			return -1;
		default:
			_DIAGASSERT(/*CONSTCOND*/0);
			errno = EINVAL;
			return -1;
		}
	}
	if (unvis(&t, c, &state, UNVIS_END) == UNVIS_VALID) {
		CHECKSPACE();
		*dst++ = t;
	}
	CHECKSPACE();
	*dst = '\0';
	return (int)(dst - start);
}


// Source: unvis.c
// Lines 495-541
