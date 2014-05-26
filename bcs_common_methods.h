// common get functions -- get methods used by both bcs and clone service.
//bcs_common_methods.c
void get_libsvm_keys_values_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, int* num_dim, K_TYPE **keys, UNORM_V_TYPE **values);
void getlibsvmdata_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, char **retsvmstr);
void get_raw_data(struct base_data_struct* orig_data_pointer_list, char** svm_raw_str, size_t *num_elements_gs, size_t *num_elements_mw);
void do_minimize_dimension_get_map(struct base_data_struct* orig_data_pointer_list, struct key_map **outputkeymap, struct key_min_max **output_max_min, K_TYPE *num_old_dim, K_TYPE *num_new_dim);
void get_orig_map(struct base_data_struct* orig_data_pointer_list, struct key_map **outputkeymap, struct key_min_max **output_max_min, K_TYPE *num_old_dim, K_TYPE *num_new_dim);
void get_k_nn_fhashes(struct base_data_struct* orig_data_pointer_list, char* input_fhash, int k, struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data);
void get_k_nn(struct base_data_struct* orig_data_pointer_list, char* in_svmstr, int k, struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data);
void get_fhash_from_sha(struct base_data_struct* orig_data_pointer_list, char* sha, char **fhash);
void get_fhash_from_md5(struct base_data_struct* orig_data_pointer_list, char* md5, char **fhash);
void get_data_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, struct fhash_data **retdata);
void get_orig_counts(struct base_data_struct* orig_data_pointer_list, int *num_elements_gs, int *num_elements_mw);
int is_orig_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash);
void get_current_fhash_data_list_count(struct base_data_struct* orig_data_pointer_list, int* curr_count);
void get_normalized_str(struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char* in_svmstr,
		char** retlibsvmstr);
void get_predict_svm_str_from_sha(struct base_data_struct* orig_data_pointer_list, struct key_min_max **key_min_max_list, struct key_map **old_new_keymap, char* sha,
		char **retlibsvmstr);
void get_predict_svm_str_from_fhash(struct base_data_struct* orig_data_pointer_list, struct key_min_max **key_min_max_list, struct key_map **old_new_keymap, char* fhash,
		char **retlibsvmstr, int* is_orig);
int get_label_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash);
void get_normalized_svm_data_mrmr(struct base_data_struct* orig_data_pointer_list, char **retsvmstr, size_t *num_gs, size_t *num_mw, struct key_map **outputkeymap,
		struct key_min_max **minmaxlist);
void get_compressed_string(char *in_str, char** ret_str);
void get_all_fhash_list(struct base_data_struct* orig_data_pointer_list, UT_array **ret_fhash_data);

// common hash functions, nethods in bcs_hash_entry.c




// test functions
void create_dummy_map(struct key_map **old_new_map);
struct fhash_data* read_fhash_data(char* fhash);
void get_data_from_sha(char* sha, char** fhash, char** md5, char** basesha, double* timestamp, int* label,
		int* priority);
void add_key_value_data(struct key_min_max **orig_key);
