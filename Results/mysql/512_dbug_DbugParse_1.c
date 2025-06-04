static int DbugParse(CODE_STATE *cs, const char *control) {
  const char *end;
  int rel, f_used = 0;
  struct settings *stack;

  /*
    Make sure we are not changing settings while inside a
      DBUG_LOCK_FILE
      DBUG_UNLOCK_FILE
    section, that is a mis use, that would cause changing
    DBUG_FILE while the caller prints to it.
  */
  assert(!cs->locked);

  stack = cs->stack;

  /*
    When parsing the global init_settings itself,
    make sure to block every other thread using dbug functions.
  */
  assert(cs->m_read_lock_count == 0);
  if (stack == &init_settings) native_rw_wrlock(&THR_LOCK_init_settings);

  if (control[0] == '-' && control[1] == '#') control += 2;

  rel = control[0] == '+' || control[0] == '-';
  if ((!rel || (!stack->out_file && !stack->next))) {
    /* Free memory associated with the state before resetting its members */
    FreeState(cs, stack, 0);
    stack->flags = 0;
    stack->delay = 0;
    stack->maxdepth = 0;
    stack->sub_level = 0;
    stack->out_file = stderr;
    stack->prof_file = nullptr;
    stack->functions = nullptr;
    stack->p_functions = nullptr;
    stack->keywords = nullptr;
    stack->processes = nullptr;
  } else if (!stack->out_file) {
    stack->flags = stack->next->flags;
    stack->delay = stack->next->delay;
    stack->maxdepth = stack->next->maxdepth;
    stack->sub_level = stack->next->sub_level;
    strcpy(stack->name, stack->next->name);
    stack->prof_file = stack->next->prof_file;
    if (stack->next == &init_settings) {
      assert(stack != &init_settings);
      native_rw_rdlock(&THR_LOCK_init_settings);

      /*
        Never share with the global parent - it can change under your feet.

        Reset out_file to stderr to prevent sharing of trace files between
        global and session settings.
      */
      stack->out_file = stderr;
      stack->functions = ListCopy(init_settings.functions);
      stack->p_functions = ListCopy(init_settings.p_functions);
      stack->keywords = ListCopy(init_settings.keywords);
      stack->processes = ListCopy(init_settings.processes);

      native_rw_unlock(&THR_LOCK_init_settings);
    } else {
      stack->out_file = stack->next->out_file;
      stack->functions = stack->next->functions;
      stack->p_functions = stack->next->p_functions;
      stack->keywords = stack->next->keywords;
      stack->processes = stack->next->processes;
    }
  }

  end = DbugStrTok(control);
  while (control < end) {
    int c, sign = (*control == '+') ? 1 : (*control == '-') ? -1 : 0;
    if (sign) control++;
    c = *control++;
    if (*control == ',') control++;
    /* XXX when adding new cases here, don't forget _db_explain_ ! */
    switch (c) {
      case 'd':
        if (sign < 0 && control == end) {
          if (!is_shared(stack, keywords)) FreeList(stack->keywords);
          stack->keywords = nullptr;
          stack->flags &= ~DEBUG_ON;
          break;
        }
        if (rel && is_shared(stack, keywords))
          stack->keywords = ListCopy(stack->keywords);
        if (sign < 0) {
          if (DEBUGGING) {
            stack->keywords = ListDel(stack->keywords, control, end);
            /* Turn off DEBUG_ON if it is last keyword to be removed. */
            if (stack->keywords == nullptr) stack->flags &= ~DEBUG_ON;
          }
          break;
        }

        /* Do not add keyword if debugging all is enabled. */
        if (!(DEBUGGING && stack->keywords == nullptr)) {
          stack->keywords = ListAdd(stack->keywords, control, end);
          stack->flags |= DEBUG_ON;
        }

        /* If debug all is enabled, make the keyword list empty. */
        if (sign == 1 && control == end) {
          FreeList(stack->keywords);
          stack->keywords = nullptr;
        }

        break;
      case 'D':
        stack->delay = atoi(control);
        break;
      case 'f':
        f_used = 1;
        if (sign < 0 && control == end) {
          if (!is_shared(stack, functions)) FreeList(stack->functions);
          stack->functions = nullptr;
          break;
        }
        if (rel && is_shared(stack, functions))
          stack->functions = ListCopy(stack->functions);
        if (sign < 0)
          stack->functions = ListDel(stack->functions, control, end);
        else
          stack->functions = ListAdd(stack->functions, control, end);
        break;
      case 'F':
        if (sign < 0)
          stack->flags &= ~FILE_ON;
        else
          stack->flags |= FILE_ON;
        break;
      case 'i':
        if (sign < 0)
          stack->flags &= ~PID_ON;
        else
          stack->flags |= PID_ON;
        break;
      case 'L':
        if (sign < 0)
          stack->flags &= ~LINE_ON;
        else
          stack->flags |= LINE_ON;
        break;
      case 'n':
        if (sign < 0)
          stack->flags &= ~DEPTH_ON;
        else
          stack->flags |= DEPTH_ON;
        break;
      case 'N':
        if (sign < 0)
          stack->flags &= ~NUMBER_ON;
        else
          stack->flags |= NUMBER_ON;
        break;
      case 'A':
      case 'O':
        stack->flags |= FLUSH_ON_WRITE;
        /* fall through */
      case 'a':
      case 'o':
        /* In case we already have an open file. */
        if (!is_shared(stack, out_file)) DBUGCloseFile(cs, stack->out_file);
        if (sign < 0) {
          stack->flags &= ~FLUSH_ON_WRITE;
          stack->out_file = stderr;
          break;
        }
        if (c == 'a' || c == 'A')
          stack->flags |= OPEN_APPEND;
        else
          stack->flags &= ~OPEN_APPEND;
        if (control != end)
          DBUGOpenFile(cs, control, end, stack->flags & OPEN_APPEND);
        else
          DBUGOpenFile(cs, "-", nullptr, 0);
        break;
      case 'p':
        if (sign < 0 && control == end) {
          if (!is_shared(stack, processes)) FreeList(stack->processes);
          stack->processes = nullptr;
          break;
        }
        if (rel && is_shared(stack, processes))
          stack->processes = ListCopy(stack->processes);
        if (sign < 0)
          stack->processes = ListDel(stack->processes, control, end);
        else
          stack->processes = ListAdd(stack->processes, control, end);
        break;
      case 'P':
        if (sign < 0)
          stack->flags &= ~PROCESS_ON;
        else
          stack->flags |= PROCESS_ON;
        break;
      case 'r':
        stack->sub_level = cs->level;
        break;
      case 't':
        if (sign < 0) {
          if (control != end)
            stack->maxdepth -= atoi(control);
          else
            stack->maxdepth = 0;
        } else {
          if (control != end)
            stack->maxdepth += atoi(control);
          else
            stack->maxdepth = MAXDEPTH;
        }
        if (stack->maxdepth > 0)
          stack->flags |= TRACE_ON;
        else
          stack->flags &= ~TRACE_ON;
        break;
      case 'T':
        if (sign < 0)
          stack->flags &= ~TIMESTAMP_ON;
        else
          stack->flags |= TIMESTAMP_ON;
        break;
    }
    if (!*end) break;
    control = end + 1;
    end = DbugStrTok(control);
  }

  if (stack->next == &init_settings) {
    /*
      Enforce nothing is shared with the global init_settings
    */
    assert((stack->functions == nullptr) ||
           (stack->functions != init_settings.functions));
    assert((stack->p_functions == nullptr) ||
           (stack->p_functions != init_settings.p_functions));
    assert((stack->keywords == nullptr) ||
           (stack->keywords != init_settings.keywords));
    assert((stack->processes == nullptr) ||
           (stack->processes != init_settings.processes));
  }

  if (stack == &init_settings) native_rw_unlock(&THR_LOCK_init_settings);

  return !rel || f_used;
}


// Source: dbug.cc
// Lines 436-684
