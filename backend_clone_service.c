#include "backend_service.h"
#include "../swig_callbacks/swig_callbacks.h"

//#define DEBUG

/*
 * Methods that are exclusively used by backend_clone_service
 *
*/
void free_clone_pointer(struct base_data_struct *training_clone_list) {

	// deletes all the data in fhash_data_list
	cleanup_fhash_data(&(training_clone_list->fhash_data_list));
	// delete data in anomaly_data_list
	clean_anomaly_data(&(training_clone_list->anomaly_data_list));

	training_clone_list->common_data_pointer_ref = NULL;

	if(training_clone_list->enum_hashes_list != NULL)
	{
		utarray_free(training_clone_list->enum_hashes_list);
	}

	cleanup_key_min_max(&(training_clone_list->key_min_max_list));
	free(training_clone_list);
}

void dispaly_cloned_data(struct base_data_struct *training_clone_list) {
	struct fhash_data *cloned_fhash_datalist = training_clone_list->fhash_data_list;

	struct fhash_data *current_ele, *tmp;
	HASH_ITER(hh, cloned_fhash_datalist, current_ele, tmp)
	{
		log_info("processing hash %s with label: %d Mrmrkey: %d", current_ele->fhash, current_ele->label, current_ele->mrmr_anomaly_key);
	}
}

void clone_training_data(struct base_data_struct* orig_data_pointer_list, struct base_data_struct **training_clone_list) {
#ifdef DEBUG
	log_info("Inside clone_training_data");
#endif
	*training_clone_list = malloc(sizeof(struct base_data_struct));

	(*training_clone_list)->fhash_data_list = NULL;

	(*training_clone_list)->common_data_pointer_ref = orig_data_pointer_list->common_data_pointer_ref;

	(*training_clone_list)->key_min_max_list = NULL;
	(*training_clone_list)->anomaly_data_list = NULL;
	(*training_clone_list)->enum_hashes_list = NULL;
	(*training_clone_list)->enum_hashes_list = 0;

	size_t num_elements = 0;
	get_min_max_list(orig_data_pointer_list, &((*training_clone_list)->key_min_max_list), &num_elements);

	struct fhash_data *cloned_fhash_datalist = NULL;

	struct fhash_data *current_ele, *tmp;

	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
#ifdef DEBUG
		log_info("processing hash %s\n", current_ele->fhash);
#endif
/*
		if (current_ele->deleted) {
#ifdef DEBUG
			log_info("This hash %s has been deleted\n", current_ele->fhash);
#endif
			continue;
		}
*/
		struct hashkey_to_empty *hashentry_fhash = create_hash_entry(orig_data_pointer_list, current_ele->fhash);
		struct fhash_data *s;
		s = malloc(sizeof(struct fhash_data));
		s->fhash = hashentry_fhash->key;
		s->label = current_ele->label;
		s->is_orig = current_ele->is_orig;
		s->deleted = current_ele->deleted;
		s->default_sha = current_ele->default_sha;
		s->duplicate_list_head = NULL;
		s->mw_prob = 0.0;
		s->mrmr_anomaly_key = -1;
		// fill in the duplicate list :

		struct hashdata *current, *tmphashdata;
		HASH_ITER(hh, current_ele->duplicate_list_head, current, tmphashdata)
		{
			struct hashdata *newdata;
			newdata = malloc(sizeof(struct hashdata));

			struct hashkey_to_empty *hashentry_sha = create_hash_entry(orig_data_pointer_list, current->sha);
			struct hashkey_to_empty *hashentry_md5 = create_hash_entry(orig_data_pointer_list, current->md5);
			struct hashkey_to_empty *hashentry_basesha = create_hash_entry(orig_data_pointer_list, current->base_sha);

			newdata->sha = hashentry_sha->key;
			newdata->md5 = hashentry_md5->key;
			newdata->base_sha = hashentry_basesha->key;

			newdata->timestamp = current->timestamp;
			newdata->priority = current->priority;
			newdata->label = current->label;
			HASH_ADD_KEYPTR(hh, s->duplicate_list_head, newdata->sha, strlen(newdata->sha), newdata);
		}
		HASH_ADD_KEYPTR(hh, cloned_fhash_datalist, s->fhash, strlen(s->fhash), s);
	}
	(*training_clone_list)->fhash_data_list = cloned_fhash_datalist;
#ifdef DEBUG
	log_info("Completed cloned data");
#endif
}

/*
 * SWig interface creates the anomaly_data struct
 */
void add_mrmr_ana(struct base_data_struct *training_clone_list, struct anomaly_data **anomaly_data_list)
{
#ifdef DEBUG
	log_info("Inside add_mrmr_ana");
#endif
	training_clone_list->anomaly_data_list = *anomaly_data_list;
}

/*
 * Adds the mrmr anomaly to the cloned data
 */
void update_clone_mrmr_anomaly(struct base_data_struct *training_clone_list, struct strkey_to_empty **fhash_hashey)
{
#ifdef DEBUG
	log_info("Inside update_clone_mrmr_anomaly");
#endif
	struct strkey_to_empty *current_ele, *tmp;
	struct fhash_data *cloned_fhash_datalist = training_clone_list->fhash_data_list;
	HASH_ITER(hh, *fhash_hashey, current_ele, tmp)
	{
		char* fhash = current_ele->key;
		int mrmr_mapped = current_ele->value;
#ifdef DEBUG
		log_info("Hash: %s mapped: %d",fhash, mrmr_mapped);
#endif
		struct fhash_data *s;
		HASH_FIND_STR(cloned_fhash_datalist, fhash, s);
		if(s == NULL)
		{
			log_info("No info about hash: %s in cloned data \n", fhash);
		}else
		{
			s->mrmr_anomaly_key = mrmr_mapped;
		}
	}
}

void get_anomaly_data_from_key(struct base_data_struct *training_clone_list, int anomaly_id)
{
	return;
}

/** get_crossValidation_data
 * python input:
 * Map used to reduce dimension: dict : {oldkey: (newkey, min, max) }
 * Map converted into 2 struct in swig -> struct key_map and struct key_min_max
 *
 *fhash to ignore
 *converted into a utarray in swig
 *
 *goes through all the existing data. ignores fhash which are deleted or are in ignore list (input)
 *returns back (compressed_cv_lsvm_str, cv_fhash_list, , no_mw_samples )
 *compressed_cv_lsvm_str : sparse lib svm data
 *  cv_fhash_list: list of fhash associated with the lib svm data (in order)
 *
 */
void get_crossValidation_data(struct base_data_struct* orig_data_pointer_list, struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list,
		struct strkey_to_empty **ignore_fhash_list, char **compressed_cv_lsvm_str, UT_array **ret_fhash_data,
		size_t *num_gs_samples, size_t *num_mw_samples, size_t *len_unpacked_str, bool is_compressed) {
#ifdef DEBUG
	log_info("Inside get_crossValidation_data");

#endif

	// create datastructures need for return values
	UT_string *libsvmstr_mw;
	utstring_new(libsvmstr_mw);
	UT_string *libsvmstr_gs;
	utstring_new(libsvmstr_gs);
	utarray_new(*ret_fhash_data, &ut_str_icd);

	int gs_initialized = 0;
	int mw_initialized = 0;

	*num_gs_samples = 0;
	*num_mw_samples = 0;

	K_TYPE num_new_dimensions;
	num_new_dimensions = HASH_COUNT(*mapping_old_to_new);

	size_t count_fhash_data_list = HASH_COUNT(orig_data_pointer_list->fhash_data_list);
	size_t curr_count = 0;
	size_t curr_per = 1;
	log_info("BCS: Inside cross-validation-data. The count of fhash data list is %zu",count_fhash_data_list);
	bool toprintstatus = false;
	if(count_fhash_data_list  > (size_t)(1000))
	{
		toprintstatus = true;
	}
	struct fhash_data *current_ele, *tmp;

	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
		if (current_ele->deleted) continue;
		if (is_fhash_in_ignore(current_ele->fhash, *ignore_fhash_list) == true) continue;

		int label = current_ele->label;
		if (label == LABEL_CLEAN) {
			*num_gs_samples = *num_gs_samples + 1;
		} else if (label == LABEL_MW) {
			*num_mw_samples = *num_mw_samples + 1;
		} else {
			log_info("ERROR: Unknown label type %d", label);
		}
	}
	log_info("BCS: Cross-validation-counts: GS: %zu MW: %zu", *num_gs_samples, *num_mw_samples);
	current_ele = NULL;
	tmp = NULL;
	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
		if (is_stop_set()) break;

#ifdef DEBUG
		log_info("processing hash %s", current_ele->fhash);
#endif
		int label = current_ele->label;
		// check if the elemet is to be ignored.
		if (is_fhash_in_ignore(current_ele->fhash, *ignore_fhash_list) == true) {
				//log_info("Ignoring feature hash with label %d: %s ", label, current_ele->fhash);
				curr_count = curr_count + 1;
				continue;
		}
		if (current_ele->deleted) {
			log_info("Ignoring feature hash as deleted: %s", current_ele->fhash);
			curr_count = curr_count + 1;
			continue;
		}

		if( curr_count > (size_t)(curr_per * count_fhash_data_list / (size_t)(100)) && toprintstatus == true)
		{
			log_info("Processed %zu out of %zu (%d percent)",curr_count, count_fhash_data_list, curr_per);
			curr_per ++;
		}

		if (label != LABEL_CLEAN && label != LABEL_MW)
		{
			log_info("ERROR: Unknown label type %d", label);
			curr_count = curr_count + 1;
			continue;
		}

		int retdata = 0;
		V_TYPE *normalized_data;
		K_TYPE *actualnewkeys;
		// get keys and values
		struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, current_ele->fhash);


		get_mapped_normalized_value_sparse(keys_values->keys, keys_values->values, keys_values->num_dimensions,
				mapping_old_to_new, oldkey_min_max_list, &normalized_data, &actualnewkeys);
		if (label == LABEL_CLEAN) {
			retdata = create_lib_svm_sparse(actualnewkeys, normalized_data, label, keys_values->num_dimensions, &libsvmstr_gs, true, true);
			if (!gs_initialized) {
#ifdef DEBUG
				log_info("Trying to reserve utstring for gs");
#endif
				size_t utlgs = (size_t)utstring_len(libsvmstr_gs);
#ifdef DEBUG
				log_info("The utstring len of gs is: %zu", utlgs);
#endif
				size_t size_init = utlgs * (*num_gs_samples + (0.1 * (*num_gs_samples)));
#ifdef DEBUG
				log_info("Size of reservation of gs is: %zu", size_init);
#endif
				utstring_reserve(libsvmstr_gs, size_init);
#ifdef DEBUG
				log_info("Completed utstring_reserve for gs");
#endif
				gs_initialized = 1;
			}
			char * c = current_ele->fhash;
			utarray_push_back((*ret_fhash_data), &c);
		} else if (label == LABEL_MW) {
			retdata = create_lib_svm_sparse(actualnewkeys, normalized_data, label, keys_values->num_dimensions, &libsvmstr_mw, true, true);

			if (!mw_initialized) {
#ifdef DEBUG
				log_info("Trying to reserve utstring for mw");
#endif
				size_t utlmw = (size_t)utstring_len(libsvmstr_mw);
#ifdef DEBUG
				log_info("The utstring len of mw is: %zu", utlmw);
#endif
				size_t size_init = utlmw * (*num_mw_samples + (0.1 * (*num_mw_samples)));
#ifdef DEBUG
				log_info("Size of variable size_init is %zu", sizeof(size_init));
				log_info("Size of reservation of mw is: %zu", size_init);
#endif
				utstring_reserve(libsvmstr_mw, size_init);
#ifdef DEBUG
				log_info("Completed utstring_reserve for mw");
#endif
				mw_initialized = 1;
			}
			char * c = current_ele->fhash;
			utarray_push_back((*ret_fhash_data), &c);
		}
		free(normalized_data);
		free(actualnewkeys);
		curr_count = curr_count + 1;
	}
	if (is_stop_set())
	{
		utstring_free(libsvmstr_mw);
		utstring_free(libsvmstr_gs);
		cleanup_strkey_to_empty(ignore_fhash_list);
		return NULL;
	}
#ifdef DEBUG
	log_info("Completed the loop processing all the feature hashes");
#endif
	utstring_concat(libsvmstr_mw, libsvmstr_gs);
	utstring_free(libsvmstr_gs);

	*len_unpacked_str = (libsvmstr_mw)->i;

	if (is_compressed == true)
	{
		UT_string *s;
		utstring_new(s);
#ifdef DEBUG
		log_info("Started compressing data");
#endif
		compress_string(utstring_body(libsvmstr_mw), s);
#ifdef DEBUG
		log_info("finished compressing data");
#endif
		*compressed_cv_lsvm_str = strdup(utstring_body(s));
		utstring_free(s);
	}
	else
	{
		*compressed_cv_lsvm_str = strdup(utstring_body(libsvmstr_mw));
	}

	utstring_free(libsvmstr_mw);
	// free struct strkey_to_empty **ignore_fhash_list

	cleanup_strkey_to_empty(ignore_fhash_list);
	log_info("Cleanup mapping_old_to_new");
	cleanup_key_map(mapping_old_to_new);
	cleanup_key_min_max(oldkey_min_max_list);

#ifdef DEBUG
	log_info("BCS: Completed get_crossValidation_data");
#endif
}



/**get_ea_data
 * compressed_ea_lsvm_str, ea_fhash_list, ea_fhash_to_anomaly_map, mrmr_mixed_anomalies, mrmr_mw_anomalies, mrmr_gs_anomalies, no_gs_samples, no_mw_samples
 *
 *# compressed_ea_lsvm_str should contain _no_ data corresponding to the mw feature hashes of mrmr_mixed_anomalies
 # compressed_ea_lsvm_str should contain only _one_ data line corresponding to a set of gs feature hashes in  mrmr_mixed_anomalies
 # compressed_ea_lsvm_str should contain only _one_ data line corresponding to a set of gs feature hashes in  mrmr_gs_anomalies
 # compressed_ea_lsvm_str should contain only _one_ data line corresponding to a set of mw feature hashes in  mrmr_mw_anomalies
 # compressed_ea_lsvm_str should contain all data which is not in anomalies
 *
 *
 *  # explanation of parameter: ea_fhash_list
 # item0 of ea_fhash_list is the feature hash corresponding to line0 of compressed_ea_lsvm_str
 # item1 of ea_fhash_list is the feature hash corresponding to line1 of compressed_ea_lsvm_str
 # ...
 #
 #
 # explanation of parameter: ea_fhash_to_anomaly_map
 # ea_fhash_to_anomaly_map should contain all data _inclusive_ the datawhich was skipped in compressed_ea_lsvm_str

 */
void get_ea_data(struct base_data_struct* orig_data_pointer_list, struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list,
		struct fhash_fhash **mapped_orig_fhash_dict) {

#ifdef DEBUG
	log_info("Inside get_crossValidation_data");

#endif

	struct fhash_data *current_ele, *tmp;
	// iterate over all the existing feature hashes
	K_TYPE num_new_dimensions;
	num_new_dimensions = HASH_COUNT(*mapping_old_to_new);
	*mapped_orig_fhash_dict = NULL;

	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
#ifdef DEBUG
		log_info("processing hash %s", current_ele->fhash);
		log_info("default sha is  %s", current_ele->default_sha);
#endif
		if (current_ele->deleted) {
#ifdef DEBUG
			log_info("Fhash: %s is deleted",current_ele->fhash);
#endif
			continue;
		}
		V_TYPE *normalized_data;
		K_TYPE *actualnewkeys;

		struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, current_ele->fhash);

		get_mapped_normalized_value_sparse(keys_values->keys, keys_values->values, keys_values->num_dimensions,
				mapping_old_to_new, oldkey_min_max_list, &normalized_data, &actualnewkeys);

		int label = current_ele->label;
		UT_string *libsvmstr;
		int retdata = create_lib_svm_sparse(actualnewkeys, normalized_data, label, keys_values->num_dimensions, &libsvmstr,
				false, false);

		free(normalized_data);
		free(actualnewkeys);
#ifdef DEBUG
		if (retdata == 0) {
			log_info("WARNING ! Empty svm string.");
		}
#endif
		// calculate the mapped hash value. push it to the structure mapped_orig_fhash_dict
		// once all data is added, then go through the data to find conflicts
		char *mappedhash = utstring_body(libsvmstr);
		struct fhash_fhash *existing;
		HASH_FIND_STR(*mapped_orig_fhash_dict, mappedhash, existing);

		if (existing != NULL) {
#ifdef DEBUG
			log_info("There is an existing mapped feature hash");
#endif
			// There is an existing entry of this mapped hash. update that
			if (label == LABEL_CLEAN) {
				utarray_push_back(existing->orig_fhash_list_gs, &(current_ele->fhash));
			} else if (label == LABEL_MW) {
				utarray_push_back(existing->orig_fhash_list_mw, &(current_ele->fhash));
			} else {
				log_error("ERROR: Unknown label type %d", label);
			}
		} else {
#ifdef DEBUG
			log_info("Adding a new mapped feature hash %s", mappedhash);
#endif
			struct fhash_fhash* newdata = malloc(sizeof(struct fhash_fhash));
			newdata->mapped_fhash = strdup(mappedhash);
			utarray_new(newdata->orig_fhash_list_gs, &ut_str_icd);
			utarray_new(newdata->orig_fhash_list_mw, &ut_str_icd);

			if (label == LABEL_CLEAN) {
				utarray_push_back(newdata->orig_fhash_list_gs, &(current_ele->fhash));
			} else if (label == LABEL_MW) {
				utarray_push_back(newdata->orig_fhash_list_mw, &(current_ele->fhash));
			} else {
				log_info("ERROR: Unknown label type %d", label);
			}
			HASH_ADD_KEYPTR(hh, *mapped_orig_fhash_dict, newdata->mapped_fhash, strlen(mappedhash), newdata);
		}
		utstring_free(libsvmstr);
	}
#ifdef DEBUG
	log_info("Completed building the fhash to fhash data structure");
#endif


}

void update_mw_prob_fhash(struct base_data_struct* training_clone_list, char* fhash, double mw_prob)
{
#ifdef DEBUG
	log_info("Inside update_mw_prob_fhash for fhash: %s", fhash);
#endif
	 struct fhash_data *s;
	HASH_FIND_STR(training_clone_list->fhash_data_list, fhash, s);
	if(s == NULL)
	{
		log_info("No info about hash: %s in cloned data", fhash);
	}else
	{
#ifdef DEBUG
		log_info("Updated Probability Fhash: %s Mw_prob: %f", fhash, mw_prob);
#endif
		s->mw_prob = mw_prob;
	}
}

void get_mw_prob_fhash(struct base_data_struct* training_clone_list, char* fhash, double* prob)
{
#ifdef DEBUG
	log_info("Inside get_mw_prob_fhash for fhash: %s", fhash);
#endif

	double mw_prob = -1;
	struct fhash_data *s;
	HASH_FIND_STR(training_clone_list->fhash_data_list, fhash, s);
	if(s == NULL)
	{
		log_info("No info about hash: %s in cloned data", fhash);
	}else
	{
		mw_prob = s->mw_prob;
	}
	*prob = mw_prob;
}



void get_k_nn_fhashes_clone(struct base_data_struct* orig_data_pointer_list, char* input_fhash, int k, struct nn_data **k_nn_data)
{
#ifdef DEBUG
	log_info("Inside get K nearest neighbor\n");
#endif

	// get keys, values for input feature hash
	struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, input_fhash);
	if (keys_values == NULL)
	{
		*k_nn_data = NULL;
		return;
	}
	K_TYPE* in_keys = keys_values->keys;
	UNORM_V_TYPE* in_values = keys_values->values;
	K_TYPE in_dimension = keys_values->num_dimensions;

	V_TYPE * in_norm_values = malloc(sizeof(V_TYPE) * in_dimension);
// Try using orig_data_pointer_list->key_min_max_list instead of in_orig_min_max

	do_normalization(in_keys, in_values, in_dimension, in_norm_values, &orig_data_pointer_list->key_min_max_list);

	fill_k_nn_data(orig_data_pointer_list, k_nn_data, &orig_data_pointer_list->key_min_max_list, &in_keys, &in_norm_values, in_dimension, k);
	LL_SORT(*k_nn_data, knn_cmp_rev);
	k_nn_keep_topk(k_nn_data, k);
	free(in_norm_values);
}


void get_k_nn_libsvm_clone(struct base_data_struct* orig_data_pointer_list, char* in_svmstr, int k, struct nn_data **k_nn_data) {

	K_TYPE *in_keys;
	UNORM_V_TYPE* in_values;
	int in_label;
	K_TYPE in_dimension;

	libsvmstr_key_values(in_svmstr, &in_label, &in_keys, &in_values, &in_dimension);

	V_TYPE * in_norm_values = malloc(sizeof(V_TYPE) * in_dimension);
	do_normalization(in_keys, in_values, in_dimension, in_norm_values, &orig_data_pointer_list->key_min_max_list);

	free(in_values); // in_values is no longer needed

	fill_k_nn_data(orig_data_pointer_list, k_nn_data, &orig_data_pointer_list->key_min_max_list, &in_keys, &in_norm_values, in_dimension, k);

// finally, must reverse it to have the smallest distance as the first element!
	LL_SORT(*k_nn_data, knn_cmp_rev);

#ifdef DEBUG
	display_k_nn_data(k_nn_data);
#endif

	// keep only top k data
	k_nn_keep_topk(k_nn_data, k);
	free(in_keys);
	free(in_norm_values);
}
