parse_assertion (cpp_reader *pfile, int type, cpp_macro **answer_ptr)
{
  cpp_hashnode *result = 0;

  /* We don't expand predicates or answers.  */
  pfile->state.prevent_expansion++;

  *answer_ptr = NULL;

  const cpp_token *predicate = cpp_get_token (pfile);
  if (predicate->type == CPP_EOF)
    cpp_error (pfile, CPP_DL_ERROR, "assertion without predicate");
  else if (predicate->type != CPP_NAME)
    cpp_error_with_line (pfile, CPP_DL_ERROR, predicate->src_loc, 0,
			 "predicate must be an identifier");
  else if (parse_answer (pfile, type, predicate->src_loc, answer_ptr))
    {
      unsigned int len = NODE_LEN (predicate->val.node.node);
      unsigned char *sym = (unsigned char *) alloca (len + 1);

      /* Prefix '#' to get it out of macro namespace.  */
      sym[0] = '#';
      memcpy (sym + 1, NODE_NAME (predicate->val.node.node), len);
      result = cpp_lookup (pfile, sym, len + 1);
    }


// Source: directives.c
// Lines 2227-2251
