el_get(EditLine *el, int op, ...)
{
	va_list ap;
	int ret;

	if (!el)
		return -1;

	va_start(ap, op);

	switch (op) {
	case EL_PROMPT:         /* el_pfunc_t * */
	case EL_RPROMPT: {
		el_pfunc_t *p = va_arg(ap, el_pfunc_t *);
		ret = prompt_get(el, p, 0, op);
		break;
	}

	case EL_PROMPT_ESC: /* el_pfunc_t *, char **/
	case EL_RPROMPT_ESC: {
		el_pfunc_t *p = va_arg(ap, el_pfunc_t *);
		char *c = va_arg(ap, char *);
		wchar_t wc = 0;
		ret = prompt_get(el, p, &wc, op);
		*c = (char)wc;
		break;
	}

	case EL_EDITOR: {
		const char **p = va_arg(ap, const char **);
		const wchar_t *pw;
		ret = el_wget(el, op, &pw);
		*p = ct_encode_string(pw, &el->el_lgcyconv);
		if (!el->el_lgcyconv.csize)
			ret = -1;
		break;
	}


// Source: eln.c
// Lines 274-310
