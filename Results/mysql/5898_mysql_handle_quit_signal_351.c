void handle_quit_signal(int sig) {
  const char *reason = "Terminal close";

  if (!executing_query) {
    tee_fprintf(stdout, "%s -- exit!\n", reason);
    goto err;
  }

  kill_query(reason);

err:
#ifdef _WIN32
  /*
   When a signal is raised on Windows, the OS creates a new thread to
   handle the interrupt. Once that thread completes, the main thread
   continues running only to find that it's resources have already been
   free'd when the signal handler called mysql_end().
  */
  mysql_thread_end();
  return;
#else
  mysql_end(sig);
#endif
}


// Source: mysql.cc
// Lines 1543-1566
