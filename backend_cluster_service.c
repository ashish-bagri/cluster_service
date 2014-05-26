#include "backend_service.h"
#include "../swig_callbacks/swig_callbacks.h"

//#define DEBUG
/*
 * Methods that are exclusively used by backend_cluster_service
 *
*/

void initialize_backend_cluster_service(struct base_data_struct** orig_data_pointer_list)
{
	*orig_data_pointer_list = malloc(sizeof(struct base_data_struct));
	(*orig_data_pointer_list)->fhash_data_list = NULL;

	(*orig_data_pointer_list)->anomaly_data_list = NULL;
	(*orig_data_pointer_list)->key_min_max_list = NULL;

	(*orig_data_pointer_list)->common_data_pointer_ref = malloc(sizeof(struct common_data_pointer));

	(*orig_data_pointer_list)->enum_hashes_list = NULL;
	UT_icd enum_hash_pair = {sizeof(struct enum_hashes), NULL, NULL, NULL};
	utarray_new((*orig_data_pointer_list)->enum_hashes_list, &enum_hash_pair);

	(*orig_data_pointer_list)->common_data_pointer_ref->hashkey_to_empty_list = NULL;
	(*orig_data_pointer_list)->common_data_pointer_ref->md5_fhash_list = NULL;
	(*orig_data_pointer_list)->common_data_pointer_ref->sha_fhash_list = NULL;
	(*orig_data_pointer_list)->common_data_pointer_ref->fhash_key_values_list = NULL;


}
// one add to rule them all !
void add_data_fhash_data_list(struct base_data_struct* orig_data_pointer_list, char* fhash, char* md5, char* sha, char* base_sha, double timestamp, int priority,
		int label, char* svmdatastr, int is_orig, bool *is_retraining, int* curr_count, bool* new_data_added) {
	// add it to the sha-fhash list
	// add it to md5-fhash list

	bool change_happened = false;

	if (is_orig == true)
	{	// add_enum_sha_md5 should be called before set_sha_fhash and set_md5_fhash
		add_enum_sha_md5(orig_data_pointer_list, sha, md5);
	}

	set_sha_fhash(orig_data_pointer_list, sha, fhash);
	set_md5_fhash(orig_data_pointer_list, md5, fhash);

	int existing_label;
	int new_label;

	struct fhash_data *s;
	HASH_FIND_STR(orig_data_pointer_list->fhash_data_list, fhash, s);
	*new_data_added = false;

	if (s == NULL) {

#ifdef DEBUG
		log_info("Adding feature hash %s . is_orig ?: %d", fhash, is_orig);
#endif

		add_fhash_data(orig_data_pointer_list, fhash, svmdatastr, is_orig);
		change_happened = true;
		add_additional_data_fhash_data_list(orig_data_pointer_list, fhash, md5, sha, base_sha, timestamp, priority, label);
		*new_data_added = true;
	} else {
		existing_label = s->label;
		add_additional_data_fhash_data_list(orig_data_pointer_list, fhash, md5, sha, base_sha, timestamp, priority, label);
		new_label = s->label;
		s->is_orig = is_orig;
#ifdef DEBUG
		log_info("The old label was %d. The new label is %d", existing_label, new_label);
#endif
		if (new_label != existing_label) {
			change_happened = true;
		}
	}

	*is_retraining = change_happened;
	*curr_count = HASH_COUNT(orig_data_pointer_list->fhash_data_list);
}

void add_additional_data_fhash_data_list(struct base_data_struct* orig_data_pointer_list, char* fhash, char* md5, char* sha, char* base_sha, double timestamp,
		int priority, int label) {

	struct fhash_data *s;
	HASH_FIND_STR(orig_data_pointer_list->fhash_data_list, fhash, s);
	if (s == NULL) {
		log_error("ERROR: expected a feature hash %s to be in the database already", fhash);
		return;
	} else {
		// need to add a hash associated with the feature hash.
		// step1. find if the hash already exists. (key is sha)
		struct hashdata *newdata;
		newdata = malloc(sizeof(struct hashdata));

		struct hashkey_to_empty *hashentry_sha = create_hash_entry(orig_data_pointer_list, sha);
		struct hashkey_to_empty *hashentry_md5 = create_hash_entry(orig_data_pointer_list, md5);
		struct hashkey_to_empty *hashentry_basesha = create_hash_entry(orig_data_pointer_list, base_sha);

		newdata->sha = hashentry_sha->key;
		newdata->md5 = hashentry_md5->key;
		newdata->base_sha = hashentry_basesha->key;

		newdata->timestamp = timestamp;
		newdata->priority = priority;
		newdata->label = label;


		struct hashdata* current_default;
		HASH_FIND_STR(s->duplicate_list_head, s->default_sha, current_default);

		struct hashdata* existingdata;
		HASH_FIND_STR(s->duplicate_list_head, sha, existingdata);

		if (existingdata == NULL) {
			// there is no data for this hash..add it, compare to default and set a right default
			HASH_ADD_KEYPTR(hh, s->duplicate_list_head, newdata->sha, strlen(newdata->sha), newdata);
			if (change_default(current_default, newdata)) {
				s->default_sha = newdata->sha;
				s->label = newdata->label;
			}
		} else {
#ifdef DEBUG
			log_info("Updating sha %s", sha);
#endif
			// there is a data with the same hash in the list.
			// update the existing hash with this new information and then compare with the default to see if the default must change ?
			bool modified = false;
			bool labelchange = false;
			if (newdata->priority > existingdata->priority) {
				modified = true;
			}
			if (newdata->timestamp > existingdata->timestamp) {
				modified = true;
			}
			if (newdata->label != existingdata->label) {
				if(existingdata->label == LABEL_MW)
				{
					modified = true;
					labelchange = true;
				}
			}
			if (modified) {
				// delete existingdata, add newdata !
				HASH_DEL(s->duplicate_list_head, existingdata);
				free(existingdata);
				HASH_ADD_KEYPTR(hh, s->duplicate_list_head, newdata->sha, strlen(newdata->sha), newdata);
				// find new default !
				struct hashdata* newdefault = find_new_default(s->duplicate_list_head);
				s->default_sha = newdefault->sha;
				s->label = newdefault->label;
			} else {
				free(newdata);
			}
		}

		struct hashdata* debughead = s->duplicate_list_head;
		if (debughead == NULL) {
#ifdef DEBUG
			log_info("vOILA ! ITS NULL");
#endif
		}
	}
}

// lazy delete. sets deleted to true but not actually deletes it
void delete_fhash_data(struct base_data_struct* orig_data_pointer_list, char* fhash) {
	struct fhash_data *s;
	HASH_FIND_STR(orig_data_pointer_list->fhash_data_list, fhash, s);
	if (s != NULL) {
		s->deleted = true;
	}
}

void add_enum_sha_md5(struct base_data_struct* orig_data_pointer_list, char* sha, char* md5)
{
	struct hashkey_to_empty *hashentry_sha = create_hash_entry(orig_data_pointer_list, sha);
	struct hashkey_to_empty *hashentry_md5 = create_hash_entry(orig_data_pointer_list, md5);
	bool newhash = false;
	struct sha_fhash *existingsha;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, sha, existingsha);
	if (existingsha == NULL) {
		// new entry
		newhash = true;
	}

	struct md5_fhash *existingmd5;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, md5, existingmd5);
	if (existingmd5 == NULL) {
		newhash = true;
	}
	if (newhash == true)
	{
		// add the data
		int len = utarray_len(orig_data_pointer_list->enum_hashes_list);
#ifdef DEBUG
		log_info("Adding sha: %s md5: %s as enum entry: %d", sha, md5, len);
#endif
		struct enum_hashes newenum;

		//struct enum_hashes *newenum = malloc(sizeof(struct enum_hashes));
		newenum.sha = hashentry_sha->key;
		newenum.md5 = hashentry_md5->key;
		newenum.enum_id = len;
		//newenum->
		utarray_push_back(orig_data_pointer_list->enum_hashes_list, &newenum);
	}
}

void set_sha_fhash(struct base_data_struct* orig_data_pointer_list, char* sha, char* fhash) {

#ifdef DEBUG
	log_info("Adding sha:%s mapping to fhash:%s ", sha, fhash);
#endif

	struct hashkey_to_empty *hashentry_fhash = create_hash_entry(orig_data_pointer_list, fhash);
	struct hashkey_to_empty *hashentry_sha = create_hash_entry(orig_data_pointer_list, sha);

	struct sha_fhash *existing;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, sha, existing);
	if (existing == NULL) {
		// sha is new. just add it.
		struct sha_fhash *s;
		s = malloc(sizeof(struct sha_fhash));
		s->sha = hashentry_sha->key;
		s->fhash = hashentry_fhash->key;
		HASH_ADD_KEYPTR(hh, orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, s->sha, strlen(s->sha), s);

	} else {
		if (strcmp(existing->fhash, fhash) != 0) {
			// problem case
			log_info("Error! differing flag case");
			log_info("The new feature hash %s does not match the existing feature hash %s for sha %s ", fhash,
					existing->fhash, sha);
		}
	}
}
void delete_sha_fhash(struct base_data_struct* orig_data_pointer_list, char* sha) {
	struct sha_fhash *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, sha, s);
	if (s != NULL) {
		HASH_DEL(orig_data_pointer_list->common_data_pointer_ref->sha_fhash_list, s);
		free(s);
	} else {
		log_info("Cannot remove %s as it is not present in list", sha);
	}
}

void set_md5_fhash(struct base_data_struct* orig_data_pointer_list, char* md5, char* fhash) {

#ifdef DEBUG
	log_info("Adding md5:%s mapping to fhash:%s ", md5, fhash);
#endif DEBUG

	struct hashkey_to_empty *hashentry_fhash = create_hash_entry(orig_data_pointer_list, fhash);
	struct hashkey_to_empty *hashentry_md5 = create_hash_entry(orig_data_pointer_list, md5);

	struct md5_fhash *existing;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, md5, existing);
	if (existing == NULL) {
		// md5 is new. just add it.
		struct md5_fhash *s;
		s = malloc(sizeof(struct md5_fhash));
		s->md5 = hashentry_md5->key;
		s->fhash = hashentry_fhash->key;
		HASH_ADD_KEYPTR(hh, orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, s->md5, strlen(s->md5), s);
	} else {
		if (strcmp(existing->fhash, fhash) != 0) {
			// problem case
			log_info("Error! differing flag case");
			log_info("The new feature hash %s does not match the existing feature hash %s for md5 %s ", fhash,
					existing->fhash, md5);
		}
	}
}

void delete_md5_fhash(struct base_data_struct* orig_data_pointer_list, char* md5) {
	struct md5_fhash *s;
	HASH_FIND_STR(orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, md5, s);
	if (s != NULL) {
		HASH_DEL(orig_data_pointer_list->common_data_pointer_ref->md5_fhash_list, s);
		free(s);
	} else {
		log_error("Cannot remove %s as it is not present in list", md5);
	}
}

void add_fhash_data(struct base_data_struct* orig_data_pointer_list, char* fhash, char* svmdatastr, int is_orig) {

	struct hashkey_to_empty *hashentry = create_hash_entry(orig_data_pointer_list, fhash);

	struct fhash_data *s;
	s = malloc(sizeof(struct fhash_data));
	s->fhash = hashentry->key;
	s->deleted = false;
	s->is_orig = is_orig;
	s->duplicate_list_head = NULL;
	s->default_sha = NULL;
	s->mrmr_anomaly_key = -1; // only used in clone data
	s->mw_prob = 0.0;

	K_TYPE num_dim = 0;
	K_TYPE* keys; // the keys of the svm data -
	UNORM_V_TYPE* values; // the values of the svm data -
	int label;
	libsvmstr_key_values(svmdatastr, &label, &keys, &values, &num_dim);

	struct fhash_key_values *key_value_entry = malloc(sizeof(struct fhash_key_values));
	key_value_entry->fhash = hashentry->key;
	key_value_entry->keys = keys;
	key_value_entry->values = values;
	//key_value_entry->label = label;
	key_value_entry->num_dimensions = num_dim;
	// add key-value entry in the correct hash-table
	HASH_ADD_KEYPTR(hh, orig_data_pointer_list->common_data_pointer_ref->fhash_key_values_list, key_value_entry->fhash, strlen(key_value_entry->fhash), key_value_entry);

	s->label = label;
	HASH_ADD_KEYPTR(hh, orig_data_pointer_list->fhash_data_list, s->fhash, strlen(s->fhash), s);

#ifdef DEBUG
	log_info("Added feature hash %s in the list", s->fhash);
#endif
}

struct hashdata* find_new_default(struct hashdata* head) {

	struct hashdata* curr_default = head;
	struct hashdata *current_ele, *tmp;
	HASH_ITER(hh, head, current_ele, tmp)
	{
		if (change_default(curr_default, current_ele)) {
			curr_default = current_ele;
		}
	}
// the default element is curr_default
	//return curr_default->sha;
	return curr_default;
}

bool change_default(struct hashdata* current_default, struct hashdata* newdata) {

	if (current_default == NULL) {
		return true;
	}

	if (newdata->priority < current_default->priority) {
		return false;
	}
	if (newdata->priority > current_default->priority) {
		return true;
	}
// comes here if the priorities are the same.

// if the new data has a older timestamp, it looses
	if (newdata->timestamp < current_default->timestamp) {
		return false;
	}
	if (newdata->timestamp > current_default->timestamp) {
		return true;
	}
// comes here if timestamp is the same ! take the hash which has a label 'clean'
	if (current_default->label == LABEL_CLEAN) {
// i need not even check the new one. If the new one is clean, the existing default stays. if new one is MW, still existing default stays
		return false;
	}

// comes here if the current default is MW.
	if (newdata->label == LABEL_CLEAN) {
		return true;
	} else {
		return false;
	}
}

// the complete implementation in swig
void get_hashes_from_enumid(struct base_data_struct* orig_data_pointer_list, int *enumid)
{
	return;
}

