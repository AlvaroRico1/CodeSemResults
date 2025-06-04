ch_enlargebufs(EditLine *el, size_t addlen)
{
	size_t sz, newsz;
	wchar_t *newbuffer, *oldbuf, *oldkbuf;

	sz = (size_t)(el->el_line.limit - el->el_line.buffer + EL_LEAVE);
	newsz = sz * 2;
	/*
	 * If newly required length is longer than current buffer, we need
	 * to make the buffer big enough to hold both old and new stuff.
	 */
	if (addlen > sz) {
		while(newsz - sz < addlen)
			newsz *= 2;
	}

	/*
	 * Reallocate line buffer.
	 */
	newbuffer = el_realloc(el->el_line.buffer, newsz * sizeof(*newbuffer));
	if (!newbuffer)
		return 0;

	/* zero the newly added memory, leave old data in */
	(void) memset(&newbuffer[sz], 0, (newsz - sz) * sizeof(*newbuffer));

	oldbuf = el->el_line.buffer;

	el->el_line.buffer = newbuffer;
	el->el_line.cursor = newbuffer + (el->el_line.cursor - oldbuf);
	el->el_line.lastchar = newbuffer + (el->el_line.lastchar - oldbuf);
	/* don't set new size until all buffers are enlarged */
	el->el_line.limit  = &newbuffer[sz - EL_LEAVE];

	/*
	 * Reallocate kill buffer.
	 */
	newbuffer = el_realloc(el->el_chared.c_kill.buf, newsz *
	    sizeof(*newbuffer));
	if (!newbuffer)
		return 0;

	/* zero the newly added memory, leave old data in */
	(void) memset(&newbuffer[sz], 0, (newsz - sz) * sizeof(*newbuffer));

	oldkbuf = el->el_chared.c_kill.buf;

	el->el_chared.c_kill.buf = newbuffer;
	el->el_chared.c_kill.last = newbuffer +
					(el->el_chared.c_kill.last - oldkbuf);
	el->el_chared.c_kill.mark = el->el_line.buffer +
					(el->el_chared.c_kill.mark - oldbuf);

	/*
	 * Reallocate undo buffer.
	 */
	newbuffer = el_realloc(el->el_chared.c_undo.buf,
	    newsz * sizeof(*newbuffer));
	if (!newbuffer)
		return 0;

	/* zero the newly added memory, leave old data in */
	(void) memset(&newbuffer[sz], 0, (newsz - sz) * sizeof(*newbuffer));
	el->el_chared.c_undo.buf = newbuffer;

	newbuffer = el_realloc(el->el_chared.c_redo.buf,
	    newsz * sizeof(*newbuffer));
	if (!newbuffer)
		return 0;
	el->el_chared.c_redo.pos = newbuffer +
			(el->el_chared.c_redo.pos - el->el_chared.c_redo.buf);
	el->el_chared.c_redo.lim = newbuffer +
			(el->el_chared.c_redo.lim - el->el_chared.c_redo.buf);
	el->el_chared.c_redo.buf = newbuffer;

	if (!hist_enlargebuf(el, sz, newsz))
		return 0;

	/* Safe to set enlarged buffer size */
	el->el_line.limit  = &el->el_line.buffer[newsz - EL_LEAVE];
	if (el->el_chared.c_resizefun)
		(*el->el_chared.c_resizefun)(el, el->el_chared.c_resizearg);
	return 1;
}


// Source: chared.c
// Lines 480-563
