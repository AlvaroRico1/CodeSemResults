builtin_macro (cpp_reader *pfile, cpp_hashnode *node,
	       location_t loc, location_t expand_loc)
{
  const uchar *buf;
  size_t len;
  char *nbuf;

  if (node->value.builtin == BT_PRAGMA)
    {
      /* Don't interpret _Pragma within directives.  The standard is
         not clear on this, but to me this makes most sense.  */
      if (pfile->state.in_directive)
	return 0;

      return _cpp_do__Pragma (pfile, loc);
    }

  buf = _cpp_builtin_macro_text (pfile, node, expand_loc);
  len = ustrlen (buf);
  nbuf = (char *) alloca (len + 1);
  memcpy (nbuf, buf, len);
  nbuf[len]='\n';

  cpp_push_buffer (pfile, (uchar *) nbuf, len, /* from_stage3 */ true);
  _cpp_clean_line (pfile);

  /* Set pfile->cur_token as required by _cpp_lex_direct.  */
  pfile->cur_token = _cpp_temp_token (pfile);
  cpp_token *token = _cpp_lex_direct (pfile);
  /* We should point to the expansion point of the builtin macro.  */
  token->src_loc = loc;
  if (pfile->context->tokens_kind == TOKENS_KIND_EXTENDED)
    {
      /* We are tracking tokens resulting from macro expansion.
	 Create a macro line map and generate a virtual location for
	 the token resulting from the expansion of the built-in
	 macro.  */
      location_t *virt_locs = NULL;
      _cpp_buff *token_buf = tokens_buff_new (pfile, 1, &virt_locs);
      const line_map_macro * map =
	linemap_enter_macro (pfile->line_table, node, loc, 1);
      tokens_buff_add_token (token_buf, virt_locs, token,
			     pfile->line_table->builtin_location,
			     pfile->line_table->builtin_location,
			    map, /*macro_token_index=*/0);
      push_extended_tokens_context (pfile, node, token_buf, virt_locs,
				    (const cpp_token **)token_buf->base,
				    1);
    }
  else
    _cpp_push_token_context (pfile, NULL, token, 1);
  if (pfile->buffer->cur != pfile->buffer->rlimit)
    cpp_error (pfile, CPP_DL_ICE, "invalid built-in macro \"%s\"",
	       NODE_NAME (node));
  _cpp_pop_buffer (pfile);

  return 1;
}


// Source: macro.c
// Lines 695-752
