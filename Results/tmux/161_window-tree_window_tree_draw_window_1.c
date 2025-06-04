window_tree_draw_window(struct window_tree_modedata *data, struct session *s,
    struct window *w, struct screen_write_ctx *ctx, u_int sx, u_int sy)
{
	struct options		*oo = s->options;
	struct window_pane	*wp;
	u_int			 cx = ctx->s->cx, cy = ctx->s->cy;
	u_int			 loop, total, visible, each, width, offset;
	u_int			 current, start, end, remaining, i;
	struct grid_cell	 gc;
	int			 colour, active_colour, left, right, pane_idx;
	char			*label;

	total = window_count_panes(w);

	memcpy(&gc, &grid_default_cell, sizeof gc);
	colour = options_get_number(oo, "display-panes-colour");
	active_colour = options_get_number(oo, "display-panes-active-colour");

	if (sx / total < 24) {
		visible = sx / 24;
		if (visible == 0)
			visible = 1;
	} else
		visible = total;

	current = 0;
	TAILQ_FOREACH(wp, &w->panes, entry) {
		if (wp == w->active)
			break;
		current++;
	}

	if (current < visible) {
		start = 0;
		end = visible;
	} else if (current >= total - visible) {
		start = total - visible;
		end = total;
	} else {
		start = current - (visible / 2);
		end = start + visible;
	}

	if (data->offset < -(int)start)
		data->offset = -(int)start;
	if (data->offset > (int)(total - end))
		data->offset = (int)(total - end);
	start += data->offset;
	end += data->offset;

	left = (start != 0);
	right = (end != total);
	if (((left && right) && sx <= 6) || ((left || right) && sx <= 3))
		left = right = 0;
	if (left && right) {
		each = (sx - 6) / visible;
		remaining = (sx - 6) - (visible * each);
	} else if (left || right) {
		each = (sx - 3) / visible;
		remaining = (sx - 3) - (visible * each);
	} else {
		each = sx / visible;
		remaining = sx - (visible * each);
	}
	if (each == 0)
		return;

	if (left) {
		data->left = cx + 2;
		screen_write_cursormove(ctx, cx + 2, cy, 0);
		screen_write_vline(ctx, sy, 0, 0);
		screen_write_cursormove(ctx, cx, cy + sy / 2, 0);
		screen_write_puts(ctx, &grid_default_cell, "<");
	} else
		data->left = -1;
	if (right) {
		data->right = cx + sx - 3;
		screen_write_cursormove(ctx, cx + sx - 3, cy, 0);
		screen_write_vline(ctx, sy, 0, 0);
		screen_write_cursormove(ctx, cx + sx - 1, cy + sy / 2, 0);
		screen_write_puts(ctx, &grid_default_cell, ">");
	} else
		data->right = -1;

	data->start = start;
	data->end = end;
	data->each = each;

	i = loop = 0;
	TAILQ_FOREACH(wp, &w->panes, entry) {
		if (loop == end)
			break;
		if (loop < start) {
			loop++;
			continue;
		}

		if (wp == w->active)
			gc.fg = active_colour;
		else
			gc.fg = colour;

		if (left)
			offset = 3 + (i * each);
		else
			offset = (i * each);
		if (loop == end - 1)
			width = each + remaining;
		else
			width = each - 1;

		screen_write_cursormove(ctx, cx + offset, cy, 0);
		screen_write_preview(ctx, &wp->base, width, sy);

		if (window_pane_index(wp, &pane_idx) != 0)
			pane_idx = loop;
		xasprintf(&label, " %u ", pane_idx);
		window_tree_draw_label(ctx, cx + offset, cy, each, sy, &gc,
		    label);
		free(label);

		if (loop != end - 1) {
			screen_write_cursormove(ctx, cx + offset + width, cy, 0);
			screen_write_vline(ctx, sy, 0, 0);
		}
		loop++;

		i++;
	}
}


// Source: window-tree.c
// Lines 664-793
