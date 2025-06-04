void ZeroFieldsBase::CopyImpl(Message* to_param, const Message& from_param) {
  auto* to = static_cast<ZeroFieldsBase*>(to_param);
  const auto* from = static_cast<const ZeroFieldsBase*>(&from_param);
  if (from == to) return;
  to->_internal_metadata_.Clear<UnknownFieldSet>();
  to->_internal_metadata_.MergeFrom<UnknownFieldSet>(from->_internal_metadata_);
}


// Source: generated_message_bases.cc
// Lines 108-114
