// helper functions that are used by bcs and clone service . in backend_cluster_helper.c
struct fhash_key_values* get_keys_values_ref(struct base_data_struct* orig_data_pointer_list, char* fhash);
V_TYPE get_normalized_value(UNORM_V_TYPE value, UNORM_V_TYPE minvalue, UNORM_V_TYPE maxvalue);
int get_mapped_key(K_TYPE key, struct key_map** keymapping);
void get_mapped_normalized_value_dense(K_TYPE *in_keys, UNORM_V_TYPE *in_values, K_TYPE num_dim,
		struct key_map** mapping_old_to_new, struct key_min_max** min_max_list, V_TYPE **out_norm_values);
void get_mapped_normalized_value_sparse(K_TYPE *in_keys, UNORM_V_TYPE *in_values, K_TYPE num_dim,
		struct key_map** mapping_old_to_new, struct key_min_max** min_max_list, V_TYPE **out_norm_values,
		K_TYPE **out_norm_keys);
bool get_min_max(K_TYPE key, struct key_min_max** min_max_list, UNORM_V_TYPE* minvalue, UNORM_V_TYPE* maxvalue);
bool is_fhash_in_ignore(char* ele, struct strkey_to_empty *ignore_fhash_list);
int create_raw_lib_svm_sparse(K_TYPE *keys, UNORM_V_TYPE* values, int label, K_TYPE key_dimension,
		UT_string **tmp_svm_data, bool include_label);
int create_lib_svm_sparse(K_TYPE *actualkeyindx, V_TYPE* normalized_data, int label, K_TYPE key_dimension,
		UT_string **tmp_svm_data, bool include_label, bool append_to_string);
int knn_cmp_rev(struct nn_data *a, struct nn_data *b);
int knn_cmp(struct nn_data *a, struct nn_data *b);
void create_normalized_svm_data_mrmr(struct base_data_struct* orig_data_pointer_list, struct key_min_max** min_max_list, struct key_map** mapping_old_to_new,
		char **retsvmstr, size_t* num_element_gs, size_t* num_element_mw);
void do_normalization(K_TYPE* inkeys, UNORM_V_TYPE* invalues, K_TYPE in_dim, V_TYPE* outvalues,
		struct key_min_max **in_orig_min_max);
int sorting_key_list(struct key_min_max* key1, struct key_min_max* key2);
void addkeymapping(K_TYPE oldkey, K_TYPE newkey, struct key_map** keymapping);
void create_mapping(struct key_min_max** min_max_list, struct key_map** mapping_existing_new, size_t total_elements);
void create_original_mapping(struct key_min_max** min_max_list, struct key_map** mapping_existing_existing);
void update_min_max(K_TYPE mykey, UNORM_V_TYPE value, struct key_min_max** min_max_list);
void libsvmstr_key_values(char* svmstr, int* label, K_TYPE** keys, UNORM_V_TYPE** values, K_TYPE* count);
void create_svm_str(K_TYPE *in_keys, UNORM_V_TYPE *in_values, K_TYPE in_dimension, int in_label,
		struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char** retlibsvmstr);
void get_min_max_list(struct base_data_struct* orig_data_pointer_list, struct key_min_max** min_max_list, size_t *num_elements);
void get_mapping_dict(struct key_map** mapping_existing_new, struct key_min_max** min_max_list, size_t num_elements);
// Hash methods
struct hashkey_to_empty* create_hash_entry(struct base_data_struct* orig_data_pointer_list, char* hashtoadd);
struct hashkey_to_empty* get_hash_entry(struct base_data_struct* orig_data_pointer_list, char* hashtoadd);

void fill_k_nn_data(struct base_data_struct* orig_data_pointer_list, struct nn_data **k_nn_data, struct key_min_max **in_orig_min_max, K_TYPE **in_keys, V_TYPE **in_norm_values, K_TYPE in_dimension, int k);
void k_nn_keep_topk(struct nn_data **k_nn_data, int k);
void display_k_nn_data(struct nn_data **k_nn_data);
