void FileDescriptor::InternalDependenciesOnceInit() const {
  GOOGLE_CHECK(finished_building_ == true);
  const char* names_ptr = reinterpret_cast<const char*>(dependencies_once_ + 1);
  for (int i = 0; i < dependency_count(); i++) {
    const char* name = names_ptr;
    names_ptr += strlen(name) + 1;
    if (name[0] != '\0') {
      dependencies_[i] = pool_->FindFileByName(name);
    }
  }


// Source: descriptor.cc
// Lines 8141-8150
