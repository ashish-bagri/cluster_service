/**
 * The data structures that are used by both backend cluster service and by backend clone service
 *
 *
 */

//#define DEBUG

struct base_data_struct{
	struct fhash_data *fhash_data_list;
	struct anomaly_data *anomaly_data_list;
	struct common_data_pointer* common_data_pointer_ref;
	UT_array *enum_hashes_list;
	struct key_min_max *key_min_max_list;
};

struct anomaly_data{
	int hashlist_id;
	int anomaly_type;	// 1 = mw / 2 = gs / 3 = mixed
	int anomaly_type_method; // anomaly for ea/mrmr/.. 1 = mrmr, 2 = ea
	UT_array *orig_fhash_list_mw;
	UT_array *orig_fhash_list_gs;
	UT_hash_handle hh; /* makes this structure hashable */
};

struct common_data_pointer
{
	struct hashkey_to_empty *hashkey_to_empty_list;
	struct md5_fhash *md5_fhash_list;
	struct sha_fhash *sha_fhash_list;
	struct fhash_key_values *fhash_key_values_list;

};


struct fhash_data {
	char* fhash; /* key : fhash */
	int label;
	int is_orig; // if the hash is original to this cluster
	bool deleted; // if the feature hash is marked deleted
	char* default_sha;
	struct hashdata* duplicate_list_head;
	int mrmr_anomaly_key;
	double mw_prob;
	UT_hash_handle hh; /* makes this structure hashable */
};

struct fhash_key_values {
	char* fhash; /* key : fhash */
	K_TYPE num_dimensions; //Number of dimensions in the svm data
	K_TYPE* keys; // the keys of the svm data - int
	UNORM_V_TYPE* values; // the values of the svm data - floats
	UT_hash_handle hh; /* makes this structure hashable */
};


struct md5_fhash {
	const char* md5; /* key : MD5 */
	char* fhash; /* value : Feature Hash */
	UT_hash_handle hh; /* makes this structure hashable */
};

struct sha_fhash {
	const char* sha; /* key : SHA */
	char* fhash; /* value : Feature Hash */
	UT_hash_handle hh; /* makes this structure hashable */
};

struct strkey_to_empty {
	char* key;
	unsigned int value;
	UT_hash_handle hh;
};

struct hashkey_to_empty {
	char key[65];
	unsigned int value;
	UT_hash_handle hh;
};


struct hashdata {
	char* sha; // sha is the key
	char* md5;
	char* base_sha;
	double timestamp;
	int priority;
	int label;
	UT_hash_handle hh;
	//struct hashdata *next; /* needed for singly- or doubly-linked lists */
};

struct key_min_max {
	K_TYPE key;
	UNORM_V_TYPE minvalue;
	UNORM_V_TYPE maxvalue;
	size_t count_key;
	UT_hash_handle hh; /* makes this structure hashable */
};

struct key_map {
	K_TYPE key;
	K_TYPE mappedkey;
	UT_hash_handle hh; /* makes this structure hashable */
};

struct nn_data {
	char* sha;
	V_TYPE dist;
	char *fhash;
	struct nn_data *next; /* needed for singly- or doubly-linked lists */
};

struct fhash_fhash {
	char* mapped_fhash;
	UT_array *orig_fhash_list_mw;
	UT_array *orig_fhash_list_gs;
	UT_hash_handle hh; /* makes this structure hashable */
};

struct enum_hashes {
	int enum_id;
	char* sha;
	char* md5;
};



