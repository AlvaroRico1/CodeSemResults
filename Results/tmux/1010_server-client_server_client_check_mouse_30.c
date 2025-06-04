server_client_check_mouse(struct client *c, struct key_event *event)
{
	struct mouse_event	*m = &event->m;
	struct session		*s = c->session;
	struct winlink		*wl;
	struct window_pane	*wp;
	u_int			 x, y, b, sx, sy, px, py;
	int			 ignore = 0;
	key_code		 key;
	struct timeval		 tv;
	struct style_range	*sr;
	enum { NOTYPE,
	       MOVE,
	       DOWN,
	       UP,
	       DRAG,
	       WHEEL,
	       SECOND,
	       DOUBLE,
	       TRIPLE } type = NOTYPE;
	enum { NOWHERE,
	       PANE,
	       STATUS,
	       STATUS_LEFT,
	       STATUS_RIGHT,
	       STATUS_DEFAULT,
	       BORDER } where = NOWHERE;

	log_debug("%s mouse %02x at %u,%u (last %u,%u) (%d)", c->name, m->b,
	    m->x, m->y, m->lx, m->ly, c->tty.mouse_drag_flag);

	/* What type of event is this? */
	if (event->key == KEYC_DOUBLECLICK) {
		type = DOUBLE;
		x = m->x, y = m->y, b = m->b;
		ignore = 1;
		log_debug("double-click at %u,%u", x, y);
	} else if ((m->sgr_type != ' ' &&
	    MOUSE_DRAG(m->sgr_b) &&
	    MOUSE_BUTTONS(m->sgr_b) == 3) ||
	    (m->sgr_type == ' ' &&
	    MOUSE_DRAG(m->b) &&
	    MOUSE_BUTTONS(m->b) == 3 &&
	    MOUSE_BUTTONS(m->lb) == 3)) {
		type = MOVE;
		x = m->x, y = m->y, b = 0;
		log_debug("move at %u,%u", x, y);
	} else if (MOUSE_DRAG(m->b)) {
		type = DRAG;
		if (c->tty.mouse_drag_flag) {
			x = m->x, y = m->y, b = m->b;
			if (x == m->lx && y == m->ly)
				return (KEYC_UNKNOWN);
			log_debug("drag update at %u,%u", x, y);
		} else {
			x = m->lx, y = m->ly, b = m->lb;
			log_debug("drag start at %u,%u", x, y);
		}
	} else if (MOUSE_WHEEL(m->b)) {
		type = WHEEL;
		x = m->x, y = m->y, b = m->b;
		log_debug("wheel at %u,%u", x, y);
	} else if (MOUSE_RELEASE(m->b)) {
		type = UP;
		x = m->x, y = m->y, b = m->lb;
		log_debug("up at %u,%u", x, y);
	} else {
		if (c->flags & CLIENT_DOUBLECLICK) {
			evtimer_del(&c->click_timer);
			c->flags &= ~CLIENT_DOUBLECLICK;
			if (m->b == c->click_button) {
				type = SECOND;
				x = m->x, y = m->y, b = m->b;
				log_debug("second-click at %u,%u", x, y);
				c->flags |= CLIENT_TRIPLECLICK;
			}
		} else if (c->flags & CLIENT_TRIPLECLICK) {
			evtimer_del(&c->click_timer);
			c->flags &= ~CLIENT_TRIPLECLICK;
			if (m->b == c->click_button) {
				type = TRIPLE;
				x = m->x, y = m->y, b = m->b;
				log_debug("triple-click at %u,%u", x, y);
				goto have_event;
			}
		} else {
			type = DOWN;
			x = m->x, y = m->y, b = m->b;
			log_debug("down at %u,%u", x, y);
			c->flags |= CLIENT_DOUBLECLICK;
		}

		if (KEYC_CLICK_TIMEOUT != 0) {
			memcpy(&c->click_event, m, sizeof c->click_event);
			c->click_button = m->b;

			log_debug("click timer started");
			tv.tv_sec = KEYC_CLICK_TIMEOUT / 1000;
			tv.tv_usec = (KEYC_CLICK_TIMEOUT % 1000) * 1000L;
			evtimer_del(&c->click_timer);
			evtimer_add(&c->click_timer, &tv);
		}
	}

have_event:
	if (type == NOTYPE)
		return (KEYC_UNKNOWN);

	/* Save the session. */
	m->s = s->id;
	m->w = -1;
	m->ignore = ignore;

	/* Is this on the status line? */
	m->statusat = status_at_line(c);
	m->statuslines = status_line_size(c);
	if (m->statusat != -1 &&
	    y >= (u_int)m->statusat &&
	    y < m->statusat + m->statuslines) {
		sr = status_get_range(c, x, y - m->statusat);
		if (sr == NULL) {
			where = STATUS_DEFAULT;
		} else {
			switch (sr->type) {
			case STYLE_RANGE_NONE:
				return (KEYC_UNKNOWN);
			case STYLE_RANGE_LEFT:
				where = STATUS_LEFT;
				break;
			case STYLE_RANGE_RIGHT:
				where = STATUS_RIGHT;
				break;
			case STYLE_RANGE_WINDOW:
				wl = winlink_find_by_index(&s->windows,
				    sr->argument);
				if (wl == NULL)
					return (KEYC_UNKNOWN);
				m->w = wl->window->id;

				where = STATUS;
				break;
			}
		}
	}

	/* Not on status line. Adjust position and check for border or pane. */
	if (where == NOWHERE) {
		px = x;
		if (m->statusat == 0 && y >= m->statuslines)
			py = y - m->statuslines;
		else if (m->statusat > 0 && y >= (u_int)m->statusat)
			py = m->statusat - 1;
		else
			py = y;

		tty_window_offset(&c->tty, &m->ox, &m->oy, &sx, &sy);
		log_debug("mouse window @%u at %u,%u (%ux%u)",
		    s->curw->window->id, m->ox, m->oy, sx, sy);
		if (px > sx || py > sy)
			return (KEYC_UNKNOWN);
		px = px + m->ox;
		py = py + m->oy;

		/* Try the pane borders if not zoomed. */
		if (~s->curw->window->flags & WINDOW_ZOOMED) {
			TAILQ_FOREACH(wp, &s->curw->window->panes, entry) {
				if ((wp->xoff + wp->sx == px &&
				    wp->yoff <= 1 + py &&
				    wp->yoff + wp->sy >= py) ||
				    (wp->yoff + wp->sy == py &&
				    wp->xoff <= 1 + px &&
				    wp->xoff + wp->sx >= px))
					break;
			}
			if (wp != NULL)
				where = BORDER;
		}

		/* Otherwise try inside the pane. */
		if (where == NOWHERE) {
			wp = window_get_active_at(s->curw->window, px, py);
			if (wp != NULL)
				where = PANE;
			else
				return (KEYC_UNKNOWN);
		}
		if (where == PANE)
			log_debug("mouse %u,%u on pane %%%u", x, y, wp->id);
		else if (where == BORDER)
			log_debug("mouse on pane %%%u border", wp->id);
		m->wp = wp->id;
		m->w = wp->window->id;
	} else
		m->wp = -1;

	/* Stop dragging if needed. */
	if (type != DRAG && type != WHEEL && c->tty.mouse_drag_flag) {
		if (c->tty.mouse_drag_release != NULL)
			c->tty.mouse_drag_release(c, m);

		c->tty.mouse_drag_update = NULL;
		c->tty.mouse_drag_release = NULL;

		/*
		 * End a mouse drag by passing a MouseDragEnd key corresponding
		 * to the button that started the drag.
		 */
		switch (c->tty.mouse_drag_flag) {
		case 1:
			if (where == PANE)
				key = KEYC_MOUSEDRAGEND1_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEDRAGEND1_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEDRAGEND1_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEDRAGEND1_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEDRAGEND1_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEDRAGEND1_BORDER;
			break;
		case 2:
			if (where == PANE)
				key = KEYC_MOUSEDRAGEND2_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEDRAGEND2_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEDRAGEND2_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEDRAGEND2_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEDRAGEND2_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEDRAGEND2_BORDER;
			break;
		case 3:
			if (where == PANE)
				key = KEYC_MOUSEDRAGEND3_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEDRAGEND3_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEDRAGEND3_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEDRAGEND3_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEDRAGEND3_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEDRAGEND3_BORDER;
			break;
		default:
			key = KEYC_MOUSE;
			break;
		}
		c->tty.mouse_drag_flag = 0;
		goto out;
	}

	/* Convert to a key binding. */
	key = KEYC_UNKNOWN;
	switch (type) {
	case NOTYPE:
		break;
	case MOVE:
		if (where == PANE)
			key = KEYC_MOUSEMOVE_PANE;
		if (where == STATUS)
			key = KEYC_MOUSEMOVE_STATUS;
		if (where == STATUS_LEFT)
			key = KEYC_MOUSEMOVE_STATUS_LEFT;
		if (where == STATUS_RIGHT)
			key = KEYC_MOUSEMOVE_STATUS_RIGHT;
		if (where == STATUS_DEFAULT)
			key = KEYC_MOUSEMOVE_STATUS_DEFAULT;
		if (where == BORDER)
			key = KEYC_MOUSEMOVE_BORDER;
		break;
	case DRAG:
		if (c->tty.mouse_drag_update != NULL)
			key = KEYC_DRAGGING;
		else {
			switch (MOUSE_BUTTONS(b)) {
			case 0:
				if (where == PANE)
					key = KEYC_MOUSEDRAG1_PANE;
				if (where == STATUS)
					key = KEYC_MOUSEDRAG1_STATUS;
				if (where == STATUS_LEFT)
					key = KEYC_MOUSEDRAG1_STATUS_LEFT;
				if (where == STATUS_RIGHT)
					key = KEYC_MOUSEDRAG1_STATUS_RIGHT;
				if (where == STATUS_DEFAULT)
					key = KEYC_MOUSEDRAG1_STATUS_DEFAULT;
				if (where == BORDER)
					key = KEYC_MOUSEDRAG1_BORDER;
				break;
			case 1:
				if (where == PANE)
					key = KEYC_MOUSEDRAG2_PANE;
				if (where == STATUS)
					key = KEYC_MOUSEDRAG2_STATUS;
				if (where == STATUS_LEFT)
					key = KEYC_MOUSEDRAG2_STATUS_LEFT;
				if (where == STATUS_RIGHT)
					key = KEYC_MOUSEDRAG2_STATUS_RIGHT;
				if (where == STATUS_DEFAULT)
					key = KEYC_MOUSEDRAG2_STATUS_DEFAULT;
				if (where == BORDER)
					key = KEYC_MOUSEDRAG2_BORDER;
				break;
			case 2:
				if (where == PANE)
					key = KEYC_MOUSEDRAG3_PANE;
				if (where == STATUS)
					key = KEYC_MOUSEDRAG3_STATUS;
				if (where == STATUS_LEFT)
					key = KEYC_MOUSEDRAG3_STATUS_LEFT;
				if (where == STATUS_RIGHT)
					key = KEYC_MOUSEDRAG3_STATUS_RIGHT;
				if (where == STATUS_DEFAULT)
					key = KEYC_MOUSEDRAG3_STATUS_DEFAULT;
				if (where == BORDER)
					key = KEYC_MOUSEDRAG3_BORDER;
				break;
			}
		}

		/*
		 * Begin a drag by setting the flag to a non-zero value that
		 * corresponds to the mouse button in use.
		 */
		c->tty.mouse_drag_flag = MOUSE_BUTTONS(b) + 1;
		break;
	case WHEEL:
		if (MOUSE_BUTTONS(b) == MOUSE_WHEEL_UP) {
			if (where == PANE)
				key = KEYC_WHEELUP_PANE;
			if (where == STATUS)
				key = KEYC_WHEELUP_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_WHEELUP_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_WHEELUP_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_WHEELUP_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_WHEELUP_BORDER;
		} else {
			if (where == PANE)
				key = KEYC_WHEELDOWN_PANE;
			if (where == STATUS)
				key = KEYC_WHEELDOWN_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_WHEELDOWN_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_WHEELDOWN_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_WHEELDOWN_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_WHEELDOWN_BORDER;
		}
		break;
	case UP:
		switch (MOUSE_BUTTONS(b)) {
		case 0:
			if (where == PANE)
				key = KEYC_MOUSEUP1_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEUP1_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEUP1_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEUP1_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEUP1_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEUP1_BORDER;
			break;
		case 1:
			if (where == PANE)
				key = KEYC_MOUSEUP2_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEUP2_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEUP2_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEUP2_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEUP2_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEUP2_BORDER;
			break;
		case 2:
			if (where == PANE)
				key = KEYC_MOUSEUP3_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEUP3_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEUP3_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEUP3_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEUP3_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEUP3_BORDER;
			break;
		}
		break;
	case DOWN:
		switch (MOUSE_BUTTONS(b)) {
		case 0:
			if (where == PANE)
				key = KEYC_MOUSEDOWN1_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEDOWN1_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEDOWN1_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEDOWN1_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEDOWN1_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEDOWN1_BORDER;
			break;
		case 1:
			if (where == PANE)
				key = KEYC_MOUSEDOWN2_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEDOWN2_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEDOWN2_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEDOWN2_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEDOWN2_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEDOWN2_BORDER;
			break;
		case 2:
			if (where == PANE)
				key = KEYC_MOUSEDOWN3_PANE;
			if (where == STATUS)
				key = KEYC_MOUSEDOWN3_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_MOUSEDOWN3_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_MOUSEDOWN3_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_MOUSEDOWN3_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_MOUSEDOWN3_BORDER;
			break;
		}
		break;
	case SECOND:
		switch (MOUSE_BUTTONS(b)) {
		case 0:
			if (where == PANE)
				key = KEYC_SECONDCLICK1_PANE;
			if (where == STATUS)
				key = KEYC_SECONDCLICK1_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_SECONDCLICK1_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_SECONDCLICK1_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_SECONDCLICK1_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_SECONDCLICK1_BORDER;
			break;
		case 1:
			if (where == PANE)
				key = KEYC_SECONDCLICK2_PANE;
			if (where == STATUS)
				key = KEYC_SECONDCLICK2_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_SECONDCLICK2_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_SECONDCLICK2_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_SECONDCLICK2_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_SECONDCLICK2_BORDER;
			break;
		case 2:
			if (where == PANE)
				key = KEYC_SECONDCLICK3_PANE;
			if (where == STATUS)
				key = KEYC_SECONDCLICK3_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_SECONDCLICK3_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_SECONDCLICK3_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_SECONDCLICK3_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_SECONDCLICK3_BORDER;
			break;
		}
		break;
	case DOUBLE:
		switch (MOUSE_BUTTONS(b)) {
		case 0:
			if (where == PANE)
				key = KEYC_DOUBLECLICK1_PANE;
			if (where == STATUS)
				key = KEYC_DOUBLECLICK1_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_DOUBLECLICK1_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_DOUBLECLICK1_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_DOUBLECLICK1_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_DOUBLECLICK1_BORDER;
			break;
		case 1:
			if (where == PANE)
				key = KEYC_DOUBLECLICK2_PANE;
			if (where == STATUS)
				key = KEYC_DOUBLECLICK2_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_DOUBLECLICK2_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_DOUBLECLICK2_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_DOUBLECLICK2_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_DOUBLECLICK2_BORDER;
			break;
		case 2:
			if (where == PANE)
				key = KEYC_DOUBLECLICK3_PANE;
			if (where == STATUS)
				key = KEYC_DOUBLECLICK3_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_DOUBLECLICK3_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_DOUBLECLICK3_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_DOUBLECLICK3_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_DOUBLECLICK3_BORDER;
			break;
		}
		break;
	case TRIPLE:
		switch (MOUSE_BUTTONS(b)) {
		case 0:
			if (where == PANE)
				key = KEYC_TRIPLECLICK1_PANE;
			if (where == STATUS)
				key = KEYC_TRIPLECLICK1_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_TRIPLECLICK1_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_TRIPLECLICK1_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_TRIPLECLICK1_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_TRIPLECLICK1_BORDER;
			break;
		case 1:
			if (where == PANE)
				key = KEYC_TRIPLECLICK2_PANE;
			if (where == STATUS)
				key = KEYC_TRIPLECLICK2_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_TRIPLECLICK2_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_TRIPLECLICK2_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_TRIPLECLICK2_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_TRIPLECLICK2_BORDER;
			break;
		case 2:
			if (where == PANE)
				key = KEYC_TRIPLECLICK3_PANE;
			if (where == STATUS)
				key = KEYC_TRIPLECLICK3_STATUS;
			if (where == STATUS_LEFT)
				key = KEYC_TRIPLECLICK3_STATUS_LEFT;
			if (where == STATUS_RIGHT)
				key = KEYC_TRIPLECLICK3_STATUS_RIGHT;
			if (where == STATUS_DEFAULT)
				key = KEYC_TRIPLECLICK3_STATUS_DEFAULT;
			if (where == BORDER)
				key = KEYC_TRIPLECLICK3_BORDER;
			break;
		}
		break;
	}
	if (key == KEYC_UNKNOWN)
		return (KEYC_UNKNOWN);

out:
	/* Apply modifiers if any. */
	if (b & MOUSE_MASK_META)
		key |= KEYC_META;
	if (b & MOUSE_MASK_CTRL)
		key |= KEYC_CTRL;
	if (b & MOUSE_MASK_SHIFT)
		key |= KEYC_SHIFT;

	if (log_get_level() != 0)
		log_debug("mouse key is %s", key_string_lookup_key (key, 1));
	return (key);
}


// Source: server-client.c
// Lines 557-1165
