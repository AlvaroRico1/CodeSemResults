omp_notice_variable (struct gimplify_omp_ctx *ctx, tree decl, bool in_code)
{
  splay_tree_node n;
  unsigned flags = in_code ? GOVD_SEEN : 0;
  bool ret = false, shared;

  if (error_operand_p (decl))
    return false;

  if (ctx->region_type == ORT_NONE)
    return lang_hooks.decls.omp_disregard_value_expr (decl, false);

  if (is_global_var (decl))
    {
      /* Threadprivate variables are predetermined.  */
      if (DECL_THREAD_LOCAL_P (decl))
	return omp_notice_threadprivate_variable (ctx, decl, NULL_TREE);

      if (DECL_HAS_VALUE_EXPR_P (decl))
	{
	  if (ctx->region_type & ORT_ACC)
	    /* For OpenACC, defer expansion of value to avoid transfering
	       privatized common block data instead of im-/explicitly transfered
	       variables which are in common blocks.  */
	    ;
	  else
	    {
	      tree value = get_base_address (DECL_VALUE_EXPR (decl));

	      if (value && DECL_P (value) && DECL_THREAD_LOCAL_P (value))
		return omp_notice_threadprivate_variable (ctx, decl, value);
	    }
	}

      if (gimplify_omp_ctxp->outer_context == NULL
	  && VAR_P (decl)
	  && oacc_get_fn_attrib (current_function_decl))
	{
	  location_t loc = DECL_SOURCE_LOCATION (decl);

	  if (lookup_attribute ("omp declare target link",
				DECL_ATTRIBUTES (decl)))
	    {
	      error_at (loc,
			"%qE with %<link%> clause used in %<routine%> function",
			DECL_NAME (decl));
	      return false;
	    }
	  else if (!lookup_attribute ("omp declare target",
				      DECL_ATTRIBUTES (decl)))
	    {
	      error_at (loc,
			"%qE requires a %<declare%> directive for use "
			"in a %<routine%> function", DECL_NAME (decl));
	      return false;
	    }
	}
    }

  n = splay_tree_lookup (ctx->variables, (splay_tree_key)decl);
  if ((ctx->region_type & ORT_TARGET) != 0)
    {
      if (ctx->region_type & ORT_ACC)
	/* For OpenACC, as remarked above, defer expansion.  */
	shared = false;
      else
	shared = true;

      ret = lang_hooks.decls.omp_disregard_value_expr (decl, shared);
      if (n == NULL)
	{
	  unsigned nflags = flags;
	  if ((ctx->region_type & ORT_ACC) == 0)
	    {
	      bool is_declare_target = false;
	      if (is_global_var (decl)
		  && varpool_node::get_create (decl)->offloadable)
		{
		  struct gimplify_omp_ctx *octx;
		  for (octx = ctx->outer_context;
		       octx; octx = octx->outer_context)
		    {
		      n = splay_tree_lookup (octx->variables,
					     (splay_tree_key)decl);
		      if (n
			  && (n->value & GOVD_DATA_SHARE_CLASS) != GOVD_SHARED
			  && (n->value & GOVD_DATA_SHARE_CLASS) != 0)
			break;
		    }
		  is_declare_target = octx == NULL;
		}
	      if (!is_declare_target)
		{
		  int gdmk;
		  if (TREE_CODE (TREE_TYPE (decl)) == POINTER_TYPE
		      || (TREE_CODE (TREE_TYPE (decl)) == REFERENCE_TYPE
			  && (TREE_CODE (TREE_TYPE (TREE_TYPE (decl)))
			      == POINTER_TYPE)))
		    gdmk = GDMK_POINTER;
		  else if (lang_hooks.decls.omp_scalar_p (decl))
		    gdmk = GDMK_SCALAR;
		  else
		    gdmk = GDMK_AGGREGATE;
		  if (ctx->defaultmap[gdmk] == 0)
		    {
		      tree d = lang_hooks.decls.omp_report_decl (decl);
		      error ("%qE not specified in enclosing %<target%>",
			     DECL_NAME (d));
		      error_at (ctx->location, "enclosing %<target%>");
		    }
		  else if (ctx->defaultmap[gdmk]
			   & (GOVD_MAP_0LEN_ARRAY | GOVD_FIRSTPRIVATE))
		    nflags |= ctx->defaultmap[gdmk];
		  else
		    {
		      gcc_assert (ctx->defaultmap[gdmk] & GOVD_MAP);
		      nflags |= ctx->defaultmap[gdmk] & ~GOVD_MAP;
		    }
		}
	    }

	  struct gimplify_omp_ctx *octx = ctx->outer_context;
	  if ((ctx->region_type & ORT_ACC) && octx)
	    {
	      /* Look in outer OpenACC contexts, to see if there's a
		 data attribute for this variable.  */
	      omp_notice_variable (octx, decl, in_code);

	      for (; octx; octx = octx->outer_context)
		{
		  if (!(octx->region_type & (ORT_TARGET_DATA | ORT_TARGET)))
		    break;
		  splay_tree_node n2
		    = splay_tree_lookup (octx->variables,
					 (splay_tree_key) decl);
		  if (n2)
		    {
		      if (octx->region_type == ORT_ACC_HOST_DATA)
		        error ("variable %qE declared in enclosing "
			       "%<host_data%> region", DECL_NAME (decl));
		      nflags |= GOVD_MAP;
		      if (octx->region_type == ORT_ACC_DATA
			  && (n2->value & GOVD_MAP_0LEN_ARRAY))
			nflags |= GOVD_MAP_0LEN_ARRAY;
		      goto found_outer;
		    }
		}
	    }

	  if ((nflags & ~(GOVD_MAP_TO_ONLY | GOVD_MAP_FROM_ONLY
			  | GOVD_MAP_ALLOC_ONLY)) == flags)
	    {
	      tree type = TREE_TYPE (decl);

	      if (gimplify_omp_ctxp->target_firstprivatize_array_bases
		  && lang_hooks.decls.omp_privatize_by_reference (decl))
		type = TREE_TYPE (type);
	      if (!lang_hooks.types.omp_mappable_type (type))
		{
		  error ("%qD referenced in target region does not have "
			 "a mappable type", decl);
		  nflags |= GOVD_MAP | GOVD_EXPLICIT;
		}
	      else
		{
		  if ((ctx->region_type & ORT_ACC) != 0)
		    nflags = oacc_default_clause (ctx, decl, flags);
		  else
		    nflags |= GOVD_MAP;
		}
	    }
	found_outer:
	  omp_add_variable (ctx, decl, nflags);
	}


// Source: gimplify.c
// Lines 7372-7545
