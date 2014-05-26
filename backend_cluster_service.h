// methods used exclusively by bcs service. function defination in backend_cluser_service.c
void initialize_backend_cluster_service(struct base_data_struct** orig_data_pointer_list);
void add_data_fhash_data_list(struct base_data_struct* orig_data_pointer_list, char* fhash, char* md5, char* sha, char* base_sha, double timestamp, int priority,
		int label, char* svmdatastr, int is_orig, bool *is_retraining, int* curr_count, bool* new_data_added);
void add_additional_data_fhash_data_list(struct base_data_struct* orig_data_pointer_list, char* fhash, char* md5, char* sha, char* base_sha, double timestamp,
		int priority, int label);
void delete_fhash_data(struct base_data_struct* orig_data_pointer_list, char* fhash);
void add_enum_sha_md5(struct base_data_struct* orig_data_pointer_list, char* sha, char* md5);
void set_sha_fhash(struct base_data_struct* orig_data_pointer_list, char* sha, char* fhash);
void delete_sha_fhash(struct base_data_struct* orig_data_pointer_list, char* sha);
void set_md5_fhash(struct base_data_struct* orig_data_pointer_list, char* md5, char* fhash);
void delete_md5_fhash(struct base_data_struct* orig_data_pointer_list, char* md5);
void add_fhash_data(struct base_data_struct* orig_data_pointer_list, char* fhash, char* svmdatastr, int is_orig);
struct hashdata* find_new_default(struct hashdata* head);
bool change_default(struct hashdata* current_default, struct hashdata* newdata);

