void FieldMaskTree::IntersectPath(const std::string& path, FieldMaskTree* out) {
  std::vector<std::string> parts = Split(path, ".");
  if (parts.empty()) {
    return;
  }
  const Node* node = &root_;
  for (const std::string& node_name : parts) {
    if (node->children.empty()) {
      if (node != &root_) {
        out->AddPath(path);
      }
      return;
    }
    const Node* result = FindPtrOrNull(node->children, node_name);
    if (result == nullptr) {
      // No intersection found.
      return;
    }
    node = result;
  }


// Source: field_mask_util.cc
// Lines 412-431
