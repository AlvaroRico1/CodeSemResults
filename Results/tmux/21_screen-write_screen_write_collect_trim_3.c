screen_write_collect_trim(struct screen_write_ctx *ctx, u_int y, u_int x,
    u_int used, int *wrapped)
{
	struct screen_write_cline	*cl = &ctx->s->write_list[y];
	struct screen_write_citem	*ci, *ci2, *tmp, *before = NULL;
	u_int				 sx = x, ex = x + used - 1;
	u_int				 csx, cex;

	if (TAILQ_EMPTY(&cl->items))
		return (NULL);
	TAILQ_FOREACH_SAFE(ci, &cl->items, entry, tmp) {
		csx = ci->x;
		cex = ci->x + ci->used - 1;

		/* Item is entirely before. */
		if (cex < sx) {
			log_debug("%s: %p %u-%u before %u-%u", __func__, ci,
			    csx, cex, sx, ex);
			continue;
		}

		/* Item is entirely after. */
		if (csx > ex) {
			log_debug("%s: %p %u-%u after %u-%u", __func__, ci,
			    csx, cex, sx, ex);
			before = ci;
			break;
		}

		/* Item is entirely inside. */
		if (csx >= sx && cex <= ex) {
			log_debug("%s: %p %u-%u inside %u-%u", __func__, ci,
			    csx, cex, sx, ex);
			TAILQ_REMOVE(&cl->items, ci, entry);
			screen_write_free_citem(ci);
			if (csx == 0 && ci->wrapped && wrapped != NULL)
				*wrapped = 1;
			continue;
		}

		/* Item under the start. */
		if (csx < sx && cex >= sx && cex <= ex) {
			log_debug("%s: %p %u-%u start %u-%u", __func__, ci,
			    csx, cex, sx, ex);
			ci->used = sx - csx;
			log_debug("%s: %p now %u-%u", __func__, ci, ci->x,
			    ci->x + ci->used + 1);
			continue;
		}

		/* Item covers the end. */
		if (cex > ex && csx >= sx && csx <= ex) {
			log_debug("%s: %p %u-%u end %u-%u", __func__, ci,
			    csx, cex, sx, ex);
			ci->x = ex + 1;
			ci->used = cex - ex;
			log_debug("%s: %p now %u-%u", __func__, ci, ci->x,
			    ci->x + ci->used + 1);
			before = ci;
			break;
		}

		/* Item must cover both sides. */
		log_debug("%s: %p %u-%u under %u-%u", __func__, ci,
		    csx, cex, sx, ex);
		ci2 = screen_write_get_citem();
		ci2->type = ci->type;
		ci2->bg = ci->bg;
		memcpy(&ci2->gc, &ci->gc, sizeof ci2->gc);
		TAILQ_INSERT_AFTER(&cl->items, ci, ci2, entry);

		ci->used = sx - csx;
		ci2->x = ex + 1;
		ci2->used = cex - ex;

		log_debug("%s: %p now %u-%u (%p) and %u-%u (%p)", __func__, ci,
		    ci->x, ci->x + ci->used - 1, ci, ci2->x,
		    ci2->x + ci2->used - 1, ci2);
		before = ci2;
		break;
	}
	return (before);
}


// Source: screen-write.c
// Lines 1504-1586
