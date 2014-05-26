/*
 * Cloning methods, api methods
 */
void clone_training_data(struct base_data_struct* orig_data_pointer_list, struct base_data_struct **training_clone_list);
void dispaly_cloned_data(struct base_data_struct *training_clone_list);
void free_clone_pointer(struct base_data_struct *training_clone_list);
void add_mrmr_mixed(struct base_data_struct *training_clone_list, struct id_fhashlist **mrmr_mixed_ana);
void add_mrmr_mw(struct base_data_struct *training_clone_list, struct id_fhashlist **mrmr_ana_list);
void update_clone_mrmr_anomaly(struct base_data_struct *training_clone_list, struct strkey_to_empty **fhash_hashey);
void add_mrmr_ana(struct base_data_struct *training_clone_list, struct anomaly_data **anomaly_data_list);
void get_anomaly_data_from_key(struct base_data_struct *training_clone_list, int anomaly_id);
void get_crossValidation_data(struct base_data_struct* orig_data_pointer_list, struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list,
		struct strkey_to_empty **ignore_fhash_list, char **compressed_cv_lsvm_str, UT_array **ret_fhash_data, size_t *num_gs_samples, size_t *num_mw_samples, size_t *len_unpacked_str, bool is_compressed);
void get_ea_data(struct base_data_struct* orig_data_pointer_list, struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list,
		struct fhash_fhash **mapped_orig_fhash_dict);

void add_mw_prob(struct base_data_struct* training_clone_list);
void update_mw_prob_fhash(struct base_data_struct* training_clone_list, char* fhash, double prob);
void get_k_nn_fhashes_clone(struct base_data_struct* orig_data_pointer_list, char* input_fhash, int k, struct nn_data **k_nn_data);
void get_k_nn_libsvm_clone(struct base_data_struct* orig_data_pointer_list, char* in_svmstr, int k, struct nn_data **k_nn_data) ;
