bool set_network_namespace(const std::string &network_namespace) {
  int fd;

  if (original_ns_fd == -1 && save_original_network_namespace(&original_ns_fd))
    return true;

  if (open_network_namespace(network_namespace, &fd)) return true;

  if (setns(fd, CLONE_NEWNET) != 0) {
#ifdef MYSQL_SERVER
    char errbuf[MYSYS_STRERROR_SIZE];

    LogErr(ERROR_LEVEL, ER_SETNS_FAILED,
           my_strerror(errbuf, sizeof(errbuf), errno));
#endif
    close(fd);

    return true;
  }

  return false;
}


// Source: net_ns.cc
// Lines 152-173
