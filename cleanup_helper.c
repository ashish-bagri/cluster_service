#include "backend_service.h"
#include "../swig_callbacks/swig_callbacks.h"

/*
 * Cleanup methods that will be called by both backend cluster service and by backend clone service
 *
 */
void cleanup_strkey_to_empty(struct strkey_to_empty **strkey_to_empty_list) {

struct strkey_to_empty *current_user, *tmp;
HASH_ITER(hh, *strkey_to_empty_list, current_user, tmp)
{
	HASH_DEL(*strkey_to_empty_list, current_user);
	free(current_user);
}
}

void cleanup_key_min_max(struct key_min_max **key_min_max_list) {

struct key_min_max *current_user, *tmp;
HASH_ITER(hh, *key_min_max_list, current_user, tmp)
{
	HASH_DEL(*key_min_max_list, current_user);
	free(current_user);
}
}

void cleanup_fhash_fhash(struct fhash_fhash **fhash_fhash_list) {
struct fhash_fhash *current_user, *tmp;
HASH_ITER(hh, *fhash_fhash_list, current_user, tmp)
{
	free(current_user->mapped_fhash);
	utarray_free(current_user->orig_fhash_list_mw);
	utarray_free(current_user->orig_fhash_list_gs);
	HASH_DEL(*fhash_fhash_list, current_user);
	free(current_user);
}
}

void cleanup_key_map(struct key_map **key_map_list) {

struct key_map *current_user, *tmp;
HASH_ITER(hh, *key_map_list, current_user, tmp)
{
	HASH_DEL(*key_map_list, current_user);
	free(current_user);
}
}

//TODO: VERIFY THIS FUNCTION
void cleanup_fhash_keys_values(struct fhash_key_values **fhash_key_values_list) {
struct fhash_key_values *current_user, *tmp;
HASH_ITER(hh, *fhash_key_values_list, current_user, tmp)
{
	free(current_user->keys);
	free(current_user->values);
	HASH_DEL(*fhash_key_values_list, current_user);
	free(current_user);
}
}


void cleanup_fhash_data(struct fhash_data **fhash_data_list) {
struct fhash_data *current_user, *tmp;
HASH_ITER(hh, *fhash_data_list, current_user, tmp)
{
	struct hashdata *current, *tmphashdata;
	HASH_ITER(hh, current_user->duplicate_list_head, current, tmphashdata)
	{
		HASH_DEL(current_user->duplicate_list_head, current);
		free(current);
	}
	HASH_DEL(*fhash_data_list, current_user);
	free(current_user);
}
}

//TODO: add proper cleanup functions for all parts of base_data_struct
void free_all_data(struct base_data_struct* orig_data_pointer_list) {
#ifdef DEBUG
	log_info("Inside Free all data. This will cleanup all the data stored by this backend cluster service");
#endif
	// call cleanup on the 3 lists : md5_fhash_list, sha_fhash_list, fhash_data_list
	cleanup_fhash_data_list(orig_data_pointer_list);
	cleanup_md5_fhash(orig_data_pointer_list);
	cleanup_sha_fhash(orig_data_pointer_list);
	// then cleanup where the hashes are : hashkey_to_empty_list
	// must add this back !
	//cleanup_hashkey_to_empty();
}
// deletes all the data and frees it
void cleanup_fhash_data_list(struct base_data_struct* orig_data_pointer_list) {
struct fhash_data *current_user, *tmp;
HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_user, tmp)
{

	//K_TYPE* tmp = current_user->keys;
	//UNORM_V_TYPE* tmp2 = current_user->values;

	struct hashdata *current, *tmphashdata;
	HASH_ITER(hh, current_user->duplicate_list_head, current, tmphashdata)
	{
		HASH_DEL(current_user->duplicate_list_head, current);
		free(current);
	}
	HASH_DEL(orig_data_pointer_list->fhash_data_list, current_user);
	//free(tmp);
	//free(tmp2);
	free(current_user);
}
}

void cleanup_sha_fhash(struct base_data_struct* orig_data_pointer_list) {
struct sha_fhash *current_user, *tmp;
HASH_ITER(hh, orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, current_user, tmp)
{
	HASH_DEL(orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, current_user); /* delete; users advances to next */
	free(current_user); /* optional- if you want to free  */
}
}

void cleanup_md5_fhash(struct base_data_struct* orig_data_pointer_list) {
struct md5_fhash *current_user, *tmp;
HASH_ITER(hh, orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, current_user, tmp)
{
	HASH_DEL(orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, current_user); /* delete; users advances to next */
	free(current_user); /* optional- if you want to free  */
}
}

void cleanup_nn_data(struct nn_data **nn_data_list) {
struct nn_data *elt, *tmp;
LL_FOREACH_SAFE(*nn_data_list,elt,tmp)
{
	LL_DELETE(*nn_data_list, elt);
	free(elt);
}
}

void cleanup_hashkey_to_empty(struct hashkey_to_empty *hashkey_to_empty_list) {

struct hashkey_to_empty *current_user, *tmp;
HASH_ITER(hh, hashkey_to_empty_list, current_user, tmp)
{
	HASH_DEL(hashkey_to_empty_list, current_user);
	free(current_user);
}
}

void clean_anomaly_data(struct anomaly_data **anomaly_data_list)
{
	if(*anomaly_data_list == NULL)
	{
		return;
	}
	struct anomaly_data *current_user, *tmp;
	HASH_ITER(hh, *anomaly_data_list, current_user, tmp)
	{
		if(current_user->orig_fhash_list_mw != NULL)
		{
			utarray_free(current_user->orig_fhash_list_mw);
		}
		if(current_user->orig_fhash_list_gs != NULL)
		{
			utarray_free(current_user->orig_fhash_list_gs);
		}
		HASH_DEL(*anomaly_data_list, current_user);
		free(current_user);
	}
	*anomaly_data_list =  NULL;
}
