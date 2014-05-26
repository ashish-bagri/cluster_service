#include "backend_service.h"
#include "../swig_callbacks/swig_callbacks.h"

/*
 * Methods that are used internally by both backend_cluster_service and backend_clone_service
 *
 */

//#define DEBUG

struct fhash_key_values* get_keys_values_ref(struct base_data_struct* orig_data_pointer_list, char* fhash)
{
	struct fhash_key_values *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->fhash_key_values_list, fhash, s);
	if (s == NULL) {
		log_error("ERROR: NO feature hash %s in the database", fhash);
		return NULL;
	}else
	{
		return s;
	}
}


V_TYPE get_normalized_value(UNORM_V_TYPE value, UNORM_V_TYPE minvalue, UNORM_V_TYPE maxvalue) {
	V_TYPE scaled = 0.0;
	if (maxvalue == minvalue) {
		scaled = 1.0;
	} else if (value > maxvalue) {
		scaled = 1.0;
	} else if (value < minvalue) {
		scaled = 0.0;
	} else {
		scaled = value / (V_TYPE) (maxvalue);
	}
	return scaled;
}

int get_mapped_key(K_TYPE key, struct key_map** keymapping) {
	struct key_map *s;
	HASH_FIND(hh, *keymapping, &key, sizeof(K_TYPE), s);
	if (s == NULL) {
		return -1;
	} else {
		int origkey = s->mappedkey;
		return origkey;
	}
}
// dense is using all the new dimensions and filling data according to the new index
void get_mapped_normalized_value_dense(K_TYPE *in_keys, UNORM_V_TYPE *in_values, K_TYPE num_dim,
		struct key_map** mapping_old_to_new, struct key_min_max** min_max_list, V_TYPE **out_norm_values) {

	K_TYPE num_new_dimensions;
	num_new_dimensions = HASH_COUNT(*mapping_old_to_new);

#ifdef DEBUG
	log_info("Num new dimensions is %d \n", num_new_dimensions);
#endif

	*out_norm_values = malloc(sizeof(V_TYPE) * num_new_dimensions);
	K_TYPE tmp = 0;
	for (tmp = 0; tmp < num_new_dimensions; tmp++) {
		(*out_norm_values)[tmp] = 0;
	}
	K_TYPE d = 0;

	for (d = 0; d < num_dim; d++) {

		K_TYPE key = in_keys[d];
		UNORM_V_TYPE value = in_values[d];
		int newkeyint = get_mapped_key(key, mapping_old_to_new);
		if (newkeyint != -1) {
			UNORM_V_TYPE minvalue;
			UNORM_V_TYPE maxvalue;
			bool minmax = get_min_max(key, min_max_list, &minvalue, &maxvalue);
			if (minmax == false) {
				log_info("ERROR: How to handle if min and max not present \n");
			}

			V_TYPE d = get_normalized_value(value, minvalue, maxvalue);
#ifdef DEBUG
			log_info("Oldkey: %d Newkey: %d Min value: %d Max value: %d value: %d Scaled: %0.18g\n", key, newkeyint, minvalue,
					maxvalue, value, d);
#endif
			(*out_norm_values)[newkeyint-1] = d;
		}
#ifdef DEBUG
		else {
			log_info("OldKey %d Value %d. This key is ignored \n", key, value);
		}
#endif
	}
}

// sparse is the output is of the same dim as the input, with keys being mapped. if something is to be dropped, it goes at the end.
// so loop over the key list and use the corresponding value
void get_mapped_normalized_value_sparse(K_TYPE *in_keys, UNORM_V_TYPE *in_values, K_TYPE num_dim,
		struct key_map** mapping_old_to_new, struct key_min_max** min_max_list, V_TYPE **out_norm_values,
		K_TYPE **out_norm_keys) {

	K_TYPE num_new_dimensions;
	num_new_dimensions = HASH_COUNT(*mapping_old_to_new);

#ifdef DEBUG
	log_info("Num new dimensions is %d \n", num_new_dimensions);
#endif

	*out_norm_values = malloc(sizeof(V_TYPE) * num_dim);
	*out_norm_keys = malloc(sizeof(K_TYPE) * num_dim);

	K_TYPE tmp = 0;
	for (tmp = 0; tmp < num_dim; tmp++) {
		(*out_norm_values)[tmp] = 0;
		(*out_norm_keys)[tmp] = 0;
	}
	K_TYPE dindx = 0;

	K_TYPE newkeyindx = 0;

	for (dindx = 0; dindx < num_dim; dindx++) {
		K_TYPE key = in_keys[dindx];
		UNORM_V_TYPE value = in_values[dindx];

		int newkeyint = get_mapped_key(key, mapping_old_to_new);
		if (newkeyint != -1) {
			K_TYPE newkey = (K_TYPE) newkeyint;
			UNORM_V_TYPE minvalue;
			UNORM_V_TYPE maxvalue;
			bool minmax = get_min_max(key, min_max_list, &minvalue, &maxvalue);
			if (minmax == false) {
				log_info("ERROR: How to handle if min and max not present \n");
			}

			V_TYPE scaled_value = get_normalized_value(value, minvalue, maxvalue);
#ifdef DEBUG
			log_info("Oldkey: %d Newkey: %d Min value: %d Max value: %d value: %d Scaled: %0.18g\n", key, newkey, minvalue,
					maxvalue, value, scaled_value);
#endif
			(*out_norm_values)[newkeyindx] = scaled_value;
			(*out_norm_keys)[newkeyindx] = newkey;
			newkeyindx++;
		}
#ifdef DEBUG
		else {
			log_info("OldKey %d Value %d. This key is ignored \n", key, value);
		}
#endif
	}
}

// TODO: error handling when the requested key is not in the map
bool get_min_max(K_TYPE key, struct key_min_max** min_max_list, UNORM_V_TYPE* minvalue, UNORM_V_TYPE* maxvalue) {
	struct key_min_max *s;
	HASH_FIND(hh, *min_max_list, &key, sizeof(K_TYPE), s);
	if (s != NULL) {
		*minvalue = s->minvalue;
		*maxvalue = s->maxvalue;
#ifdef DEBUG
		log_info("Key: %d Min value: %u Max value: %u",key, *minvalue, *maxvalue);
#endif
		return true;
	} else {
		return false;
	}
}

bool is_fhash_in_ignore(char* ele, struct strkey_to_empty *ignore_fhash_list) {

	struct strkey_to_empty *existing;
	HASH_FIND_STR(ignore_fhash_list, ele, existing);

	if (existing == NULL) {
		return false;
	} else {
		return true;
	}

}

int create_raw_lib_svm_sparse(K_TYPE *keys, UNORM_V_TYPE* values, int label, K_TYPE key_dimension,
		UT_string **tmp_svm_data, bool include_label) {

	int tmpnewdim = 0;
	utstring_new(*tmp_svm_data);

	// reserve 2k per string upfront
	utstring_reserve(*tmp_svm_data, 2048);

	// add label
	if (include_label == true) {
		utstring_printf(*tmp_svm_data, "%i", label);
	}

	int num_non_zero = 0;

	for (tmpnewdim = 0; tmpnewdim < key_dimension; tmpnewdim++) {
		if (values[tmpnewdim] != 0) {
			utstring_printf(*tmp_svm_data, " %u:%u", keys[tmpnewdim], values[tmpnewdim]);
		}
	}

	utstring_printf(*tmp_svm_data, "\n");

#ifdef DEBUG
	log_info("The svm string is %s", utstring_body(*tmp_svm_data));
#endif
	return num_non_zero;
}

int create_lib_svm_sparse(K_TYPE *actualkeyindx, V_TYPE* actualsvmdata, int label, K_TYPE key_dimension,
		UT_string **tmp_svm_data, bool include_label, bool append_to_string) {

	K_TYPE tmpnewdim = 0;

	if (append_to_string == false)
	{
#ifdef DEBUG
		log_info("Creating a new UT string for label: %d \n", label);
#endif
		utstring_new(*tmp_svm_data);
		// reserve 2k per string upfront
		utstring_reserve(*tmp_svm_data, 2048);
	}
	// add label
	if (include_label == true) {
		utstring_printf(*tmp_svm_data, "%i", label);
	}

	K_TYPE num_non_zero = 0;
	for (tmpnewdim = 0; tmpnewdim < key_dimension; tmpnewdim++) {
		if (actualsvmdata[tmpnewdim] != 0) {
			num_non_zero = num_non_zero + 1;
			utstring_printf(*tmp_svm_data, " %u:%0.18g", actualkeyindx[tmpnewdim], actualsvmdata[tmpnewdim]);
		}
	}

	utstring_printf(*tmp_svm_data, "\n");

#ifdef DEBUG
	log_info("The svm string is %s", utstring_body(*tmp_svm_data));
#endif
	return num_non_zero;
}

int knn_cmp_rev(struct nn_data *a, struct nn_data *b) {
	if (a->dist < b->dist)
		return -1;
	if (a->dist == b->dist)
		return 0;
	return 1;

}

int knn_cmp(struct nn_data *a, struct nn_data *b) {
	if (a->dist > b->dist)
		return -1;
	if (a->dist == b->dist)
		return 0;
	return 1;
}

// method that applies logic to determine if the newdata is to replace the currentdefault data as the new default
void create_normalized_svm_data_mrmr(struct base_data_struct* orig_data_pointer_list, struct key_min_max** min_max_list, struct key_map** mapping_old_to_new,
		char **retsvmstr, size_t *num_element_gs, size_t *num_element_mw) {

	*num_element_gs = 0;
	*num_element_mw = 0;

	struct fhash_data *current_ele, *tmp;
	UT_string *csv_svm_data; // the string demanded by the user
	utstring_new(csv_svm_data);
	// reserve upfront space

	utstring_reserve(csv_svm_data, 2048);

	utstring_printf(csv_svm_data, "class,");

	K_TYPE num_new_dimensions;
	num_new_dimensions = HASH_COUNT(*mapping_old_to_new);

#ifdef DEBUG
	log_info("Number of new dimensions : %d\n", num_new_dimensions);
#endif

	if (num_new_dimensions == 0) {
		log_debug("No MRMR string to create. Must return empty\n");
		*num_element_gs = 0;
		*num_element_mw = 0;
		*retsvmstr = NULL;
		return;
	}

	int t = 0;
	for (t = 1; t < num_new_dimensions; t++) {
		utstring_printf(csv_svm_data, "%u,", t);
	}
	utstring_printf(csv_svm_data, "%u\n", num_new_dimensions);

#ifdef DEBUG
	log_info("Utstring yet is %s \n", utstring_body(csv_svm_data));
#endif

	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
#ifdef DEBUG
		log_info("processing hash %s\n", current_ele->fhash);
#endif
		if (current_ele->deleted) {
#ifdef DEBUG
			log_info("This hash %s has been deleted\n", current_ele->fhash);
#endif
			continue;
		}
		V_TYPE *normalized_svm_data;

		struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, current_ele->fhash);

		get_mapped_normalized_value_dense(keys_values->keys, keys_values->values, keys_values->num_dimensions,
				mapping_old_to_new, min_max_list, &normalized_svm_data);

		K_TYPE tmpnewdim = 0;
		UT_string *tmp_svm_data;
		utstring_new(tmp_svm_data);
		utstring_reserve(tmp_svm_data, 2048);

		utstring_printf(tmp_svm_data, "%i,", current_ele->label);

		for (tmpnewdim = 0; tmpnewdim < num_new_dimensions-1; tmpnewdim++) {
#ifdef DEBUG
			log_info("Mapped key : %u data: %0.18g \n",  tmpnewdim+1, normalized_svm_data[tmpnewdim]);
#endif

			if (normalized_svm_data[tmpnewdim] != 0) {
				utstring_printf(tmp_svm_data, "%0.18g,", normalized_svm_data[tmpnewdim]);
			} else {
				utstring_printf(tmp_svm_data, "%d,", 0);
			}
		}
		tmpnewdim = num_new_dimensions - 1;
		if (normalized_svm_data[tmpnewdim] != 0) {
			utstring_printf(tmp_svm_data, "%0.18g\n", normalized_svm_data[tmpnewdim]);
		} else {
			utstring_printf(tmp_svm_data, "0\n");
		}
		utstring_concat(csv_svm_data, tmp_svm_data);
#ifdef DEBUG
		log_info("The svm string is %s \n", utstring_body(tmp_svm_data));
#endif

		if (current_ele->label == LABEL_MW) {
			*num_element_mw = *num_element_mw + 1;
		} else if (current_ele->label == LABEL_CLEAN) {
			*num_element_gs = *num_element_gs + 1;
		} else {
			log_info("ERROR! UNKNOWN LABEL KIND %d\n", current_ele->label);
		}

		utstring_free(tmp_svm_data);
		free(normalized_svm_data);
	}

	UT_string *s;
	utstring_new(s);

	compress_string(utstring_body(csv_svm_data), s);

	utstring_free(csv_svm_data);

	*retsvmstr = strdup(utstring_body(s));

	utstring_free(s);
}

void do_normalization(K_TYPE* inkeys, UNORM_V_TYPE* invalues, K_TYPE in_dim, V_TYPE* outvalues,
		struct key_min_max **in_orig_min_max) {
	K_TYPE d = 0;
	for (d = 0; d < in_dim; d++) {
		K_TYPE key = inkeys[d];
		UNORM_V_TYPE value = invalues[d];
		UNORM_V_TYPE minvalue;
		UNORM_V_TYPE maxvalue;
		bool minmax = get_min_max(key, in_orig_min_max, &minvalue, &maxvalue);
		if (minmax == false) {
			log_debug("DEBUG: Key: %d not present in the map. Ignoring\n", key);
			outvalues[d] = 0.0;
		}else
		{
			V_TYPE normalized = get_normalized_value(value, minvalue, maxvalue);
#ifdef DEBUG
		log_info("Oldkey: %d Min value: %d Max value: %d value: %d Scaled: %0.18g\n", key, minvalue,
							maxvalue, value, normalized);
#endif
		outvalues[d] = normalized;
	}
	}
}

int sorting_key_list(struct key_min_max* key1, struct key_min_max* key2) {
	if (key1->key < key2->key)
		return -1;
	else if (key1->key == key2->key)
		return 0;
	else
		return 1;
}

void addkeymapping(K_TYPE key_data, K_TYPE value_data, struct key_map** keymapping) {
	struct key_map *s;
	s = malloc(sizeof(struct key_map));
	s->key = key_data;
	s->mappedkey = value_data;
	HASH_ADD(hh, *keymapping, key, (sizeof(K_TYPE)), s);
#ifdef DEBUG
	log_info("Added mapping between %d -> %d \n", key_data, value_data);
#endif
}

// takes in the min and max list and creates a mapping to dimensions which are not needed/used
// checks which are not set, checks if min and max are same
void create_mapping(struct key_min_max** min_max_list, struct key_map** mapping_existing_new, size_t total_elements) {
// sort the key list
	HASH_SORT(*min_max_list, sorting_key_list);

// run through the key list, creating another map. if there is an id to be discarded or missing, note it for the map

	struct key_min_max *keypointer;
	keypointer = *min_max_list;

	K_TYPE runningkey = (*min_max_list)->key;
	K_TYPE lastkey = (*min_max_list)->key;

	K_TYPE current_new_key = 0;
	for (keypointer = *min_max_list; keypointer != NULL; keypointer = keypointer->hh.next) {
// except the keys to be sorted now ! make a check for it
		K_TYPE oldkey = keypointer->key;
		size_t key_occurences = keypointer->count_key;
		if (key_occurences <  total_elements)
		{
#ifdef DEBUG
			log_info("The key: %d did not occur always. Will use it in the mapping\n",oldkey);
#endif
			current_new_key = current_new_key + 1;
			addkeymapping(oldkey, current_new_key, mapping_existing_new);
		}else if (keypointer->minvalue != keypointer->maxvalue) {
			// need to add this key !
			current_new_key = current_new_key + 1;
			addkeymapping(oldkey, current_new_key, mapping_existing_new);
		}
	}
}

void create_original_mapping(struct key_min_max** min_max_list, struct key_map** mapping_existing_existing) {
#ifdef DEBUG
	log_info("Inside create_original_mapping");
#endif
	// sort the key list
	HASH_SORT(*min_max_list, sorting_key_list);
	int num_ele_after = HASH_COUNT(*min_max_list);
	struct key_min_max *keypointer;
	keypointer = *min_max_list;

	K_TYPE runningkey = (*min_max_list)->key;
	K_TYPE lastkey = (*min_max_list)->key;

	//K_TYPE new_key = 0;

	for (keypointer = *min_max_list; keypointer != NULL; keypointer = keypointer->hh.next) {
		//new_key = new_key + 1;
		K_TYPE oldkey = keypointer->key;
		addkeymapping(oldkey, oldkey, mapping_existing_existing);
	}
}

// adds key if not present, updates min and max for this key depending of the value
void update_min_max(K_TYPE mykey, UNORM_V_TYPE value, struct key_min_max** min_max_list) {
struct key_min_max *s;
HASH_FIND(hh, *min_max_list, &mykey, sizeof(K_TYPE), s);
if (s != NULL) {
	s->count_key += 1;
// key already present, check for min and max
	if (value >= s->minvalue && value <= s->maxvalue) {
		return;
	} else {
		if (value < s->minvalue) {
			s->minvalue = value;
		}
		if (value > s->maxvalue) {
			s->maxvalue = value;
		}
	}
} else {
// add this new key
	struct key_min_max *newkey = NULL;
	newkey = (struct key_min_max *) malloc(sizeof(struct key_min_max));
	newkey->key = mykey;
	newkey->minvalue = value;
	newkey->maxvalue = value;
	newkey->count_key = 1;
	HASH_ADD(hh, *min_max_list, key, (sizeof(K_TYPE)), newkey);
}
}

void libsvmstr_key_values(char* in_svmstr, int* label, K_TYPE** keys, UNORM_V_TYPE** values, K_TYPE* totalcount) {

char *svmstr = strdup(in_svmstr);
char *savestr;
int position_in_str = 0;
K_TYPE count = 0;

*label = atoi(in_svmstr);
// find the number keys and values in the string by counting the colons
for (count=0; svmstr[position_in_str]; position_in_str++) {
	if (svmstr[position_in_str] == ':') {
		count++;
	}
}

char* tok = NULL;

*totalcount = count;

*keys = malloc(sizeof(K_TYPE) * count);
*values = malloc(sizeof(UNORM_V_TYPE) * count);

tok = NULL;
tok = strtok_r(svmstr, " ", &savestr);
tok = strtok_r(NULL, " :", &savestr);

int i = 0;

while (tok) {
	K_TYPE key = (K_TYPE) atoi(tok);
	tok = strtok_r(NULL, " :", &savestr);
	UNORM_V_TYPE value = (UNORM_V_TYPE) atoi(tok);
	tok = strtok_r(NULL, " :", &savestr);
	(*keys)[i] = key;
	(*values)[i] = value;
	i += 1;
}
free(svmstr);
return;
}


// takes in a map, un normalized svm data and a map
// normalizes it, creates a svm string and returns uncompressed data back
void create_svm_str(K_TYPE *in_keys, UNORM_V_TYPE *in_values, K_TYPE in_dimension, int in_label,
	struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char** retlibsvmstr) {

// normalize using the min and max functions
#ifdef DEBUG
log_info("Number of input dimension is %d \n", in_dimension);
#endif

V_TYPE *in_norm_values;
K_TYPE *in_newkeys;
get_mapped_normalized_value_sparse(in_keys, in_values, in_dimension, mapping_old_to_new, oldkey_min_max_list, &in_norm_values, &in_newkeys);

// using the map and the normalized value, write the lib svm
UT_string *libsvmstr;
int retdata = create_lib_svm_sparse(in_newkeys, in_norm_values, in_label, in_dimension, &libsvmstr, true, false);
free(in_norm_values);
free(in_newkeys);

#ifdef DEBUG
if (retdata == 0) {
	log_info("WARNING ! Empty svm string.\n");
}
#endif
// even if empty, it always has label \n
*retlibsvmstr = strdup(utstring_body(libsvmstr));
utstring_free(libsvmstr);
}


void get_min_max_list(struct base_data_struct* orig_data_pointer_list, struct key_min_max** min_max_list, size_t *num_elements)
{
	struct fhash_data *current_ele, *tmp;
	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
// skip deleted elements
		if (current_ele->deleted) {
			continue;
		}
// get the svm data
		struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, current_ele->fhash);

		K_TYPE* keys = keys_values->keys;
		UNORM_V_TYPE* values = keys_values->values;
		K_TYPE num_dimensions = keys_values->num_dimensions;
		*num_elements = *num_elements + 1;

		K_TYPE d = 0;
		for (d = 0; d < num_dimensions; d++) {
			K_TYPE key = keys[d];
			UNORM_V_TYPE value = values[d];
			// add if key not present and update min and max appropriately
			update_min_max(key, value, min_max_list);
		}
	}
}


void get_mapping_dict(struct key_map** mapping_existing_new, struct key_min_max** min_max_list, size_t num_elements)
{
	create_mapping	(min_max_list, mapping_existing_new, num_elements);
	int num_ele_map = HASH_COUNT(*mapping_existing_new);
#ifdef DEBUG
	log_info("The number of elements in map is : %d ",num_ele_map);
#endif

	if (num_ele_map == 0) {
		log_info("Create original mapping");
		create_original_mapping(min_max_list, mapping_existing_new);
	}
#ifdef DEBUG
	struct key_map *current_map,
	*maptemp;

	log_info("Mapping, Old key to New key \n");
	HASH_ITER(hh, *mapping_existing_new, current_map, maptemp)
	{
		log_info("OldKey: %d, New: %d\n", current_map->key, current_map->mappedkey);
	}
#endif
}

struct hashkey_to_empty* create_hash_entry(struct base_data_struct* orig_data_pointer_list, char* hashtoadd) {
	struct hashkey_to_empty *hashentry;
	hashentry = get_hash_entry(orig_data_pointer_list, hashtoadd);
	if (hashentry == NULL) {
#ifdef DEBUG
		log_info("Creating new hash entry for %s\n", hashtoadd);
#endif
		hashentry = malloc(sizeof(struct hashkey_to_empty));
		hashentry->value = 1;
		char* c = hashtoadd;
		int i = 0;
		for (i = 0; i < 64; i++) {
			hashentry->key[i] = *c++;
		}
		//strncpy(hashentry->key, hashtoadd, sizeof(hashentry->key));
		hashentry->key[64] = '\0';
		HASH_ADD_STR(orig_data_pointer_list->common_data_pointer_ref->hashkey_to_empty_list, key, hashentry);
	} else {
#ifdef DEBUG
		log_info("Found existing hash entry for %s\n", hashtoadd);
#endif
	}

	return hashentry;
}

struct hashkey_to_empty* get_hash_entry(struct base_data_struct* orig_data_pointer_list, char* hashtoadd) {
#ifdef DEBUG
	log_info("Get hash entry for %s", hashtoadd);
#endif

	struct hashkey_to_empty *hashentry;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->hashkey_to_empty_list, hashtoadd, hashentry);
	return hashentry;
}


void fill_k_nn_data(struct base_data_struct* orig_data_pointer_list, struct nn_data **k_nn_data, struct key_min_max **in_orig_min_max, K_TYPE **in_keys, V_TYPE **in_norm_values, K_TYPE in_dimension, int k)
{
	*k_nn_data = NULL; // ut list structure which stores data
	double in_norm2 = p2_norm_squared(*in_norm_values, in_dimension);
#ifdef DEBUG
	log_info("The norm2 is: %0.18g \n",in_norm2);
#endif

	struct fhash_data *current_ele, *tmp;
// iterate over all the existing feature hashes
	HASH_ITER(hh, orig_data_pointer_list->fhash_data_list, current_ele, tmp)
	{
		if (current_ele->deleted) {
			continue;
		}
		struct fhash_key_values* keys_values = get_keys_values_ref(orig_data_pointer_list, current_ele->fhash);

		K_TYPE *keys = keys_values->keys;
		UNORM_V_TYPE* values = keys_values->values;
		K_TYPE num_dimensions = keys_values->num_dimensions;
// normalize the data using the given map
		V_TYPE * norm_values = malloc(sizeof(V_TYPE) * num_dimensions);
		do_normalization(keys, values, num_dimensions, norm_values, in_orig_min_max);

#ifdef DEBUG
		K_TYPE d = 0;
		log_info("Data point \n");
		for (d = 0; d < num_dimensions; d++) {
			log_info("Key: %d, value %d normalized %0.18g \n", keys[d], values[d], norm_values[d]);
		}
#endif

		double norm2 = p2_norm_squared(norm_values, num_dimensions);

		double dist = p2_metric_sparse_sparse(*in_keys, *in_norm_values, in_dimension, keys, norm_values, num_dimensions,
				in_norm2, norm2);
#ifdef DEBUG
		log_info("The distance is %0.18g\n", dist);
#endif

// find max data :
		int curr_count;
		struct nn_data *tmpele;
		struct nn_data *newdata;
		newdata = malloc(sizeof(struct nn_data));
		newdata->dist = dist;
		newdata->sha = current_ele->default_sha;
		newdata->fhash = current_ele->fhash;
		LL_COUNT(*k_nn_data, tmpele, curr_count);
		LL_PREPEND(*k_nn_data, newdata);
//		LL_SORT(*k_nn_data, knn_cmp);

		/*if (curr_count < k) {
			// there is room for everyone still
			// add new data at the start and sort it !
			LL_PREPEND(*k_nn_data, newdata);
			LL_SORT(*k_nn_data, knn_cmp);
		}
			else {
			struct nn_data *topele = NULL;
			topele = *k_nn_data;

			UNORM_V_TYPE max_current_dist = topele->dist;
			if (dist < max_current_dist) {
				// need to add this distance !
				LL_PREPEND(*k_nn_data, newdata);
				LL_DELETE(*k_nn_data, topele);
				LL_SORT(*k_nn_data, knn_cmp);
			}
		}*/
		free(norm_values); // free norm values as they need not be stored
	}
}

void k_nn_keep_topk(struct nn_data **k_nn_data, int k)
{
	struct nn_data *elt, *tmp;
	int nncount = 0;
	LL_FOREACH_SAFE(*k_nn_data,elt,tmp)
	{
		if (nncount < k)
		{	nncount = nncount + 1;
			continue;
		}
		else
		{
			LL_DELETE(*k_nn_data, elt);
			free(elt);
		}
	}
}

void display_k_nn_data(struct nn_data **k_nn_data)
{
		log_info("All distances are : \n");
		struct nn_data *tmpele;
		LL_FOREACH(*k_nn_data,tmpele)
		{
			log_info("Fhash: %s Distance: %0.18g\n",tmpele->fhash, tmpele->dist);
		}
}
