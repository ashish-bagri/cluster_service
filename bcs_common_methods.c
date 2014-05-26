#include "backend_service.h"
#include "../swig_callbacks/swig_callbacks.h"
/*
 *Interfacing get methods that can be called using backend cluster service or by backend clone service
 *
 *
 */

//#define DEBUG

void get_libsvm_keys_values_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, int* num_dim, K_TYPE **out_keys, UNORM_V_TYPE **out_values) {

#ifdef DEBUG
	log_info("Inside get_libsvm_keys_values_from_fhash\n");
#endif
	//struct fhash_data *s;
	struct fhash_key_values *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->fhash_key_values_list, fhash, s);
	if (s == NULL) {
		log_error("ERROR: NO feature hash %s in the database", fhash);
		*num_dim = 0;
		return NULL;
	}

	int num = s->num_dimensions;
#ifdef DEBUG
	log_info("Number of dimensions %d\n", num);
#endif
	*num_dim = num;

	K_TYPE* keys = malloc(sizeof(K_TYPE) * num);
	UNORM_V_TYPE* values = malloc(sizeof(UNORM_V_TYPE) * num);

	int i = 0;
	for (i = 0; i < num; i++) {
		keys[i] = s->keys[i];
		values[i] = s->values[i];
	}
	*out_keys = keys;
	*out_values = values;
}

//given fhash, get the associated libsvm data, in normal lib svm format !
void getlibsvmdata_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, char** retsvmstr) {
	struct fhash_key_values *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->fhash_key_values_list, fhash, s);
	if (s == NULL) {
		log_error("No info about fhash %s in the main list. Returning error!", fhash);
		return NULL;
	}
	K_TYPE* keys = s->keys;
	UNORM_V_TYPE* values = s->values;
	K_TYPE num_dim = s->num_dimensions;

	// GET THE CORRECT LABEL OF THIS FHASH
	struct fhash_data *fhash_data_obj;
	HASH_FIND_STR(orig_data_pointer_list->fhash_data_list, fhash, fhash_data_obj);
	if(fhash_data_obj == NULL)
	{
		log_error("No info about this fhash in the main list. Returning error!");
		*retsvmstr = NULL;
		return;
	}

	int label = fhash_data_obj->label;


	UT_string *svm_data; // the string demanded by the user
	create_raw_lib_svm_sparse(keys,values,label,num_dim,&svm_data, true);

	*retsvmstr = strdup(utstring_body(svm_data));
	utstring_free(svm_data);
}

void get_raw_data(struct base_data_struct* orig_data_pointer_list, char** svm_raw_str, size_t *num_elements_gs, size_t *num_elements_mw) {
	size_t num_elements = 0;
	UT_string *libsvmstr_raw;
	utstring_new(libsvmstr_raw);
	struct fhash_data *current_ele, *tmp;
	*num_elements_gs = 0;
	*num_elements_mw = 0;
	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
// skip deleted elements
		if (current_ele->deleted) {
			continue;
		}
		struct fhash_key_values *s;
		HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->fhash_key_values_list, current_ele->fhash, s);
		if (s == NULL) {
			log_error("ERROR: NO feature hash %s in the database", current_ele->fhash);
			continue;
		}
		K_TYPE* keys = s->keys;
		UNORM_V_TYPE* values = s->values;
		K_TYPE num_dimensions = s->num_dimensions;
		int label = current_ele->label;
		if (label == LABEL_CLEAN) {
			*num_elements_gs = *num_elements_gs + 1;
		} else if (label == LABEL_MW) {
			*num_elements_mw = *num_elements_mw + 1;
		} else {
			continue;
		}

		num_elements = num_elements + 1;
		UT_string *libsvmstr;
		create_raw_lib_svm_sparse(keys, values, label, num_dimensions, &libsvmstr, true);
		utstring_concat(libsvmstr_raw, libsvmstr);
		utstring_free(libsvmstr);
	}
	*svm_raw_str = strdup(utstring_body(libsvmstr_raw));
	utstring_free(libsvmstr_raw);
}

void do_minimize_dimension_get_map(struct base_data_struct* orig_data_pointer_list, struct key_map **outputkeymap, struct key_min_max **output_max_min, K_TYPE *num_old_dim, K_TYPE *num_new_dim) {

	struct key_min_max* min_max_list = NULL;
	struct key_map* mapping_existing_new = NULL;
	size_t num_elements = 0;
	K_TYPE num_ele_before = 0;
	K_TYPE num_ele_map = 0;

	get_min_max_list(orig_data_pointer_list, &min_max_list, &num_elements);

	get_mapping_dict(&mapping_existing_new, &min_max_list, num_elements);

	num_ele_before = HASH_COUNT(min_max_list);
	*num_old_dim = num_ele_before;

	num_ele_map = HASH_COUNT(mapping_existing_new);
	*num_new_dim = num_ele_map;
	*outputkeymap = mapping_existing_new;
	*output_max_min = min_max_list;

#ifdef DEBUG
	log_info("Num of dimensions before was %d", num_ele_before);
	log_info("Num of dimensions after is %d", num_ele_map);
	log_info("Completed creating mapping \n");
#endif
}

void get_orig_map(struct base_data_struct* orig_data_pointer_list, struct key_map **outputkeymap, struct key_min_max **output_max_min, K_TYPE *num_old_dim, K_TYPE *num_new_dim) {

	struct key_min_max* min_max_list = NULL;
	struct key_map* mapping_existing_new = NULL;
	size_t num_elements = 0;
	K_TYPE num_ele_before = 0;
	K_TYPE num_ele_map = 0;

	get_min_max_list(orig_data_pointer_list, &min_max_list, &num_elements);

	create_original_mapping(&min_max_list, &mapping_existing_new);

	num_ele_before = HASH_COUNT(min_max_list);
	*num_old_dim = num_ele_before;

	num_ele_map = HASH_COUNT(mapping_existing_new);
	*num_new_dim = num_ele_map;
	*outputkeymap = mapping_existing_new;
	*output_max_min = min_max_list;

#ifdef DEBUG
	log_info("Num of dimensions before was %d", num_ele_before);
	log_info("Num of dimensions after is %d", num_ele_map);
	log_info("Completed creating mapping \n");
#endif
}

void get_k_nn_fhashes(struct base_data_struct* orig_data_pointer_list, char* input_fhash, int k, struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data)
{
#ifdef DEBUG
	log_info("Inside get K nearest neighbor\n");
#endif

	// get keys, values for input feature hash
	struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, input_fhash);
	K_TYPE* in_keys = keys_values->keys;
	UNORM_V_TYPE* in_values = keys_values->values;
	K_TYPE in_dimension = keys_values->num_dimensions;

	V_TYPE * in_norm_values = malloc(sizeof(V_TYPE) * in_dimension);
// Try using orig_data_pointer_list->key_min_max_list instead of in_orig_min_max

	do_normalization(in_keys, in_values, in_dimension, in_norm_values, in_orig_min_max);

	fill_k_nn_data(orig_data_pointer_list, k_nn_data, in_orig_min_max, &in_keys, &in_norm_values, in_dimension, k);
	LL_SORT(*k_nn_data, knn_cmp_rev);
	k_nn_keep_topk(k_nn_data, k);
}

void get_k_nn(struct base_data_struct* orig_data_pointer_list, char* in_svmstr, int k, struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data) {

#ifdef DEBUG
	log_info("Inside get K nearest neighbor\n");
#endif

	if (k == 0) {
		log_info("WARNING : No neighbors requested \n");
		return;
	}

	K_TYPE *in_keys;
	UNORM_V_TYPE* in_values;
	int in_label;
	K_TYPE in_dimension;

	libsvmstr_key_values(in_svmstr, &in_label, &in_keys, &in_values, &in_dimension);

#ifdef DEBUG
	log_info("The number of in dimension: %d", in_dimension);
#endif

	V_TYPE * in_norm_values = malloc(sizeof(V_TYPE) * in_dimension);
	do_normalization(in_keys, in_values, in_dimension, in_norm_values, in_orig_min_max);

#ifdef DEBUG
	K_TYPE d = 0;
	log_info("Input data :: \n");
	for (d = 0; d < in_dimension; d++) {
		log_info("Key: %d, value %d normalized %0.18g \n", in_keys[d], in_values[d], in_norm_values[d]);
	}
#endif
	free(in_values); // in_values is no longer needed


	fill_k_nn_data(orig_data_pointer_list, k_nn_data, in_orig_min_max, &in_keys, &in_norm_values, in_dimension, k);

// finally, must reverse it to have the smallest distance as the first element!
	LL_SORT(*k_nn_data, knn_cmp_rev);

#ifdef DEBUG
	display_k_nn_data(k_nn_data);
#endif

	// keep only top k data
	k_nn_keep_topk(k_nn_data, k);
	free(in_keys);
	free(in_norm_values);

#ifdef DEBUG
	log_info("Completed K nearest neighbor\n");
#endif

}


void get_fhash_from_sha(struct base_data_struct* orig_data_pointer_list, char* sha, char **fhash) {
	struct sha_fhash *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, sha, s);
	if (s != NULL) {
		*fhash = strdup(s->fhash);
	} else {
		*fhash = NULL;
	}
}

void get_fhash_from_md5(struct base_data_struct* orig_data_pointer_list, char* md5, char** fhash) {
	struct md5_fhash *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, md5, s);
	if (s != NULL) {
		*fhash = strdup(s->fhash);
	} else {
		*fhash = NULL;
	}
}

void get_data_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, struct fhash_data **retdata) {
	if(orig_data_pointer_list->fhash_data_list == NULL)
	{
		*retdata = NULL;
	}else
	{
		struct fhash_data *s;
		HASH_FIND_STR(orig_data_pointer_list->fhash_data_list, fhash, s);
		if (s != NULL) {
			*retdata = s;
		} else
		{
			*retdata = NULL;
		}
	}
}

void get_orig_counts(struct base_data_struct* orig_data_pointer_list, int *num_elements_mw, int *num_elements_gs)
{
	int gs = 0;
	int mw = 0;

	struct fhash_data *current_ele, *tmp;
	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
// skip deleted elements
		if (current_ele->deleted) {
			continue;
		}

		int label = current_ele->label;
		int is_orig = current_ele->is_orig;
		if (is_orig == 1)
		{
			if (label == LABEL_CLEAN) {
				gs++;
			}else if(label == LABEL_MW)
			{
				mw++;
			}
			else
			{
				continue;
			}
		}
	}
#ifdef DEBUG
	printf("The number of original MW: %d \n", mw);
	printf("The number of original GS: %d \n",gs);
#endif
	*num_elements_mw = mw;
	*num_elements_gs = gs;
}

int is_orig_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash)
{
	struct fhash_data *fhashdata;
	get_data_from_fhash(orig_data_pointer_list, fhash, &fhashdata);
	if (fhashdata == NULL) {
		return NULL;
	} else {
		int is_orig = fhashdata->is_orig;
		return is_orig;
	}
}

void get_current_fhash_data_list_count(struct base_data_struct* orig_data_pointer_list, int* curr_count)
{
	*curr_count = HASH_COUNT(orig_data_pointer_list->fhash_data_list);
}

void get_normalized_str(struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char* in_svmstr,
		char** retlibsvmstr) {

// extract the key and value of the given svm data
#ifdef DEBUG
	log_info("Input svm string %s ", in_svmstr);
#endif
	K_TYPE *in_keys;
	UNORM_V_TYPE* in_values;
	int in_label;
	K_TYPE in_dimension;

	libsvmstr_key_values(in_svmstr, &in_label, &in_keys, &in_values, &in_dimension);
	create_svm_str(in_keys, in_values, in_dimension, in_label, oldkey_min_max_list, mapping_old_to_new, retlibsvmstr);
	free(in_keys);
	free(in_values);
}


void get_predict_svm_str_from_sha(struct base_data_struct* orig_data_pointer_list, struct key_min_max **key_min_max_list, struct key_map **old_new_keymap, char* sha,
		char **retlibsvmstr) {

	// get the feature hash from sha,
	// get the keys and values corresponsing to it
	// normalize/minimize and return
#ifdef DEBUG
	log_info("Input Sha is  %s ", sha);
#endif

	char* fhash;
	get_fhash_from_sha(orig_data_pointer_list, sha, &fhash);
	if (fhash == NULL) {
		*retlibsvmstr = NULL;
		return;
	}

	struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, fhash);
	K_TYPE* keys = keys_values->keys;
	UNORM_V_TYPE* values = keys_values->values;

	// GET THE CORRECT LABEL OF THIS FHASH
	struct fhash_data *fhash_data_obj;
	HASH_FIND_STR(orig_data_pointer_list->fhash_data_list, fhash, fhash_data_obj);
	if(fhash_data_obj == NULL)
	{
		log_error("No info about fhash %s in the main list. Returning error!", fhash);
		*retlibsvmstr = NULL;
		return;
	}

	int label = fhash_data_obj->label;

	K_TYPE num_dim = keys_values->num_dimensions;
	create_svm_str(keys, values, num_dim, label, key_min_max_list, old_new_keymap, retlibsvmstr);
	return;
}

void get_predict_svm_str_from_fhash(struct base_data_struct* orig_data_pointer_list, struct key_min_max **key_min_max_list, struct key_map **old_new_keymap, char* fhash,
		char **retlibsvmstr, int* is_orig ) {
	// get the keys and values corresponsing to it
	// normalize/minimize and return
#ifdef DEBUG
	log_info("Input fhash is  %s ", fhash);
#endif

	struct fhash_data *s;
	get_data_from_fhash(orig_data_pointer_list, fhash, &s);

	struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, fhash);

	K_TYPE* keys = keys_values->keys;
	UNORM_V_TYPE* values = keys_values->values;
	K_TYPE num_dim = keys_values->num_dimensions;
	int label = s->label;
	*is_orig = s->is_orig;

	create_svm_str(keys, values, num_dim, label, key_min_max_list, old_new_keymap, retlibsvmstr);
	return;
}


int get_label_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash) {
	struct fhash_data *fhashdata;
	get_data_from_fhash(orig_data_pointer_list, fhash, &fhashdata);
	if (fhashdata == NULL) {
		return NULL;
	} else {
		int label = fhashdata->label;
		return label;
	}
}


/** void get_normalized_svm_data_mrmr()
 * returns all non deleted default svm data in dense csv format (to be used by MRMR)
 * iterate over all the feature hashes.
 * First find out the min, max for each. Then remove dimensions not used at all.
 * Normalize using min and max and create a map.
 * return only dimensions that were used along with the mapping list
 */
void get_normalized_svm_data_mrmr(struct base_data_struct* orig_data_pointer_list, char** svm_mrmr_str, size_t *num_elements_gs, size_t *num_elements_mw,
		struct key_map **outputkeymap, struct key_min_max **output_max_min) {

	struct key_min_max* min_max_list = NULL;
	size_t num_elements = 0;
	struct key_map* mapping_existing_new = NULL;

	get_min_max_list(orig_data_pointer_list, &min_max_list, &num_elements);

	get_mapping_dict(&mapping_existing_new, &min_max_list, num_elements);

	create_normalized_svm_data_mrmr(orig_data_pointer_list, &min_max_list, &mapping_existing_new, svm_mrmr_str, num_elements_gs,
			num_elements_mw);

#ifdef DEBUG
	log_info("The number of gs elements returned is %ld", *num_elements_gs);
	log_info("The number of mw elements returned is %ld", *num_elements_mw);
#endif

	*outputkeymap = mapping_existing_new;
	*output_max_min = min_max_list;
}


void get_compressed_string(char *in_str, char** ret_str) {
	UT_string *s;
	utstring_new(s);
	compress_string(in_str, s);
	*ret_str = strdup(utstring_body(s));
	utstring_free(s);
}
void get_all_fhash_list(struct base_data_struct* orig_data_pointer_list, UT_array **ret_fhash_data)
{
#ifdef DEBUG
	log_info("Inside get_all_fhash_list");
#endif

	utarray_new(*ret_fhash_data, &ut_str_icd);

	struct fhash_data *current_ele, *tmp;
	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
		char *c = current_ele->fhash;
		utarray_push_back((*ret_fhash_data), &c);
	}
}

