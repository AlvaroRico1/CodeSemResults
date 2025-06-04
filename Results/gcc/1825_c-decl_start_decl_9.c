start_decl (struct c_declarator *declarator, struct c_declspecs *declspecs,
	    bool initialized, tree attributes)
{
  tree decl;
  tree tem;
  tree expr = NULL_TREE;
  enum deprecated_states deprecated_state = DEPRECATED_NORMAL;

  /* An object declared as __attribute__((deprecated)) suppresses
     warnings of uses of other deprecated items.  */
  if (lookup_attribute ("deprecated", attributes))
    deprecated_state = DEPRECATED_SUPPRESS;

  decl = grokdeclarator (declarator, declspecs,
			 NORMAL, initialized, NULL, &attributes, &expr, NULL,
			 deprecated_state);
  if (!decl || decl == error_mark_node)
    return NULL_TREE;

  if (expr)
    add_stmt (fold_convert (void_type_node, expr));

  if (TREE_CODE (decl) != FUNCTION_DECL && MAIN_NAME_P (DECL_NAME (decl))
      && TREE_PUBLIC (decl))
    warning (OPT_Wmain, "%q+D is usually a function", decl);

  if (initialized)
    /* Is it valid for this decl to have an initializer at all?
       If not, set INITIALIZED to zero, which will indirectly
       tell 'finish_decl' to ignore the initializer once it is parsed.  */
    switch (TREE_CODE (decl))
      {
      case TYPE_DECL:
	error ("typedef %qD is initialized (use %<__typeof__%> instead)", decl);
	initialized = false;
	break;

      case FUNCTION_DECL:
	error ("function %qD is initialized like a variable", decl);
	initialized = false;
	break;

      case PARM_DECL:
	/* DECL_INITIAL in a PARM_DECL is really DECL_ARG_TYPE.  */
	error ("parameter %qD is initialized", decl);
	initialized = false;
	break;

      default:
	/* Don't allow initializations for incomplete types except for
	   arrays which might be completed by the initialization.  */

	/* This can happen if the array size is an undefined macro.
	   We already gave a warning, so we don't need another one.  */
	if (TREE_TYPE (decl) == error_mark_node)
	  initialized = false;
	else if (COMPLETE_TYPE_P (TREE_TYPE (decl)))
	  {
	    /* A complete type is ok if size is fixed.  */

	    if (!poly_int_tree_p (TYPE_SIZE (TREE_TYPE (decl)))
		|| C_DECL_VARIABLE_SIZE (decl))
	      {
		error ("variable-sized object may not be initialized");
		initialized = false;
	      }
	  }
	else if (TREE_CODE (TREE_TYPE (decl)) != ARRAY_TYPE)
	  {
	    error ("variable %qD has initializer but incomplete type", decl);
	    initialized = false;
	  }
	else if (C_DECL_VARIABLE_SIZE (decl))
	  {
	    /* Although C99 is unclear about whether incomplete arrays
	       of VLAs themselves count as VLAs, it does not make
	       sense to permit them to be initialized given that
	       ordinary VLAs may not be initialized.  */
	    error ("variable-sized object may not be initialized");
	    initialized = false;
	  }
      }

  if (initialized)
    {
      if (current_scope == file_scope)
	TREE_STATIC (decl) = 1;

      /* Tell 'pushdecl' this is an initialized decl
	 even though we don't yet have the initializer expression.
	 Also tell 'finish_decl' it may store the real initializer.  */
      DECL_INITIAL (decl) = error_mark_node;
    }

  /* If this is a function declaration, write a record describing it to the
     prototypes file (if requested).  */

  if (TREE_CODE (decl) == FUNCTION_DECL)
    gen_aux_info_record (decl, 0, 0, prototype_p (TREE_TYPE (decl)));

  /* ANSI specifies that a tentative definition which is not merged with
     a non-tentative definition behaves exactly like a definition with an
     initializer equal to zero.  (Section 3.7.2)

     -fno-common gives strict ANSI behavior, though this tends to break
     a large body of code that grew up without this rule.

     Thread-local variables are never common, since there's no entrenched
     body of code to break, and it allows more efficient variable references
     in the presence of dynamic linking.  */

  if (VAR_P (decl)
      && !initialized
      && TREE_PUBLIC (decl)
      && !DECL_THREAD_LOCAL_P (decl)
      && !flag_no_common)
    DECL_COMMON (decl) = 1;

  /* Set attributes here so if duplicate decl, will have proper attributes.  */
  c_decl_attributes (&decl, attributes, 0);

  /* Handle gnu_inline attribute.  */
  if (declspecs->inline_p
      && !flag_gnu89_inline
      && TREE_CODE (decl) == FUNCTION_DECL
      && (lookup_attribute ("gnu_inline", DECL_ATTRIBUTES (decl))
	  || current_function_decl))
    {
      if (declspecs->storage_class == csc_auto && current_scope != file_scope)
	;
      else if (declspecs->storage_class != csc_static)
	DECL_EXTERNAL (decl) = !DECL_EXTERNAL (decl);
    }

  if (TREE_CODE (decl) == FUNCTION_DECL
      && targetm.calls.promote_prototypes (TREE_TYPE (decl)))
    {
      struct c_declarator *ce = declarator;

      if (ce->kind == cdk_pointer)
	ce = declarator->declarator;
      if (ce->kind == cdk_function)
	{
	  tree args = ce->u.arg_info->parms;
	  for (; args; args = DECL_CHAIN (args))
	    {
	      tree type = TREE_TYPE (args);
	      if (type && INTEGRAL_TYPE_P (type)
		  && TYPE_PRECISION (type) < TYPE_PRECISION (integer_type_node))
		DECL_ARG_TYPE (args) = c_type_promotes_to (type);
	    }
	}
    }

  if (TREE_CODE (decl) == FUNCTION_DECL
      && DECL_DECLARED_INLINE_P (decl)
      && DECL_UNINLINABLE (decl)
      && lookup_attribute ("noinline", DECL_ATTRIBUTES (decl)))
    warning (OPT_Wattributes, "inline function %q+D given attribute %qs",
	     decl, "noinline");

  /* C99 6.7.4p3: An inline definition of a function with external
     linkage shall not contain a definition of a modifiable object
     with static storage duration...  */
  if (VAR_P (decl)
      && current_scope != file_scope
      && TREE_STATIC (decl)
      && !TREE_READONLY (decl)
      && DECL_DECLARED_INLINE_P (current_function_decl)
      && DECL_EXTERNAL (current_function_decl))
    record_inline_static (input_location, current_function_decl,
			  decl, csi_modifiable);

  if (c_dialect_objc ()
      && VAR_OR_FUNCTION_DECL_P (decl))
      objc_check_global_decl (decl);

  /* Add this decl to the current scope.
     TEM may equal DECL or it may be a previous decl of the same name.  */
  tem = pushdecl (decl);

  if (initialized && DECL_EXTERNAL (tem))
    {
      DECL_EXTERNAL (tem) = 0;
      TREE_STATIC (tem) = 1;
    }

  return tem;
}


// Source: c-decl.c
// Lines 4998-5186
