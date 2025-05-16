static void *curl_thread_create_thunk(void *arg)
{
  struct Curl_actual_call *ac = arg;
  unsigned int (*func)(void *) = ac->func;
  void *real_arg = ac->arg;

  free(ac);

  (*func)(real_arg);

  return 0;
}


// Source: curl_threads.c
// Lines 49-60
