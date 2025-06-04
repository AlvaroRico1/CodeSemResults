c_parser_struct_declaration (c_parser *parser)
{
  struct c_declspecs *specs;
  tree prefix_attrs;
  tree all_prefix_attrs;
  tree decls;
  location_t decl_loc;
  if (c_parser_next_token_is_keyword (parser, RID_EXTENSION))
    {
      int ext;
      tree decl;
      ext = disable_extension_diagnostics ();
      c_parser_consume_token (parser);
      decl = c_parser_struct_declaration (parser);
      restore_extension_diagnostics (ext);
      return decl;
    }
  if (c_parser_next_token_is_keyword (parser, RID_STATIC_ASSERT))
    {
      c_parser_static_assert_declaration_no_semi (parser);
      return NULL_TREE;
    }
  specs = build_null_declspecs ();
  decl_loc = c_parser_peek_token (parser)->location;
  /* Strictly by the standard, we shouldn't allow _Alignas here,
     but it appears to have been intended to allow it there, so
     we're keeping it as it is until WG14 reaches a conclusion
     of N1731.
     <http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1731.pdf>  */
  c_parser_declspecs (parser, specs, false, true, true,
		      true, false, true, true, cla_nonabstract_decl);
  if (parser->error)
    return NULL_TREE;
  if (!specs->declspecs_seen_p)
    {
      c_parser_error (parser, "expected specifier-qualifier-list");
      return NULL_TREE;
    }
  finish_declspecs (specs);
  if (c_parser_next_token_is (parser, CPP_SEMICOLON)
      || c_parser_next_token_is (parser, CPP_CLOSE_BRACE))
    {
      tree ret;
      if (specs->typespec_kind == ctsk_none)
	{
	  pedwarn (decl_loc, OPT_Wpedantic,
		   "ISO C forbids member declarations with no members");
	  shadow_tag_warned (specs, pedantic);
	  ret = NULL_TREE;
	}
      else
	{
	  /* Support for unnamed structs or unions as members of
	     structs or unions (which is [a] useful and [b] supports
	     MS P-SDK).  */
	  tree attrs = NULL;

	  ret = grokfield (c_parser_peek_token (parser)->location,
			   build_id_declarator (NULL_TREE), specs,
			   NULL_TREE, &attrs);
	  if (ret)
	    decl_attributes (&ret, attrs, 0);
	}
      return ret;
    }

  /* Provide better error recovery.  Note that a type name here is valid,
     and will be treated as a field name.  */
  if (specs->typespec_kind == ctsk_tagdef
      && TREE_CODE (specs->type) != ENUMERAL_TYPE
      && c_parser_next_token_starts_declspecs (parser)
      && !c_parser_next_token_is (parser, CPP_NAME))
    {
      c_parser_error (parser, "expected %<;%>, identifier or %<(%>");
      parser->error = false;
      return NULL_TREE;
    }

  pending_xref_error ();
  prefix_attrs = specs->attrs;
  all_prefix_attrs = prefix_attrs;
  specs->attrs = NULL_TREE;
  decls = NULL_TREE;
  while (true)
    {
      /* Declaring one or more declarators or un-named bit-fields.  */
      struct c_declarator *declarator;
      bool dummy = false;
      if (c_parser_next_token_is (parser, CPP_COLON))
	declarator = build_id_declarator (NULL_TREE);
      else
	declarator = c_parser_declarator (parser,
					  specs->typespec_kind != ctsk_none,
					  C_DTR_NORMAL, &dummy);
      if (declarator == NULL)
	{
	  c_parser_skip_to_end_of_block_or_statement (parser);
	  break;
	}
      if (c_parser_next_token_is (parser, CPP_COLON)
	  || c_parser_next_token_is (parser, CPP_COMMA)
	  || c_parser_next_token_is (parser, CPP_SEMICOLON)
	  || c_parser_next_token_is (parser, CPP_CLOSE_BRACE)
	  || c_parser_next_token_is_keyword (parser, RID_ATTRIBUTE))
	{
	  tree postfix_attrs = NULL_TREE;
	  tree width = NULL_TREE;
	  tree d;
	  if (c_parser_next_token_is (parser, CPP_COLON))
	    {
	      c_parser_consume_token (parser);
	      width = c_parser_expr_no_commas (parser, NULL).value;
	    }
	  if (c_parser_next_token_is_keyword (parser, RID_ATTRIBUTE))
	    postfix_attrs = c_parser_gnu_attributes (parser);
	  d = grokfield (c_parser_peek_token (parser)->location,
			 declarator, specs, width, &all_prefix_attrs);
	  decl_attributes (&d, chainon (postfix_attrs,
					all_prefix_attrs), 0);
	  DECL_CHAIN (d) = decls;
	  decls = d;
	  if (c_parser_next_token_is_keyword (parser, RID_ATTRIBUTE))
	    all_prefix_attrs = chainon (c_parser_gnu_attributes (parser),
					prefix_attrs);
	  else
	    all_prefix_attrs = prefix_attrs;
	  if (c_parser_next_token_is (parser, CPP_COMMA))
	    c_parser_consume_token (parser);
	  else if (c_parser_next_token_is (parser, CPP_SEMICOLON)
		   || c_parser_next_token_is (parser, CPP_CLOSE_BRACE))
	    {
	      /* Semicolon consumed in caller.  */
	      break;
	    }
	  else
	    {
	      c_parser_error (parser, "expected %<,%>, %<;%> or %<}%>");
	      break;
	    }


// Source: c-parser.c
// Lines 3502-3640
