layout_fix_offsets(struct window *w)
{
	struct layout_cell      *lc = w->layout_root;

	lc->xoff = 0;
	lc->yoff = 0;

	layout_fix_offsets1(lc);
}


// Source: layout.c
// Lines 231-239
