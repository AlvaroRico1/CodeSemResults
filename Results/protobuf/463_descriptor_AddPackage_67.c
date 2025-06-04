void DescriptorBuilder::AddPackage(const std::string& name,
                                   const Message& proto, FileDescriptor* file) {
  if (name.find('\0') != std::string::npos) {
    AddError(name, proto, DescriptorPool::ErrorCollector::NAME,
             "\"" + name + "\" contains null character.");
    return;
  }

  Symbol existing_symbol = tables_->FindSymbol(name);
  // It's OK to redefine a package.
  if (existing_symbol.IsNull()) {
    if (&name == &file->package()) {
      // It is the toplevel package name, so insert the descriptor directly.
      tables_->AddSymbol(file->package(), Symbol(file));
    } else {
      auto* package = tables_->Allocate<Symbol::Subpackage>();
      // If the name is the package name, then it is already in the arena.
      // If not, copy it there. It came from the call to AddPackage below.
      package->name_size = static_cast<int>(name.size());
      package->file = file;
      tables_->AddSymbol(name, Symbol(package));
    }


// Source: descriptor.cc
// Lines 4638-4659
