malloc_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	size_t i;
	const char *f;

#define APPEND_C(c) do {						\
	if (i < size) {							\
		str[i] = (c);						\
	}								\
	i++;								\
} while (0)
#define APPEND_S(s, slen) do {						\
	if (i < size) {							\
		size_t cpylen = (slen <= size - i) ? slen : size - i;	\
		memcpy(&str[i], s, cpylen);				\
	}								\
	i += slen;							\
} while (0)
#define APPEND_PADDED_S(s, slen, width, left_justify) do {		\
	/* Left padding. */						\
	size_t pad_len = (width == -1) ? 0 : ((slen < (size_t)width) ?	\
	    (size_t)width - slen : 0);					\
	if (!left_justify && pad_len != 0) {				\
		size_t j;						\
		for (j = 0; j < pad_len; j++) {				\
			APPEND_C(' ');					\
		}							\
	}								\
	/* Value. */							\
	APPEND_S(s, slen);						\
	/* Right padding. */						\
	if (left_justify && pad_len != 0) {				\
		size_t j;						\
		for (j = 0; j < pad_len; j++) {				\
			APPEND_C(' ');					\
		}							\
	}								\
} while (0)
#define GET_ARG_NUMERIC(val, len) do {					\
	switch ((unsigned char)len) {					\
	case '?':							\
		val = va_arg(ap, int);					\
		break;							\
	case '?' | 0x80:						\
		val = va_arg(ap, unsigned int);				\
		break;							\
	case 'l':							\
		val = va_arg(ap, long);					\
		break;							\
	case 'l' | 0x80:						\
		val = va_arg(ap, unsigned long);			\
		break;							\
	case 'q':							\
		val = va_arg(ap, long long);				\
		break;							\
	case 'q' | 0x80:						\
		val = va_arg(ap, unsigned long long);			\
		break;							\
	case 'j':							\
		val = va_arg(ap, intmax_t);				\
		break;							\
	case 'j' | 0x80:						\
		val = va_arg(ap, uintmax_t);				\
		break;							\
	case 't':							\
		val = va_arg(ap, ptrdiff_t);				\
		break;							\
	case 'z':							\
		val = va_arg(ap, ssize_t);				\
		break;							\
	case 'z' | 0x80:						\
		val = va_arg(ap, size_t);				\
		break;							\
	case 'p': /* Synthetic; used for %p. */				\
		val = va_arg(ap, uintptr_t);				\
		break;							\
	default:							\
		not_reached();						\
		val = 0;						\
	}								\
} while (0)

	i = 0;
	f = format;
	while (true) {
		switch (*f) {
		case '\0': goto label_out;
		case '%': {
			bool alt_form = false;
			bool left_justify = false;
			bool plus_space = false;
			bool plus_plus = false;
			int prec = -1;
			int width = -1;
			unsigned char len = '?';
			char *s;
			size_t slen;

			f++;
			/* Flags. */
			while (true) {
				switch (*f) {
				case '#':
					assert(!alt_form);
					alt_form = true;
					break;
				case '-':
					assert(!left_justify);
					left_justify = true;
					break;
				case ' ':
					assert(!plus_space);
					plus_space = true;
					break;
				case '+':
					assert(!plus_plus);
					plus_plus = true;
					break;
				default: goto label_width;
				}
				f++;
			}
			/* Width. */
			label_width:
			switch (*f) {
			case '*':
				width = va_arg(ap, int);
				f++;
				if (width < 0) {
					left_justify = true;
					width = -width;
				}
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': {
				uintmax_t uwidth;
				set_errno(0);
				uwidth = malloc_strtoumax(f, (char **)&f, 10);
				assert(uwidth != UINTMAX_MAX || get_errno() !=
				    ERANGE);
				width = (int)uwidth;
				break;
			} default:
				break;
			}
			/* Width/precision separator. */
			if (*f == '.') {
				f++;
			} else {
				goto label_length;
			}
			/* Precision. */
			switch (*f) {
			case '*':
				prec = va_arg(ap, int);
				f++;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': {
				uintmax_t uprec;
				set_errno(0);
				uprec = malloc_strtoumax(f, (char **)&f, 10);
				assert(uprec != UINTMAX_MAX || get_errno() !=
				    ERANGE);
				prec = (int)uprec;
				break;
			}
			default: break;
			}
			/* Length. */
			label_length:
			switch (*f) {
			case 'l':
				f++;
				if (*f == 'l') {
					len = 'q';
					f++;
				} else {
					len = 'l';
				}
				break;
			case 'q': case 'j': case 't': case 'z':
				len = *f;
				f++;
				break;
			default: break;
			}
			/* Conversion specifier. */
			switch (*f) {
			case '%':
				/* %% */
				APPEND_C(*f);
				f++;
				break;
			case 'd': case 'i': {
				intmax_t val JEMALLOC_CC_SILENCE_INIT(0);
				char buf[D2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len);
				s = d2s(val, (plus_plus ? '+' : (plus_space ?
				    ' ' : '-')), buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				f++;
				break;
			} case 'o': {
				uintmax_t val JEMALLOC_CC_SILENCE_INIT(0);
				char buf[O2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len | 0x80);
				s = o2s(val, alt_form, buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				f++;
				break;
			} case 'u': {
				uintmax_t val JEMALLOC_CC_SILENCE_INIT(0);
				char buf[U2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len | 0x80);
				s = u2s(val, 10, false, buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				f++;
				break;
			} case 'x': case 'X': {
				uintmax_t val JEMALLOC_CC_SILENCE_INIT(0);
				char buf[X2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len | 0x80);
				s = x2s(val, alt_form, *f == 'X', buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				f++;
				break;
			} case 'c': {
				unsigned char val;
				char buf[2];

				assert(len == '?' || len == 'l');
				assert_not_implemented(len != 'l');
				val = va_arg(ap, int);
				buf[0] = val;
				buf[1] = '\0';
				APPEND_PADDED_S(buf, 1, width, left_justify);
				f++;
				break;
			} case 's':
				assert(len == '?' || len == 'l');
				assert_not_implemented(len != 'l');
				s = va_arg(ap, char *);
				slen = (prec < 0) ? strlen(s) : (size_t)prec;
				APPEND_PADDED_S(s, slen, width, left_justify);
				f++;
				break;
			case 'p': {
				uintmax_t val;
				char buf[X2S_BUFSIZE];

				GET_ARG_NUMERIC(val, 'p');
				s = x2s(val, true, false, buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				f++;
				break;
			} default: not_reached();
			}
			break;
		} default: {
			APPEND_C(*f);
			f++;
			break;
		}}
	}
	label_out:
	if (i < size) {
		str[i] = '\0';
	} else {
		str[size - 1] = '\0';
	}

#undef APPEND_C
#undef APPEND_S
#undef APPEND_PADDED_S
#undef GET_ARG_NUMERIC
	return i;
}


// Source: malloc_io.c
// Lines 327-607
