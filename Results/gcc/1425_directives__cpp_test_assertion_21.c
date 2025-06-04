_cpp_test_assertion (cpp_reader *pfile, unsigned int *value)
{
  cpp_macro *answer;
  cpp_hashnode *node = parse_assertion (pfile, T_IF, &answer);

  /* For recovery, an erroneous assertion expression is handled as a
     failing assertion.  */
  *value = 0;

  if (node)
    {
      if (node->value.answers)
	*value = !answer || *find_answer (node, answer);
    }
  else if (pfile->cur_token[-1].type == CPP_EOF)
    _cpp_backup_tokens (pfile, 1);

  /* We don't commit the memory for the answer - it's temporary only.  */
  return node == 0;
}


// Source: directives.c
// Lines 2289-2308
