void SourceTreeDescriptorDatabase::ValidationErrorCollector::AddWarning(
    const std::string& filename, const std::string& element_name,
    const Message* descriptor, ErrorLocation location,
    const std::string& message) {
  if (owner_->error_collector_ == nullptr) return;

  int line, column;
  if (location == DescriptorPool::ErrorCollector::IMPORT) {
    owner_->source_locations_.FindImport(descriptor, element_name, &line,
                                         &column);
  } else {
    owner_->source_locations_.Find(descriptor, location, &line, &column);
  }
  owner_->error_collector_->AddWarning(filename, line, column, message);
}


// Source: importer.cc
// Lines 202-216
