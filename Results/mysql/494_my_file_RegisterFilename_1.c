void RegisterFilename(File fd, const char *file_name, OpenType type_of_file) {
  assert(fd > -1);
  FileInfoVector &fiv = *fivp;
  MUTEX_LOCK(g, &THR_LOCK_open);
  if (static_cast<size_t>(fd) >= fiv.size()) {
    fiv.resize(fd + 1);
  }
  CountFileOpen(fiv[fd].type(), type_of_file);
  fiv[fd] = {file_name, type_of_file};

  dbug("fileinfo", [&]() {
    std::cerr << "Registering (" << fd << ", '" << file_name << ")"
              << std::endl;
  });
}

/**
   Remove FileInfo entry for file descriptor. Decrements status
   variables for open files/streams.
   @relates file_info::UnregisterFilename

   @param fd file descriptor
 */
void UnregisterFilename(File fd) {
  FileInfoVector &fiv = *fivp;
  MUTEX_LOCK(g, &THR_LOCK_open);

  if (static_cast<size_t>(fd) >= fiv.size()) {
    dbug("fileinfo", [&]() {
      std::cerr << "Un-registering unknown fd:" << fd << "!" << std::endl;
    });
    return;
  }
  if (fiv[fd].type() == OpenType::UNOPEN) {
    dbug("fileinfo", [&]() {
      std::cerr << "Un-registering already UNOPEN fd:" << fd << std::endl;
    });
    return;
  }
  CountFileClose(fiv[fd].type());

  dbug("fileinfo", [&]() {
    std::cerr << "Un-registering (" << fd << ", '" << fiv[fd].name() << "')"
              << std::endl;
  });
  fiv[fd] = {};
}
}  // namespace file_info


// Source: my_file.cc
// Lines 191-238
