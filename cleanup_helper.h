// cleanup functions - in cleanup_helper.c
void cleanup_strkey_to_empty(struct strkey_to_empty **strkey_to_empty_list);
void cleanup_key_min_max(struct key_min_max **key_min_max_list);
void cleanup_fhash_fhash(struct fhash_fhash **fhash_fhash_list);
void cleanup_key_map(struct key_map **key_map_list);
void cleanup_fhash_keys_values(struct fhash_key_values **fhash_key_values_list);
void cleanup_fhash_data(struct fhash_data **fhash_data_list);
void free_all_data(struct base_data_struct* orig_data_pointer_list);
void cleanup_fhash_data_list(struct base_data_struct* orig_data_pointer_list);
void cleanup_sha_fhash(struct base_data_struct* orig_data_pointer_list);
void cleanup_md5_fhash(struct base_data_struct* orig_data_pointer_list);
void cleanup_nn_data(struct nn_data **nn_data_list);
void cleanup_hashkey_to_empty();
void clean_anomaly_data(struct anomaly_data **anomaly_data_list);
