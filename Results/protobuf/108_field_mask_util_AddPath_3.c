void FieldMaskTree::AddPath(const std::string& path) {
  std::vector<std::string> parts = Split(path, ".");
  if (parts.empty()) {
    return;
  }
  bool new_branch = false;
  Node* node = &root_;
  for (const std::string& node_name : parts) {
    if (!new_branch && node != &root_ && node->children.empty()) {
      // Path matches an existing leaf node. This means the path is already
      // covered by this tree (for example, adding "foo.bar.baz" to a tree
      // which already contains "foo.bar").
      return;
    }
    Node*& child = node->children[node_name];
    if (child == nullptr) {
      new_branch = true;
      child = new Node();
    }
    node = child;
  }


// Source: field_mask_util.cc
// Lines 327-347
