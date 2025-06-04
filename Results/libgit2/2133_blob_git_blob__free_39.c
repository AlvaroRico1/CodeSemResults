void git_blob__free(void *_blob)
{
	git_blob *blob = (git_blob *) _blob;
	if (!blob->raw)
		git_odb_object_free(blob->data.odb);
	git__free(blob);
}


// Source: blob.c
// Lines 47-53
