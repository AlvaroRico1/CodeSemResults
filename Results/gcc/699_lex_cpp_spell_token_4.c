cpp_spell_token (cpp_reader *pfile, const cpp_token *token,
		 unsigned char *buffer, bool forstring)
{
  switch (TOKEN_SPELL (token))
    {
    case SPELL_OPERATOR:
      {
	const unsigned char *spelling;
	unsigned char c;

	if (token->flags & DIGRAPH)
	  spelling = cpp_digraph2name (token->type);
	else if (token->flags & NAMED_OP)
	  goto spell_ident;
	else
	  spelling = TOKEN_NAME (token);

	while ((c = *spelling++) != '\0')
	  *buffer++ = c;
      }


// Source: lex.c
// Lines 3289-3308
