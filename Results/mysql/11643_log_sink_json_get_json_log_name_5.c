static log_service_error get_json_log_name(void *instance, char *buf,
                                           size_t bufsize) {
  my_state *mi = (my_state *)instance;
  int stream_id = 0;  // default stream-ID
  size_t len;

  if (buf == nullptr) {
    return LOG_SERVICE_BUFFER_SIZE_INSUFFICIENT; /* purecov: inspected */
  }

  // instance was given and has an existing extension. return it.
  if ((mi != nullptr) && (mi->ext != nullptr)) {
    if (bufsize <= strlen(mi->ext)) {  // caller's buffer is too small
      return LOG_SERVICE_BUFFER_SIZE_INSUFFICIENT; /* purecov: inspected */
    }
    strcpy(buf, mi->ext);
    return LOG_SERVICE_SUCCESS;
  }

  // no extension exists. make one.

  // is there enough space?
  if (bufsize < (3 + sizeof(LOG_EXT)))           // caller's buffer is too small
    return LOG_SERVICE_BUFFER_SIZE_INSUFFICIENT; /* purecov: inspected */

  if (mi != nullptr)  // non-default session was given; retrieve its stream-ID.
    stream_id = mi->id;

  // make the name with the correct stream-ID.
  len = log_bs->substitute(buf, bufsize, ".%02d" LOG_EXT, stream_id);

  if (len >= bufsize)
    return LOG_SERVICE_BUFFER_SIZE_INSUFFICIENT; /* purecov: inspected */

  return LOG_SERVICE_SUCCESS;
}


// Source: log_sink_json.cc
// Lines 391-426
