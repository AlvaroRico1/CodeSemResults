wcs_re_match_2_internal (struct re_pattern_buffer *bufp,
                         const char *cstring1, int csize1,
                         const char *cstring2, int csize2,
                         int pos,
			 struct re_registers *regs,
                         int stop,
     /* string1 == string2 == NULL means string1/2, size1/2 and
	mbs_offset1/2 need seting up in this function.  */
     /* We need wchar_t* buffers correspond to cstring1, cstring2.  */
                         wchar_t *string1, int size1,
                         wchar_t *string2, int size2,
     /* offset buffer for optimizatoin. See convert_mbs_to_wc.  */
			 int *mbs_offset1, int *mbs_offset2)
#else /* BYTE */
static int
byte_re_match_2_internal (struct re_pattern_buffer *bufp,
                          const char *string1, int size1,
                          const char *string2, int size2,
                          int pos,
			  struct re_registers *regs, int stop)
#endif /* BYTE */
{
  /* General temporaries.  */
  int mcnt;
  UCHAR_T *p1;
#ifdef WCHAR
  /* They hold whether each wchar_t is binary data or not.  */
  char *is_binary = NULL;
  /* If true, we can't free string1/2, mbs_offset1/2.  */
  int cant_free_wcs_buf = 1;
#endif /* WCHAR */

  /* Just past the end of the corresponding string.  */
  const CHAR_T *end1, *end2;

  /* Pointers into string1 and string2, just past the last characters in
     each to consider matching.  */
  const CHAR_T *end_match_1, *end_match_2;

  /* Where we are in the data, and the end of the current string.  */
  const CHAR_T *d, *dend;

  /* Where we are in the pattern, and the end of the pattern.  */
#ifdef WCHAR
  UCHAR_T *pattern, *p;
  register UCHAR_T *pend;
#else /* BYTE */
  UCHAR_T *p = bufp->buffer;
  register UCHAR_T *pend = p + bufp->used;
#endif /* WCHAR */

  /* Mark the opcode just after a start_memory, so we can test for an
     empty subpattern when we get to the stop_memory.  */
  UCHAR_T *just_past_start_mem = 0;

  /* We use this to map every character in the string.  */
  RE_TRANSLATE_TYPE translate = bufp->translate;

  /* Failure point stack.  Each place that can handle a failure further
     down the line pushes a failure point on this stack.  It consists of
     restart, regend, and reg_info for all registers corresponding to
     the subexpressions we're currently inside, plus the number of such
     registers, and, finally, two char *'s.  The first char * is where
     to resume scanning the pattern; the second one is where to resume
     scanning the strings.  If the latter is zero, the failure point is
     a ``dummy''; if a failure happens and the failure point is a dummy,
     it gets discarded and the next next one is tried.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, this is global.  */
  PREFIX(fail_stack_type) fail_stack;
#endif
#ifdef DEBUG
  static unsigned failure_id;
  unsigned nfailure_points_pushed = 0, nfailure_points_popped = 0;
#endif

#ifdef REL_ALLOC
  /* This holds the pointer to the failure stack, when
     it is allocated relocatably.  */
  fail_stack_elt_t *failure_stack_ptr;
#endif

  /* We fill all the registers internally, independent of what we
     return, for use in backreferences.  The number here includes
     an element for register zero.  */
  size_t num_regs = bufp->re_nsub + 1;

  /* The currently active registers.  */
  active_reg_t lowest_active_reg = NO_LOWEST_ACTIVE_REG;
  active_reg_t highest_active_reg = NO_HIGHEST_ACTIVE_REG;

  /* Information on the contents of registers. These are pointers into
     the input strings; they record just what was matched (on this
     attempt) by a subexpression part of the pattern, that is, the
     regnum-th regstart pointer points to where in the pattern we began
     matching and the regnum-th regend points to right after where we
     stopped matching the regnum-th subexpression.  (The zeroth register
     keeps track of what the whole pattern matches.)  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const CHAR_T **regstart, **regend;
#endif

  /* If a group that's operated upon by a repetition operator fails to
     match anything, then the register for its start will need to be
     restored because it will have been set to wherever in the string we
     are when we last see its open-group operator.  Similarly for a
     register's end.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const CHAR_T **old_regstart, **old_regend;
#endif

  /* The is_active field of reg_info helps us keep track of which (possibly
     nested) subexpressions we are currently in. The matched_something
     field of reg_info[reg_num] helps us tell whether or not we have
     matched any of the pattern so far this time through the reg_num-th
     subexpression.  These two fields get reset each time through any
     loop their register is in.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, this is global.  */
  PREFIX(register_info_type) *reg_info;
#endif

  /* The following record the register info as found in the above
     variables when we find a match better than any we've seen before.
     This happens as we backtrack through the failure points, which in
     turn happens only if we have not yet matched the entire string. */
  unsigned best_regs_set = false;
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const CHAR_T **best_regstart, **best_regend;
#endif

  /* Logically, this is `best_regend[0]'.  But we don't want to have to
     allocate space for that if we're not allocating space for anything
     else (see below).  Also, we never need info about register 0 for
     any of the other register vectors, and it seems rather a kludge to
     treat `best_regend' differently than the rest.  So we keep track of
     the end of the best match so far in a separate variable.  We
     initialize this to NULL so that when we backtrack the first time
     and need to test it, it's not garbage.  */
  const CHAR_T *match_end = NULL;

  /* This helps SET_REGS_MATCHED avoid doing redundant work.  */
  int set_regs_matched_done = 0;

  /* Used when we pop values we don't care about.  */
#ifdef MATCH_MAY_ALLOCATE /* otherwise, these are global.  */
  const CHAR_T **reg_dummy;
  PREFIX(register_info_type) *reg_info_dummy;
#endif

#ifdef DEBUG
  /* Counts the total number of registers pushed.  */
  unsigned num_regs_pushed = 0;
#endif

  DEBUG_PRINT1 ("\n\nEntering re_match_2.\n");

  INIT_FAIL_STACK ();

#ifdef MATCH_MAY_ALLOCATE
  /* Do not bother to initialize all the register variables if there are
     no groups in the pattern, as it takes a fair amount of time.  If
     there are groups, we include space for register 0 (the whole
     pattern), even though we never use it, since it simplifies the
     array indexing.  We should fix this.  */
  if (bufp->re_nsub)
    {
      regstart = REGEX_TALLOC (num_regs, const CHAR_T *);
      regend = REGEX_TALLOC (num_regs, const CHAR_T *);
      old_regstart = REGEX_TALLOC (num_regs, const CHAR_T *);
      old_regend = REGEX_TALLOC (num_regs, const CHAR_T *);
      best_regstart = REGEX_TALLOC (num_regs, const CHAR_T *);
      best_regend = REGEX_TALLOC (num_regs, const CHAR_T *);
      reg_info = REGEX_TALLOC (num_regs, PREFIX(register_info_type));
      reg_dummy = REGEX_TALLOC (num_regs, const CHAR_T *);
      reg_info_dummy = REGEX_TALLOC (num_regs, PREFIX(register_info_type));

      if (!(regstart && regend && old_regstart && old_regend && reg_info
            && best_regstart && best_regend && reg_dummy && reg_info_dummy))
        {
          FREE_VARIABLES ();
          return -2;
        }
    }
  else
    {
      /* We must initialize all our variables to NULL, so that
         `FREE_VARIABLES' doesn't try to free them.  */
      regstart = regend = old_regstart = old_regend = best_regstart
        = best_regend = reg_dummy = NULL;
      reg_info = reg_info_dummy = (PREFIX(register_info_type) *) NULL;
    }
#endif /* MATCH_MAY_ALLOCATE */

  /* The starting position is bogus.  */
#ifdef WCHAR
  if (pos < 0 || pos > csize1 + csize2)
#else /* BYTE */
  if (pos < 0 || pos > size1 + size2)
#endif
    {
      FREE_VARIABLES ();
      return -1;
    }

#ifdef WCHAR
  /* Allocate wchar_t array for string1 and string2 and
     fill them with converted string.  */
  if (string1 == NULL && string2 == NULL)
    {
      /* We need seting up buffers here.  */

      /* We must free wcs buffers in this function.  */
      cant_free_wcs_buf = 0;

      if (csize1 != 0)
	{
	  string1 = REGEX_TALLOC (csize1 + 1, CHAR_T);
	  mbs_offset1 = REGEX_TALLOC (csize1 + 1, int);
	  is_binary = REGEX_TALLOC (csize1 + 1, char);
	  if (!string1 || !mbs_offset1 || !is_binary)
	    {
	      FREE_VAR (string1);
	      FREE_VAR (mbs_offset1);
	      FREE_VAR (is_binary);
	      return -2;
	    }
	}
      if (csize2 != 0)
	{
	  string2 = REGEX_TALLOC (csize2 + 1, CHAR_T);
	  mbs_offset2 = REGEX_TALLOC (csize2 + 1, int);
	  is_binary = REGEX_TALLOC (csize2 + 1, char);
	  if (!string2 || !mbs_offset2 || !is_binary)
	    {
	      FREE_VAR (string1);
	      FREE_VAR (mbs_offset1);
	      FREE_VAR (string2);
	      FREE_VAR (mbs_offset2);
	      FREE_VAR (is_binary);
	      return -2;
	    }
	  size2 = convert_mbs_to_wcs(string2, cstring2, csize2,
				     mbs_offset2, is_binary);
	  string2[size2] = L'\0'; /* for a sentinel  */
	  FREE_VAR (is_binary);
	}
    }

  /* We need to cast pattern to (wchar_t*), because we casted this compiled
     pattern to (char*) in regex_compile.  */
  p = pattern = (CHAR_T*)bufp->buffer;
  pend = (CHAR_T*)(bufp->buffer + bufp->used);

#endif /* WCHAR */

  /* Initialize subexpression text positions to -1 to mark ones that no
     start_memory/stop_memory has been seen for. Also initialize the
     register information struct.  */
  for (mcnt = 1; (unsigned) mcnt < num_regs; mcnt++)
    {
      regstart[mcnt] = regend[mcnt]
        = old_regstart[mcnt] = old_regend[mcnt] = REG_UNSET_VALUE;

      REG_MATCH_NULL_STRING_P (reg_info[mcnt]) = MATCH_NULL_UNSET_VALUE;
      IS_ACTIVE (reg_info[mcnt]) = 0;
      MATCHED_SOMETHING (reg_info[mcnt]) = 0;
      EVER_MATCHED_SOMETHING (reg_info[mcnt]) = 0;
    }

  /* We move `string1' into `string2' if the latter's empty -- but not if
     `string1' is null.  */
  if (size2 == 0 && string1 != NULL)
    {
      string2 = string1;
      size2 = size1;
      string1 = 0;
      size1 = 0;
#ifdef WCHAR
      mbs_offset2 = mbs_offset1;
      csize2 = csize1;
      mbs_offset1 = NULL;
      csize1 = 0;
#endif
    }
  end1 = string1 + size1;
  end2 = string2 + size2;

  /* Compute where to stop matching, within the two strings.  */
#ifdef WCHAR
  if (stop <= csize1)
    {
      mcnt = count_mbs_length(mbs_offset1, stop);
      end_match_1 = string1 + mcnt;
      end_match_2 = string2;
    }
  else
    {
      if (stop > csize1 + csize2)
	stop = csize1 + csize2;
      end_match_1 = end1;
      mcnt = count_mbs_length(mbs_offset2, stop-csize1);
      end_match_2 = string2 + mcnt;
    }
  if (mcnt < 0)
    { /* count_mbs_length return error.  */
      FREE_VARIABLES ();
      return -1;
    }
#else
  if (stop <= size1)
    {
      end_match_1 = string1 + stop;
      end_match_2 = string2;
    }
  else
    {
      end_match_1 = end1;
      end_match_2 = string2 + stop - size1;
    }
#endif /* WCHAR */

  /* `p' scans through the pattern as `d' scans through the data.
     `dend' is the end of the input string that `d' points within.  `d'
     is advanced into the following input string whenever necessary, but
     this happens before fetching; therefore, at the beginning of the
     loop, `d' can be pointing at the end of a string, but it cannot
     equal `string2'.  */
#ifdef WCHAR
  if (size1 > 0 && pos <= csize1)
    {
      mcnt = count_mbs_length(mbs_offset1, pos);
      d = string1 + mcnt;
      dend = end_match_1;
    }
  else
    {
      mcnt = count_mbs_length(mbs_offset2, pos-csize1);
      d = string2 + mcnt;
      dend = end_match_2;
    }

  if (mcnt < 0)
    { /* count_mbs_length return error.  */
      FREE_VARIABLES ();
      return -1;
    }
#else
  if (size1 > 0 && pos <= size1)
    {
      d = string1 + pos;
      dend = end_match_1;
    }
  else
    {
      d = string2 + pos - size1;
      dend = end_match_2;
    }
#endif /* WCHAR */

  DEBUG_PRINT1 ("The compiled pattern is:\n");
  DEBUG_PRINT_COMPILED_PATTERN (bufp, p, pend);
  DEBUG_PRINT1 ("The string to match is: `");
  DEBUG_PRINT_DOUBLE_STRING (d, string1, size1, string2, size2);
  DEBUG_PRINT1 ("'\n");

  /* This loops over pattern commands.  It exits by returning from the
     function if the match is complete, or it drops through if the match
     fails at this starting point in the input data.  */
  for (;;)
    {
#ifdef _LIBC
      DEBUG_PRINT2 ("\n%p: ", p);
#else
      DEBUG_PRINT2 ("\n0x%x: ", p);
#endif

      if (p == pend)
	{ /* End of pattern means we might have succeeded.  */
          DEBUG_PRINT1 ("end of pattern ... ");

	  /* If we haven't matched the entire string, and we want the
             longest match, try backtracking.  */
          if (d != end_match_2)
	    {
	      /* 1 if this match ends in the same string (string1 or string2)
		 as the best previous match.  */
	      boolean same_str_p;

	      /* 1 if this match is the best seen so far.  */
	      boolean best_match_p;

              same_str_p = (FIRST_STRING_P (match_end)
                            == MATCHING_IN_FIRST_STRING);

	      /* AIX compiler got confused when this was combined
		 with the previous declaration.  */
	      if (same_str_p)
		best_match_p = d > match_end;
	      else
		best_match_p = !MATCHING_IN_FIRST_STRING;

              DEBUG_PRINT1 ("backtracking.\n");

              if (!FAIL_STACK_EMPTY ())
                { /* More failure points to try.  */

                  /* If exceeds best match so far, save it.  */
                  if (!best_regs_set || best_match_p)
                    {
                      best_regs_set = true;
                      match_end = d;

                      DEBUG_PRINT1 ("\nSAVING match as best so far.\n");

                      for (mcnt = 1; (unsigned) mcnt < num_regs; mcnt++)
                        {
                          best_regstart[mcnt] = regstart[mcnt];
                          best_regend[mcnt] = regend[mcnt];
                        }
                    }
                  goto fail;
                }

              /* If no failure points, don't restore garbage.  And if
                 last match is real best match, don't restore second
                 best one. */
              else if (best_regs_set && !best_match_p)
                {
  	        restore_best_regs:
                  /* Restore best match.  It may happen that `dend ==
                     end_match_1' while the restored d is in string2.
                     For example, the pattern `x.*y.*z' against the
                     strings `x-' and `y-z-', if the two strings are
                     not consecutive in memory.  */
                  DEBUG_PRINT1 ("Restoring best registers.\n");

                  d = match_end;
                  dend = ((d >= string1 && d <= end1)
		           ? end_match_1 : end_match_2);

		  for (mcnt = 1; (unsigned) mcnt < num_regs; mcnt++)
		    {
		      regstart[mcnt] = best_regstart[mcnt];
		      regend[mcnt] = best_regend[mcnt];
		    }
                }
            } /* d != end_match_2 */

	succeed_label:
          DEBUG_PRINT1 ("Accepting match.\n");
          /* If caller wants register contents data back, do it.  */
          if (regs && !bufp->no_sub)
	    {
	      /* Have the register data arrays been allocated?  */
              if (bufp->regs_allocated == REGS_UNALLOCATED)
                { /* No.  So allocate them with malloc.  We need one
                     extra element beyond `num_regs' for the `-1' marker
                     GNU code uses.  */
                  regs->num_regs = MAX (RE_NREGS, num_regs + 1);
                  regs->start = TALLOC (regs->num_regs, regoff_t);
                  regs->end = TALLOC (regs->num_regs, regoff_t);
                  if (regs->start == NULL || regs->end == NULL)
		    {
		      FREE_VARIABLES ();
		      return -2;
		    }
                  bufp->regs_allocated = REGS_REALLOCATE;
                }
              else if (bufp->regs_allocated == REGS_REALLOCATE)
                { /* Yes.  If we need more elements than were already
                     allocated, reallocate them.  If we need fewer, just
                     leave it alone.  */
                  if (regs->num_regs < num_regs + 1)
                    {
                      regs->num_regs = num_regs + 1;
                      RETALLOC (regs->start, regs->num_regs, regoff_t);
                      RETALLOC (regs->end, regs->num_regs, regoff_t);
                      if (regs->start == NULL || regs->end == NULL)
			{
			  FREE_VARIABLES ();
			  return -2;
			}
                    }
                }
              else
		{
		  /* These braces fend off a "empty body in an else-statement"
		     warning under GCC when assert expands to nothing.  */
		  assert (bufp->regs_allocated == REGS_FIXED);
		}

              /* Convert the pointer data in `regstart' and `regend' to
                 indices.  Register zero has to be set differently,
                 since we haven't kept track of any info for it.  */
              if (regs->num_regs > 0)
                {
                  regs->start[0] = pos;
#ifdef WCHAR
		  if (MATCHING_IN_FIRST_STRING)
		    regs->end[0] = mbs_offset1 != NULL ?
					mbs_offset1[d-string1] : 0;
		  else
		    regs->end[0] = csize1 + (mbs_offset2 != NULL ?
					     mbs_offset2[d-string2] : 0);
#else
                  regs->end[0] = (MATCHING_IN_FIRST_STRING
				  ? ((regoff_t) (d - string1))
			          : ((regoff_t) (d - string2 + size1)));
#endif /* WCHAR */
                }

              /* Go through the first `min (num_regs, regs->num_regs)'
                 registers, since that is all we initialized.  */
	      for (mcnt = 1; (unsigned) mcnt < MIN (num_regs, regs->num_regs);
		   mcnt++)
		{
                  if (REG_UNSET (regstart[mcnt]) || REG_UNSET (regend[mcnt]))
                    regs->start[mcnt] = regs->end[mcnt] = -1;
                  else
                    {
		      regs->start[mcnt]
			= (regoff_t) POINTER_TO_OFFSET (regstart[mcnt]);
                      regs->end[mcnt]
			= (regoff_t) POINTER_TO_OFFSET (regend[mcnt]);
                    }
		}

              /* If the regs structure we return has more elements than
                 were in the pattern, set the extra elements to -1.  If
                 we (re)allocated the registers, this is the case,
                 because we always allocate enough to have at least one
                 -1 at the end.  */
              for (mcnt = num_regs; (unsigned) mcnt < regs->num_regs; mcnt++)
                regs->start[mcnt] = regs->end[mcnt] = -1;
	    } /* regs && !bufp->no_sub */

          DEBUG_PRINT4 ("%u failure points pushed, %u popped (%u remain).\n",
                        nfailure_points_pushed, nfailure_points_popped,
                        nfailure_points_pushed - nfailure_points_popped);
          DEBUG_PRINT2 ("%u registers pushed.\n", num_regs_pushed);

#ifdef WCHAR
	  if (MATCHING_IN_FIRST_STRING)
	    mcnt = mbs_offset1 != NULL ? mbs_offset1[d-string1] : 0;
	  else
	    mcnt = (mbs_offset2 != NULL ? mbs_offset2[d-string2] : 0) +
			csize1;
          mcnt -= pos;
#else
          mcnt = d - pos - (MATCHING_IN_FIRST_STRING
			    ? string1
			    : string2 - size1);
#endif /* WCHAR */

          DEBUG_PRINT2 ("Returning %d from re_match_2.\n", mcnt);

          FREE_VARIABLES ();
          return mcnt;
        }

      /* Otherwise match next pattern command.  */
      switch (SWITCH_ENUM_CAST ((re_opcode_t) *p++))
	{
        /* Ignore these.  Used to ignore the n of succeed_n's which
           currently have n == 0.  */
        case no_op:
          DEBUG_PRINT1 ("EXECUTING no_op.\n");
          break;

	case succeed:
          DEBUG_PRINT1 ("EXECUTING succeed.\n");
	  goto succeed_label;

        /* Match the next n pattern characters exactly.  The following
           byte in the pattern defines n, and the n bytes after that
           are the characters to match.  */
	case exactn:
#ifdef MBS_SUPPORT
	case exactn_bin:
#endif
	  mcnt = *p++;
          DEBUG_PRINT2 ("EXECUTING exactn %d.\n", mcnt);

          /* This is written out as an if-else so we don't waste time
             testing `translate' inside the loop.  */
          if (translate)
	    {
	      do
		{
		  PREFETCH ();
#ifdef WCHAR
		  if (*d <= 0xff)
		    {
		      if ((UCHAR_T) translate[(unsigned char) *d++]
			  != (UCHAR_T) *p++)
			goto fail;
		    }
		  else
		    {
		      if (*d++ != (CHAR_T) *p++)
			goto fail;
		    }
#else
		  if ((UCHAR_T) translate[(unsigned char) *d++]
		      != (UCHAR_T) *p++)
                    goto fail;
#endif /* WCHAR */
		}
	      while (--mcnt);
	    }
	  else
	    {
	      do
		{
		  PREFETCH ();
		  if (*d++ != (CHAR_T) *p++) goto fail;
		}
	      while (--mcnt);
	    }
	  SET_REGS_MATCHED ();
          break;


        /* Match any character except possibly a newline or a null.  */
	case anychar:
          DEBUG_PRINT1 ("EXECUTING anychar.\n");

          PREFETCH ();

          if ((!(bufp->syntax & RE_DOT_NEWLINE) && TRANSLATE (*d) == '\n')
              || (bufp->syntax & RE_DOT_NOT_NULL && TRANSLATE (*d) == '\000'))
	    goto fail;

          SET_REGS_MATCHED ();
          DEBUG_PRINT2 ("  Matched `%ld'.\n", (long int) *d);
          d++;
	  break;


	case charset:
	case charset_not:
	  {
	    register UCHAR_T c;
#ifdef WCHAR
	    unsigned int i, char_class_length, coll_symbol_length,
              equiv_class_length, ranges_length, chars_length, length;
	    CHAR_T *workp, *workp2, *charset_top;
#define WORK_BUFFER_SIZE 128
            CHAR_T str_buf[WORK_BUFFER_SIZE];
# ifdef _LIBC
	    uint32_t nrules;
# endif /* _LIBC */
#endif /* WCHAR */
	    boolean negate = (re_opcode_t) *(p - 1) == charset_not;

            DEBUG_PRINT2 ("EXECUTING charset%s.\n", negate ? "_not" : "");
	    PREFETCH ();
	    c = TRANSLATE (*d); /* The character to match.  */
#ifdef WCHAR
# ifdef _LIBC
	    nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
# endif /* _LIBC */
	    charset_top = p - 1;
	    char_class_length = *p++;
	    coll_symbol_length = *p++;
	    equiv_class_length = *p++;
	    ranges_length = *p++;
	    chars_length = *p++;
	    /* p points charset[6], so the address of the next instruction
	       (charset[l+m+n+2o+k+p']) equals p[l+m+n+2*o+p'],
	       where l=length of char_classes, m=length of collating_symbol,
	       n=equivalence_class, o=length of char_range,
	       p'=length of character.  */
	    workp = p;
	    /* Update p to indicate the next instruction.  */
	    p += char_class_length + coll_symbol_length+ equiv_class_length +
              2*ranges_length + chars_length;

            /* match with char_class?  */
	    for (i = 0; i < char_class_length ; i += CHAR_CLASS_SIZE)
	      {
		wctype_t wctype;
		uintptr_t alignedp = ((uintptr_t)workp
				      + __alignof__(wctype_t) - 1)
		  		      & ~(uintptr_t)(__alignof__(wctype_t) - 1);
		wctype = *((wctype_t*)alignedp);
		workp += CHAR_CLASS_SIZE;
# ifdef _LIBC
		if (__iswctype((wint_t)c, wctype))
		  goto char_set_matched;
# else
		if (iswctype((wint_t)c, wctype))
		  goto char_set_matched;
# endif
	      }

            /* match with collating_symbol?  */
# ifdef _LIBC
	    if (nrules != 0)
	      {
		const unsigned char *extra = (const unsigned char *)
		  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_SYMB_EXTRAMB);

		for (workp2 = workp + coll_symbol_length ; workp < workp2 ;
		     workp++)
		  {
		    int32_t *wextra;
		    wextra = (int32_t*)(extra + *workp++);
		    for (i = 0; i < *wextra; ++i)
		      if (TRANSLATE(d[i]) != wextra[1 + i])
			break;

		    if (i == *wextra)
		      {
			/* Update d, however d will be incremented at
			   char_set_matched:, we decrement d here.  */
			d += i - 1;
			goto char_set_matched;
		      }
		  }
	      }
	    else /* (nrules == 0) */
# endif
	      /* If we can't look up collation data, we use wcscoll
		 instead.  */
	      {
		for (workp2 = workp + coll_symbol_length ; workp < workp2 ;)
		  {
		    const CHAR_T *backup_d = d, *backup_dend = dend;
# ifdef _LIBC
		    length = __wcslen (workp);
# else
		    length = wcslen (workp);
# endif

		    /* If wcscoll(the collating symbol, whole string) > 0,
		       any substring of the string never match with the
		       collating symbol.  */
# ifdef _LIBC
		    if (__wcscoll (workp, d) > 0)
# else
		    if (wcscoll (workp, d) > 0)
# endif
		      {
			workp += length + 1;
			continue;
		      }

		    /* First, we compare the collating symbol with
		       the first character of the string.
		       If it don't match, we add the next character to
		       the compare buffer in turn.  */
		    for (i = 0 ; i < WORK_BUFFER_SIZE-1 ; i++, d++)
		      {
			int match;
			if (d == dend)
			  {
			    if (dend == end_match_2)
			      break;
			    d = string2;
			    dend = end_match_2;
			  }

			/* add next character to the compare buffer.  */
			str_buf[i] = TRANSLATE(*d);
			str_buf[i+1] = '\0';

# ifdef _LIBC
			match = __wcscoll (workp, str_buf);
# else
			match = wcscoll (workp, str_buf);
# endif
			if (match == 0)
			  goto char_set_matched;

			if (match < 0)
			  /* (str_buf > workp) indicate (str_buf + X > workp),
			     because for all X (str_buf + X > str_buf).
			     So we don't need continue this loop.  */
			  break;

			/* Otherwise(str_buf < workp),
			   (str_buf+next_character) may equals (workp).
			   So we continue this loop.  */
		      }
		    /* not matched */
		    d = backup_d;
		    dend = backup_dend;
		    workp += length + 1;
		  }
              }
            /* match with equivalence_class?  */
# ifdef _LIBC
	    if (nrules != 0)
	      {
                const CHAR_T *backup_d = d, *backup_dend = dend;
		/* Try to match the equivalence class against
		   those known to the collate implementation.  */
		const int32_t *table;
		const int32_t *weights;
		const int32_t *extra;
		const int32_t *indirect;
		int32_t idx, idx2;
		wint_t *cp;
		size_t len;

		/* This #include defines a local function!  */
#  include <locale/weightwc.h>

		table = (const int32_t *)
		  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEWC);
		weights = (const wint_t *)
		  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTWC);
		extra = (const wint_t *)
		  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAWC);
		indirect = (const int32_t *)
		  _NL_CURRENT (LC_COLLATE, _NL_COLLATE_INDIRECTWC);

		/* Write 1 collating element to str_buf, and
		   get its index.  */
		idx2 = 0;

		for (i = 0 ; idx2 == 0 && i < WORK_BUFFER_SIZE - 1; i++)
		  {
		    cp = (wint_t*)str_buf;
		    if (d == dend)
		      {
			if (dend == end_match_2)
			  break;
			d = string2;
			dend = end_match_2;
		      }
		    str_buf[i] = TRANSLATE(*(d+i));
		    str_buf[i+1] = '\0'; /* sentinel */
		    idx2 = findidx ((const wint_t**)&cp);
		  }

		/* Update d, however d will be incremented at
		   char_set_matched:, we decrement d here.  */
		d = backup_d + ((wchar_t*)cp - (wchar_t*)str_buf - 1);
		if (d >= dend)
		  {
		    if (dend == end_match_2)
			d = dend;
		    else
		      {
			d = string2;
			dend = end_match_2;
		      }
		  }

		len = weights[idx2];

		for (workp2 = workp + equiv_class_length ; workp < workp2 ;
		     workp++)
		  {
		    idx = (int32_t)*workp;
		    /* We already checked idx != 0 in regex_compile. */

		    if (idx2 != 0 && len == weights[idx])
		      {
			int cnt = 0;
			while (cnt < len && (weights[idx + 1 + cnt]
					     == weights[idx2 + 1 + cnt]))
			  ++cnt;

			if (cnt == len)
			  goto char_set_matched;
		      }
		  }
		/* not matched */
                d = backup_d;
                dend = backup_dend;
	      }
	    else /* (nrules == 0) */
# endif
	      /* If we can't look up collation data, we use wcscoll
		 instead.  */
	      {
		for (workp2 = workp + equiv_class_length ; workp < workp2 ;)
		  {
		    const CHAR_T *backup_d = d, *backup_dend = dend;
# ifdef _LIBC
		    length = __wcslen (workp);
# else
		    length = wcslen (workp);
# endif

		    /* If wcscoll(the collating symbol, whole string) > 0,
		       any substring of the string never match with the
		       collating symbol.  */
# ifdef _LIBC
		    if (__wcscoll (workp, d) > 0)
# else
		    if (wcscoll (workp, d) > 0)
# endif
		      {
			workp += length + 1;
			break;
		      }

		    /* First, we compare the equivalence class with
		       the first character of the string.
		       If it don't match, we add the next character to
		       the compare buffer in turn.  */
		    for (i = 0 ; i < WORK_BUFFER_SIZE - 1 ; i++, d++)
		      {
			int match;
			if (d == dend)
			  {
			    if (dend == end_match_2)
			      break;
			    d = string2;
			    dend = end_match_2;
			  }

			/* add next character to the compare buffer.  */
			str_buf[i] = TRANSLATE(*d);
			str_buf[i+1] = '\0';

# ifdef _LIBC
			match = __wcscoll (workp, str_buf);
# else
			match = wcscoll (workp, str_buf);
# endif

			if (match == 0)
			  goto char_set_matched;

			if (match < 0)
			/* (str_buf > workp) indicate (str_buf + X > workp),
			   because for all X (str_buf + X > str_buf).
			   So we don't need continue this loop.  */
			  break;

			/* Otherwise(str_buf < workp),
			   (str_buf+next_character) may equals (workp).
			   So we continue this loop.  */
		      }
		    /* not matched */
		    d = backup_d;
		    dend = backup_dend;
		    workp += length + 1;
		  }
	      }

            /* match with char_range?  */
# ifdef _LIBC
	    if (nrules != 0)
	      {
		uint32_t collseqval;
		const char *collseq = (const char *)
		  _NL_CURRENT(LC_COLLATE, _NL_COLLATE_COLLSEQWC);

		collseqval = collseq_table_lookup (collseq, c);

		for (; workp < p - chars_length ;)
		  {
		    uint32_t start_val, end_val;

		    /* We already compute the collation sequence value
		       of the characters (or collating symbols).  */
		    start_val = (uint32_t) *workp++; /* range_start */
		    end_val = (uint32_t) *workp++; /* range_end */

		    if (start_val <= collseqval && collseqval <= end_val)
		      goto char_set_matched;
		  }
	      }
	    else
# endif
	      {
		/* We set range_start_char at str_buf[0], range_end_char
		   at str_buf[4], and compared char at str_buf[2].  */
		str_buf[1] = 0;
		str_buf[2] = c;
		str_buf[3] = 0;
		str_buf[5] = 0;
		for (; workp < p - chars_length ;)
		  {
		    wchar_t *range_start_char, *range_end_char;

		    /* match if (range_start_char <= c <= range_end_char).  */

		    /* If range_start(or end) < 0, we assume -range_start(end)
		       is the offset of the collating symbol which is specified
		       as the character of the range start(end).  */

		    /* range_start */
		    if (*workp < 0)
		      range_start_char = charset_top - (*workp++);
		    else
		      {
			str_buf[0] = *workp++;
			range_start_char = str_buf;
		      }

		    /* range_end */
		    if (*workp < 0)
		      range_end_char = charset_top - (*workp++);
		    else
		      {
			str_buf[4] = *workp++;
			range_end_char = str_buf + 4;
		      }

# ifdef _LIBC
		    if (__wcscoll (range_start_char, str_buf+2) <= 0
			&& __wcscoll (str_buf+2, range_end_char) <= 0)
# else
		    if (wcscoll (range_start_char, str_buf+2) <= 0
			&& wcscoll (str_buf+2, range_end_char) <= 0)
# endif
		      goto char_set_matched;
		  }
	      }

            /* match with char?  */
	    for (; workp < p ; workp++)
	      if (c == *workp)
		goto char_set_matched;

	    negate = !negate;

	  char_set_matched:
	    if (negate) goto fail;
#else
            /* Cast to `unsigned' instead of `unsigned char' in case the
               bit list is a full 32 bytes long.  */
	    if (c < (unsigned) (*p * BYTEWIDTH)
		&& p[1 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
	      negate = !negate;

	    p += 1 + *p;

	    if (!negate) goto fail;
#undef WORK_BUFFER_SIZE
#endif /* WCHAR */
	    SET_REGS_MATCHED ();
            d++;
	    break;
	  }


        /* The beginning of a group is represented by start_memory.
           The arguments are the register number in the next byte, and the
           number of groups inner to this one in the next.  The text
           matched within the group is recorded (in the internal
           registers data structure) under the register number.  */
        case start_memory:
	  DEBUG_PRINT3 ("EXECUTING start_memory %ld (%ld):\n",
			(long int) *p, (long int) p[1]);

          /* Find out if this group can match the empty string.  */
	  p1 = p;		/* To send to group_match_null_string_p.  */

          if (REG_MATCH_NULL_STRING_P (reg_info[*p]) == MATCH_NULL_UNSET_VALUE)
            REG_MATCH_NULL_STRING_P (reg_info[*p])
              = PREFIX(group_match_null_string_p) (&p1, pend, reg_info);

          /* Save the position in the string where we were the last time
             we were at this open-group operator in case the group is
             operated upon by a repetition operator, e.g., with `(a*)*b'
             against `ab'; then we want to ignore where we are now in
             the string in case this attempt to match fails.  */
          old_regstart[*p] = REG_MATCH_NULL_STRING_P (reg_info[*p])
                             ? REG_UNSET (regstart[*p]) ? d : regstart[*p]
                             : regstart[*p];
	  DEBUG_PRINT2 ("  old_regstart: %d\n",
			 POINTER_TO_OFFSET (old_regstart[*p]));

          regstart[*p] = d;
	  DEBUG_PRINT2 ("  regstart: %d\n", POINTER_TO_OFFSET (regstart[*p]));

          IS_ACTIVE (reg_info[*p]) = 1;
          MATCHED_SOMETHING (reg_info[*p]) = 0;

	  /* Clear this whenever we change the register activity status.  */
	  set_regs_matched_done = 0;

          /* This is the new highest active register.  */
          highest_active_reg = *p;

          /* If nothing was active before, this is the new lowest active
             register.  */
          if (lowest_active_reg == NO_LOWEST_ACTIVE_REG)
            lowest_active_reg = *p;

          /* Move past the register number and inner group count.  */
          p += 2;
	  just_past_start_mem = p;

          break;


        /* The stop_memory opcode represents the end of a group.  Its
           arguments are the same as start_memory's: the register
           number, and the number of inner groups.  */
	case stop_memory:
	  DEBUG_PRINT3 ("EXECUTING stop_memory %ld (%ld):\n",
			(long int) *p, (long int) p[1]);

          /* We need to save the string position the last time we were at
             this close-group operator in case the group is operated
             upon by a repetition operator, e.g., with `((a*)*(b*)*)*'
             against `aba'; then we want to ignore where we are now in
             the string in case this attempt to match fails.  */
          old_regend[*p] = REG_MATCH_NULL_STRING_P (reg_info[*p])
                           ? REG_UNSET (regend[*p]) ? d : regend[*p]
			   : regend[*p];
	  DEBUG_PRINT2 ("      old_regend: %d\n",
			 POINTER_TO_OFFSET (old_regend[*p]));

          regend[*p] = d;
	  DEBUG_PRINT2 ("      regend: %d\n", POINTER_TO_OFFSET (regend[*p]));

          /* This register isn't active anymore.  */
          IS_ACTIVE (reg_info[*p]) = 0;

	  /* Clear this whenever we change the register activity status.  */
	  set_regs_matched_done = 0;

          /* If this was the only register active, nothing is active
             anymore.  */
          if (lowest_active_reg == highest_active_reg)
            {
              lowest_active_reg = NO_LOWEST_ACTIVE_REG;
              highest_active_reg = NO_HIGHEST_ACTIVE_REG;
            }
          else
            { /* We must scan for the new highest active register, since
                 it isn't necessarily one less than now: consider
                 (a(b)c(d(e)f)g).  When group 3 ends, after the f), the
                 new highest active register is 1.  */
              UCHAR_T r = *p - 1;
              while (r > 0 && !IS_ACTIVE (reg_info[r]))
                r--;

              /* If we end up at register zero, that means that we saved
                 the registers as the result of an `on_failure_jump', not
                 a `start_memory', and we jumped to past the innermost
                 `stop_memory'.  For example, in ((.)*) we save
                 registers 1 and 2 as a result of the *, but when we pop
                 back to the second ), we are at the stop_memory 1.
                 Thus, nothing is active.  */
	      if (r == 0)
                {
                  lowest_active_reg = NO_LOWEST_ACTIVE_REG;
                  highest_active_reg = NO_HIGHEST_ACTIVE_REG;
                }
              else
                highest_active_reg = r;
            }

          /* If just failed to match something this time around with a
             group that's operated on by a repetition operator, try to
             force exit from the ``loop'', and restore the register
             information for this group that we had before trying this
             last match.  */
          if ((!MATCHED_SOMETHING (reg_info[*p])
               || just_past_start_mem == p - 1)
	      && (p + 2) < pend)
            {
              boolean is_a_jump_n = false;

              p1 = p + 2;
              mcnt = 0;
              switch ((re_opcode_t) *p1++)
                {
                  case jump_n:
		    is_a_jump_n = true;
		    /* Fall through.  */
                  case pop_failure_jump:
		  case maybe_pop_jump:
		  case jump:
		  case dummy_failure_jump:
                    EXTRACT_NUMBER_AND_INCR (mcnt, p1);
		    if (is_a_jump_n)
		      p1 += OFFSET_ADDRESS_SIZE;
                    break;

                  default:
                    /* do nothing */ ;
                }
	      p1 += mcnt;

              /* If the next operation is a jump backwards in the pattern
	         to an on_failure_jump right before the start_memory
                 corresponding to this stop_memory, exit from the loop
                 by forcing a failure after pushing on the stack the
                 on_failure_jump's jump in the pattern, and d.  */
              if (mcnt < 0 && (re_opcode_t) *p1 == on_failure_jump
                  && (re_opcode_t) p1[1+OFFSET_ADDRESS_SIZE] == start_memory
		  && p1[2+OFFSET_ADDRESS_SIZE] == *p)
		{
                  /* If this group ever matched anything, then restore
                     what its registers were before trying this last
                     failed match, e.g., with `(a*)*b' against `ab' for
                     regstart[1], and, e.g., with `((a*)*(b*)*)*'
                     against `aba' for regend[3].

                     Also restore the registers for inner groups for,
                     e.g., `((a*)(b*))*' against `aba' (register 3 would
                     otherwise get trashed).  */

                  if (EVER_MATCHED_SOMETHING (reg_info[*p]))
		    {
		      unsigned r;

                      EVER_MATCHED_SOMETHING (reg_info[*p]) = 0;

		      /* Restore this and inner groups' (if any) registers.  */
                      for (r = *p; r < (unsigned) *p + (unsigned) *(p + 1);
			   r++)
                        {
                          regstart[r] = old_regstart[r];

                          /* xx why this test?  */
                          if (old_regend[r] >= regstart[r])
                            regend[r] = old_regend[r];
                        }
                    }
		  p1++;
                  EXTRACT_NUMBER_AND_INCR (mcnt, p1);
                  PUSH_FAILURE_POINT (p1 + mcnt, d, -2);

                  goto fail;
                }
            }

          /* Move past the register number and the inner group count.  */
          p += 2;
          break;


	/* \<digit> has been turned into a `duplicate' command which is
           followed by the numeric value of <digit> as the register number.  */
        case duplicate:
	  {
	    register const CHAR_T *d2, *dend2;
	    int regno = *p++;   /* Get which register to match against.  */
	    DEBUG_PRINT2 ("EXECUTING duplicate %d.\n", regno);

	    /* Can't back reference a group which we've never matched.  */
            if (REG_UNSET (regstart[regno]) || REG_UNSET (regend[regno]))
              goto fail;

            /* Where in input to try to start matching.  */
            d2 = regstart[regno];

            /* Where to stop matching; if both the place to start and
               the place to stop matching are in the same string, then
               set to the place to stop, otherwise, for now have to use
               the end of the first string.  */

            dend2 = ((FIRST_STRING_P (regstart[regno])
		      == FIRST_STRING_P (regend[regno]))
		     ? regend[regno] : end_match_1);
	    for (;;)
	      {
		/* If necessary, advance to next segment in register
                   contents.  */
		while (d2 == dend2)
		  {
		    if (dend2 == end_match_2) break;
		    if (dend2 == regend[regno]) break;

                    /* End of string1 => advance to string2. */
                    d2 = string2;
                    dend2 = regend[regno];
		  }
		/* At end of register contents => success */
		if (d2 == dend2) break;

		/* If necessary, advance to next segment in data.  */
		PREFETCH ();

		/* How many characters left in this segment to match.  */
		mcnt = dend - d;

		/* Want how many consecutive characters we can match in
                   one shot, so, if necessary, adjust the count.  */
                if (mcnt > dend2 - d2)
		  mcnt = dend2 - d2;

		/* Compare that many; failure if mismatch, else move
                   past them.  */
		if (translate
                    ? PREFIX(bcmp_translate) (d, d2, mcnt, translate)
                    : memcmp (d, d2, mcnt*sizeof(UCHAR_T)))
		  goto fail;
		d += mcnt, d2 += mcnt;

		/* Do this because we've match some characters.  */
		SET_REGS_MATCHED ();
	      }
	  }
	  break;


        /* begline matches the empty string at the beginning of the string
           (unless `not_bol' is set in `bufp'), and, if
           `newline_anchor' is set, after newlines.  */
	case begline:
          DEBUG_PRINT1 ("EXECUTING begline.\n");

          if (AT_STRINGS_BEG (d))
            {
              if (!bufp->not_bol) break;
            }
          else if (d[-1] == '\n' && bufp->newline_anchor)
            {
              break;
            }
          /* In all other cases, we fail.  */
          goto fail;


        /* endline is the dual of begline.  */
	case endline:
          DEBUG_PRINT1 ("EXECUTING endline.\n");

          if (AT_STRINGS_END (d))
            {
              if (!bufp->not_eol) break;
            }

          /* We have to ``prefetch'' the next character.  */
          else if ((d == end1 ? *string2 : *d) == '\n'
                   && bufp->newline_anchor)
            {
              break;
            }
          goto fail;


	/* Match at the very beginning of the data.  */
        case begbuf:
          DEBUG_PRINT1 ("EXECUTING begbuf.\n");
          if (AT_STRINGS_BEG (d))
            break;
          goto fail;


	/* Match at the very end of the data.  */
        case endbuf:
          DEBUG_PRINT1 ("EXECUTING endbuf.\n");
	  if (AT_STRINGS_END (d))
	    break;
          goto fail;


        /* on_failure_keep_string_jump is used to optimize `.*\n'.  It
           pushes NULL as the value for the string on the stack.  Then
           `pop_failure_point' will keep the current value for the
           string, instead of restoring it.  To see why, consider
           matching `foo\nbar' against `.*\n'.  The .* matches the foo;
           then the . fails against the \n.  But the next thing we want
           to do is match the \n against the \n; if we restored the
           string value, we would be back at the foo.

           Because this is used only in specific cases, we don't need to
           check all the things that `on_failure_jump' does, to make
           sure the right things get saved on the stack.  Hence we don't
           share its code.  The only reason to push anything on the
           stack at all is that otherwise we would have to change
           `anychar's code to do something besides goto fail in this
           case; that seems worse than this.  */
        case on_failure_keep_string_jump:
          DEBUG_PRINT1 ("EXECUTING on_failure_keep_string_jump");

          EXTRACT_NUMBER_AND_INCR (mcnt, p);
#ifdef _LIBC
          DEBUG_PRINT3 (" %d (to %p):\n", mcnt, p + mcnt);
#else
          DEBUG_PRINT3 (" %d (to 0x%x):\n", mcnt, p + mcnt);
#endif

          PUSH_FAILURE_POINT (p + mcnt, NULL, -2);
          break;


	/* Uses of on_failure_jump:

           Each alternative starts with an on_failure_jump that points
           to the beginning of the next alternative.  Each alternative
           except the last ends with a jump that in effect jumps past
           the rest of the alternatives.  (They really jump to the
           ending jump of the following alternative, because tensioning
           these jumps is a hassle.)

           Repeats start with an on_failure_jump that points past both
           the repetition text and either the following jump or
           pop_failure_jump back to this on_failure_jump.  */
	case on_failure_jump:
        on_failure:
          DEBUG_PRINT1 ("EXECUTING on_failure_jump");

          EXTRACT_NUMBER_AND_INCR (mcnt, p);
#ifdef _LIBC
          DEBUG_PRINT3 (" %d (to %p)", mcnt, p + mcnt);
#else
          DEBUG_PRINT3 (" %d (to 0x%x)", mcnt, p + mcnt);
#endif

          /* If this on_failure_jump comes right before a group (i.e.,
             the original * applied to a group), save the information
             for that group and all inner ones, so that if we fail back
             to this point, the group's information will be correct.
             For example, in \(a*\)*\1, we need the preceding group,
             and in \(zz\(a*\)b*\)\2, we need the inner group.  */

          /* We can't use `p' to check ahead because we push
             a failure point to `p + mcnt' after we do this.  */
          p1 = p;

          /* We need to skip no_op's before we look for the
             start_memory in case this on_failure_jump is happening as
             the result of a completed succeed_n, as in \(a\)\{1,3\}b\1
             against aba.  */
          while (p1 < pend && (re_opcode_t) *p1 == no_op)
            p1++;

          if (p1 < pend && (re_opcode_t) *p1 == start_memory)
            {
              /* We have a new highest active register now.  This will
                 get reset at the start_memory we are about to get to,
                 but we will have saved all the registers relevant to
                 this repetition op, as described above.  */
              highest_active_reg = *(p1 + 1) + *(p1 + 2);
              if (lowest_active_reg == NO_LOWEST_ACTIVE_REG)
                lowest_active_reg = *(p1 + 1);
            }

          DEBUG_PRINT1 (":\n");
          PUSH_FAILURE_POINT (p + mcnt, d, -2);
          break;


        /* A smart repeat ends with `maybe_pop_jump'.
	   We change it to either `pop_failure_jump' or `jump'.  */
        case maybe_pop_jump:
          EXTRACT_NUMBER_AND_INCR (mcnt, p);
          DEBUG_PRINT2 ("EXECUTING maybe_pop_jump %d.\n", mcnt);
          {
	    register UCHAR_T *p2 = p;

            /* Compare the beginning of the repeat with what in the
               pattern follows its end. If we can establish that there
               is nothing that they would both match, i.e., that we
               would have to backtrack because of (as in, e.g., `a*a')
               then we can change to pop_failure_jump, because we'll
               never have to backtrack.

               This is not true in the case of alternatives: in
               `(a|ab)*' we do need to backtrack to the `ab' alternative
               (e.g., if the string was `ab').  But instead of trying to
               detect that here, the alternative has put on a dummy
               failure point which is what we will end up popping.  */

	    /* Skip over open/close-group commands.
	       If what follows this loop is a ...+ construct,
	       look at what begins its body, since we will have to
	       match at least one of that.  */
	    while (1)
	      {
		if (p2 + 2 < pend
		    && ((re_opcode_t) *p2 == stop_memory
			|| (re_opcode_t) *p2 == start_memory))
		  p2 += 3;
		else if (p2 + 2 + 2 * OFFSET_ADDRESS_SIZE < pend
			 && (re_opcode_t) *p2 == dummy_failure_jump)
		  p2 += 2 + 2 * OFFSET_ADDRESS_SIZE;
		else
		  break;
	      }

	    p1 = p + mcnt;
	    /* p1[0] ... p1[2] are the `on_failure_jump' corresponding
	       to the `maybe_finalize_jump' of this case.  Examine what
	       follows.  */

            /* If we're at the end of the pattern, we can change.  */
            if (p2 == pend)
	      {
		/* Consider what happens when matching ":\(.*\)"
		   against ":/".  I don't really understand this code
		   yet.  */
  	        p[-(1+OFFSET_ADDRESS_SIZE)] = (UCHAR_T)
		  pop_failure_jump;
                DEBUG_PRINT1
                  ("  End of pattern: change to `pop_failure_jump'.\n");
              }

            else if ((re_opcode_t) *p2 == exactn
#ifdef MBS_SUPPORT
		     || (re_opcode_t) *p2 == exactn_bin
#endif
		     || (bufp->newline_anchor && (re_opcode_t) *p2 == endline))
	      {
		register UCHAR_T c
                  = *p2 == (UCHAR_T) endline ? '\n' : p2[2];

                if (((re_opcode_t) p1[1+OFFSET_ADDRESS_SIZE] == exactn
#ifdef MBS_SUPPORT
		     || (re_opcode_t) p1[1+OFFSET_ADDRESS_SIZE] == exactn_bin
#endif
		    ) && p1[3+OFFSET_ADDRESS_SIZE] != c)
                  {
  		    p[-(1+OFFSET_ADDRESS_SIZE)] = (UCHAR_T)
		      pop_failure_jump;
#ifdef WCHAR
		      DEBUG_PRINT3 ("  %C != %C => pop_failure_jump.\n",
				    (wint_t) c,
				    (wint_t) p1[3+OFFSET_ADDRESS_SIZE]);
#else
		      DEBUG_PRINT3 ("  %c != %c => pop_failure_jump.\n",
				    (char) c,
				    (char) p1[3+OFFSET_ADDRESS_SIZE]);
#endif
                  }

#ifndef WCHAR
		else if ((re_opcode_t) p1[3] == charset
			 || (re_opcode_t) p1[3] == charset_not)
		  {
		    int negate = (re_opcode_t) p1[3] == charset_not;

		    if (c < (unsigned) (p1[4] * BYTEWIDTH)
			&& p1[5 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
		      negate = !negate;

                    /* `negate' is equal to 1 if c would match, which means
                        that we can't change to pop_failure_jump.  */
		    if (!negate)
                      {
  		        p[-3] = (unsigned char) pop_failure_jump;
                        DEBUG_PRINT1 ("  No match => pop_failure_jump.\n");
                      }
		  }
#endif /* not WCHAR */
	      }
#ifndef WCHAR
            else if ((re_opcode_t) *p2 == charset)
	      {
		/* We win if the first character of the loop is not part
                   of the charset.  */
                if ((re_opcode_t) p1[3] == exactn
 		    && ! ((int) p2[1] * BYTEWIDTH > (int) p1[5]
 			  && (p2[2 + p1[5] / BYTEWIDTH]
 			      & (1 << (p1[5] % BYTEWIDTH)))))
		  {
		    p[-3] = (unsigned char) pop_failure_jump;
		    DEBUG_PRINT1 ("  No match => pop_failure_jump.\n");
                  }

		else if ((re_opcode_t) p1[3] == charset_not)
		  {
		    int idx;
		    /* We win if the charset_not inside the loop
		       lists every character listed in the charset after.  */
		    for (idx = 0; idx < (int) p2[1]; idx++)
		      if (! (p2[2 + idx] == 0
			     || (idx < (int) p1[4]
				 && ((p2[2 + idx] & ~ p1[5 + idx]) == 0))))
			break;

		    if (idx == p2[1])
                      {
  		        p[-3] = (unsigned char) pop_failure_jump;
                        DEBUG_PRINT1 ("  No match => pop_failure_jump.\n");
                      }
		  }
		else if ((re_opcode_t) p1[3] == charset)
		  {
		    int idx;
		    /* We win if the charset inside the loop
		       has no overlap with the one after the loop.  */
		    for (idx = 0;
			 idx < (int) p2[1] && idx < (int) p1[4];
			 idx++)
		      if ((p2[2 + idx] & p1[5 + idx]) != 0)
			break;

		    if (idx == p2[1] || idx == p1[4])
                      {
  		        p[-3] = (unsigned char) pop_failure_jump;
                        DEBUG_PRINT1 ("  No match => pop_failure_jump.\n");
                      }
		  }
	      }
#endif /* not WCHAR */
	  }


// Source: regex.c
// Lines 5530-7121
