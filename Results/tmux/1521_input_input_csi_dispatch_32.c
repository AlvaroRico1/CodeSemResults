input_csi_dispatch(struct input_ctx *ictx)
{
	struct screen_write_ctx	       *sctx = &ictx->ctx;
	struct screen		       *s = sctx->s;
	struct input_table_entry       *entry;
	int				i, n, m;
	u_int				cx, bg = ictx->cell.cell.bg;

	if (ictx->flags & INPUT_DISCARD)
		return (0);

	log_debug("%s: '%c' \"%s\" \"%s\"",
	    __func__, ictx->ch, ictx->interm_buf, ictx->param_buf);

	if (input_split(ictx) != 0)
		return (0);

	entry = bsearch(ictx, input_csi_table, nitems(input_csi_table),
	    sizeof input_csi_table[0], input_table_compare);
	if (entry == NULL) {
		log_debug("%s: unknown '%c'", __func__, ictx->ch);
		return (0);
	}

	switch (entry->type) {
	case INPUT_CSI_CBT:
		/* Find the previous tab point, n times. */
		cx = s->cx;
		if (cx > screen_size_x(s) - 1)
			cx = screen_size_x(s) - 1;
		n = input_get(ictx, 0, 1, 1);
		if (n == -1)
			break;
		while (cx > 0 && n-- > 0) {
			do
				cx--;
			while (cx > 0 && !bit_test(s->tabs, cx));
		}
		s->cx = cx;
		break;
	case INPUT_CSI_CUB:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_cursorleft(sctx, n);
		break;
	case INPUT_CSI_CUD:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_cursordown(sctx, n);
		break;
	case INPUT_CSI_CUF:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_cursorright(sctx, n);
		break;
	case INPUT_CSI_CUP:
		n = input_get(ictx, 0, 1, 1);
		m = input_get(ictx, 1, 1, 1);
		if (n != -1 && m != -1)
			screen_write_cursormove(sctx, m - 1, n - 1, 1);
		break;
	case INPUT_CSI_MODSET:
		n = input_get(ictx, 0, 0, 0);
		m = input_get(ictx, 1, 0, 0);
		if (options_get_number(global_options, "extended-keys") == 2)
			break;
		if (n == 0 || (n == 4 && m == 0))
			screen_write_mode_clear(sctx, MODE_KEXTENDED);
		else if (n == 4 && (m == 1 || m == 2))
			screen_write_mode_set(sctx, MODE_KEXTENDED);
		break;
	case INPUT_CSI_MODOFF:
		n = input_get(ictx, 0, 0, 0);
		if (n == 4)
			screen_write_mode_clear(sctx, MODE_KEXTENDED);
		break;
	case INPUT_CSI_WINOPS:
		input_csi_dispatch_winops(ictx);
		break;
	case INPUT_CSI_CUU:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_cursorup(sctx, n);
		break;
	case INPUT_CSI_CNL:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1) {
			screen_write_carriagereturn(sctx);
			screen_write_cursordown(sctx, n);
		}
		break;
	case INPUT_CSI_CPL:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1) {
			screen_write_carriagereturn(sctx);
			screen_write_cursorup(sctx, n);
		}
		break;
	case INPUT_CSI_DA:
		switch (input_get(ictx, 0, 0, 0)) {
		case -1:
			break;
		case 0:
			input_reply(ictx, "\033[?1;2c");
			break;
		default:
			log_debug("%s: unknown '%c'", __func__, ictx->ch);
			break;
		}
		break;
	case INPUT_CSI_DA_TWO:
		switch (input_get(ictx, 0, 0, 0)) {
		case -1:
			break;
		case 0:
			input_reply(ictx, "\033[>84;0;0c");
			break;
		default:
			log_debug("%s: unknown '%c'", __func__, ictx->ch);
			break;
		}
		break;
	case INPUT_CSI_ECH:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_clearcharacter(sctx, n, bg);
		break;
	case INPUT_CSI_DCH:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_deletecharacter(sctx, n, bg);
		break;
	case INPUT_CSI_DECSTBM:
		n = input_get(ictx, 0, 1, 1);
		m = input_get(ictx, 1, 1, screen_size_y(s));
		if (n != -1 && m != -1)
			screen_write_scrollregion(sctx, n - 1, m - 1);
		break;
	case INPUT_CSI_DL:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_deleteline(sctx, n, bg);
		break;
	case INPUT_CSI_DSR:
		switch (input_get(ictx, 0, 0, 0)) {
		case -1:
			break;
		case 5:
			input_reply(ictx, "\033[0n");
			break;
		case 6:
			input_reply(ictx, "\033[%u;%uR", s->cy + 1, s->cx + 1);
			break;
		default:
			log_debug("%s: unknown '%c'", __func__, ictx->ch);
			break;
		}
		break;
	case INPUT_CSI_ED:
		switch (input_get(ictx, 0, 0, 0)) {
		case -1:
			break;
		case 0:
			screen_write_clearendofscreen(sctx, bg);
			break;
		case 1:
			screen_write_clearstartofscreen(sctx, bg);
			break;
		case 2:
			screen_write_clearscreen(sctx, bg);
			break;
		case 3:
			if (input_get(ictx, 1, 0, 0) == 0) {
				/*
				 * Linux console extension to clear history
				 * (for example before locking the screen).
				 */
				screen_write_clearhistory(sctx);
			}
			break;
		default:
			log_debug("%s: unknown '%c'", __func__, ictx->ch);
			break;
		}
		break;
	case INPUT_CSI_EL:
		switch (input_get(ictx, 0, 0, 0)) {
		case -1:
			break;
		case 0:
			screen_write_clearendofline(sctx, bg);
			break;
		case 1:
			screen_write_clearstartofline(sctx, bg);
			break;
		case 2:
			screen_write_clearline(sctx, bg);
			break;
		default:
			log_debug("%s: unknown '%c'", __func__, ictx->ch);
			break;
		}
		break;
	case INPUT_CSI_HPA:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_cursormove(sctx, n - 1, -1, 1);
		break;
	case INPUT_CSI_ICH:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_insertcharacter(sctx, n, bg);
		break;
	case INPUT_CSI_IL:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_insertline(sctx, n, bg);
		break;
	case INPUT_CSI_REP:
		n = input_get(ictx, 0, 1, 1);
		if (n == -1)
			break;

		m = screen_size_x(s) - s->cx;
		if (n > m)
			n = m;

		if (ictx->last == -1)
			break;
		ictx->ch = ictx->last;

		for (i = 0; i < n; i++)
			input_print(ictx);
		break;
	case INPUT_CSI_RCP:
		input_restore_state(ictx);
		break;
	case INPUT_CSI_RM:
		input_csi_dispatch_rm(ictx);
		break;
	case INPUT_CSI_RM_PRIVATE:
		input_csi_dispatch_rm_private(ictx);
		break;
	case INPUT_CSI_SCP:
		input_save_state(ictx);
		break;
	case INPUT_CSI_SGR:
		input_csi_dispatch_sgr(ictx);
		break;
	case INPUT_CSI_SM:
		input_csi_dispatch_sm(ictx);
		break;
	case INPUT_CSI_SM_PRIVATE:
		input_csi_dispatch_sm_private(ictx);
		break;
	case INPUT_CSI_SU:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_scrollup(sctx, n, bg);
		break;
	case INPUT_CSI_SD:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_scrolldown(sctx, n, bg);
		break;
	case INPUT_CSI_TBC:
		switch (input_get(ictx, 0, 0, 0)) {
		case -1:
			break;
		case 0:
			if (s->cx < screen_size_x(s))
				bit_clear(s->tabs, s->cx);
			break;
		case 3:
			bit_nclear(s->tabs, 0, screen_size_x(s) - 1);
			break;
		default:
			log_debug("%s: unknown '%c'", __func__, ictx->ch);
			break;
		}
		break;
	case INPUT_CSI_VPA:
		n = input_get(ictx, 0, 1, 1);
		if (n != -1)
			screen_write_cursormove(sctx, -1, n - 1, 1);
		break;
	case INPUT_CSI_DECSCUSR:
		n = input_get(ictx, 0, 0, 0);
		if (n != -1)
			screen_set_cursor_style(s, n);
		break;
	case INPUT_CSI_XDA:
		n = input_get(ictx, 0, 0, 0);
		if (n == 0)
			input_reply(ictx, "\033P>|tmux %s\033\\", getversion());
		break;

	}

	ictx->last = -1;
	return (0);
}


// Source: input.c
// Lines 1331-1632
