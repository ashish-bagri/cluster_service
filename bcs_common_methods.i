%module backend_cluster_service

%include<cstring.i>
%include "../swig_callbacks/callback.i"

%{
#include "backend_service.h"
%}

// input get_libsvm_keys_values_from_fhash

%typemap(in,numinputs=0)(int *dim, K_TYPE **keys, UNORM_V_TYPE **values)
				(int dim, K_TYPE *keys, UNORM_V_TYPE *values){					
	$1 = &dim;
	$2 = &keys;
	$3 = &values;
}

// output get_libsvm_keys_values_from_fhash

%typemap(argout)(int* dim, K_TYPE **keys, UNORM_V_TYPE **values){
	
	PyObject *tempp;
	
	int dim2 = *$1;
		
	if (dim2 == 0)
	{
		$result = Py_None;
	}
	else
	{
		$result = PyList_New(dim2);
		int i=0;
		for(i=0; i<dim2; i++)
		{
			tempp = PyTuple_New(2);
			PyTuple_SetItem(tempp, 0, PyInt_FromLong((*$2)[i]));
			PyTuple_SetItem(tempp, 1, PyInt_FromLong((*$3)[i]));		
			PyList_SetItem($result, i, tempp);	
		}
		free(*($2));
		free(*($3));
	}
}
// input get_normalized_str, get_predict_svm_str_from_sha, get_predict_svm_str_from_fhash
// converting map to structure that is needed
%typemap(in,numinputs=1)(struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new)
				(struct key_min_max *oldkey_min_max_list, struct key_map *mapping_old_to_new){
	
	mapping_old_to_new = NULL;
	oldkey_min_max_list = NULL;
		
		
	assert(PyDict_Check($input));
	Py_ssize_t pos = 0;
	PyObject *key, *value;
	
	while (PyDict_Next($input, &pos, &key, &value)) {
	    K_TYPE oldkey = PyInt_AS_LONG(key);	    	    
	    assert(PyTuple_Check(value));
	    Py_ssize_t newkeypos = 0;
	    Py_ssize_t minpos = 1;
	    Py_ssize_t maxpos = 2;	    
	    K_TYPE newkey = PyInt_AsLong(PyTuple_GetItem(value,newkeypos));
	    double min = PyFloat_AsDouble(PyTuple_GetItem(value,minpos));
	    double max = PyFloat_AsDouble(PyTuple_GetItem(value,maxpos));
	    
	    struct key_map *newdata = malloc(sizeof(struct key_map));
	    newdata->key = oldkey;
	    newdata->mappedkey = newkey;
	    HASH_ADD(hh, mapping_old_to_new, key, (sizeof(K_TYPE)), newdata);
		
		struct key_min_max *keyminmax = malloc(sizeof(struct key_min_max));
		keyminmax->key = oldkey;
		keyminmax->minvalue = min;
		keyminmax->maxvalue = max;
		HASH_ADD(hh, oldkey_min_max_list, key, (sizeof(K_TYPE)), keyminmax);
     }
	
	$1 = &oldkey_min_max_list;
	$2 = &mapping_old_to_new;	
}


// input get_data_from_fhash
%typemap(in,numinputs=0)(struct fhash_data **retdata)(struct fhash_data *retdata){
	$1 = &retdata;
}

// output get_data_from_fhash
%typemap(argout)(struct fhash_data **retdata){
	
	if(*$1 == NULL)
	{
		$result = Py_None;
		return;
	}
	
	PyObject *tempp, *dups;
	$result = PyDict_New();
	
	PyDict_SetItem($result,PyString_FromString("is_orig"),PyInt_FromLong((*$1)->is_orig));
	PyDict_SetItem($result,PyString_FromString("deleted"),PyInt_FromLong((*$1)->deleted));
	PyDict_SetItem($result,PyString_FromString("defaultsha"),PyString_FromString((*$1)->default_sha));
	PyDict_SetItem($result,PyString_FromString("mrmr_anomaly_key"),PyInt_FromLong((*$1)->mrmr_anomaly_key));
	PyDict_SetItem($result,PyString_FromString("mw_prob"),PyFloat_FromDouble((*$1)->mw_prob));
		
	struct hashdata* duplicate_list_head = (*$1)->duplicate_list_head;
	dups = PyDict_New();
	struct hashdata *current_ele, *tmp;
	HASH_ITER(hh, duplicate_list_head, current_ele, tmp)
	{
		tempp = PyDict_New();
		PyDict_SetItem(tempp,PyString_FromString("sha"),PyString_FromString(current_ele->sha));
		PyDict_SetItem(tempp,PyString_FromString("md5"),PyString_FromString(current_ele->md5));
		PyDict_SetItem(tempp,PyString_FromString("basesha"),PyString_FromString(current_ele->base_sha));
		PyDict_SetItem(tempp,PyString_FromString("timestamp"),PyInt_FromLong(current_ele->timestamp));
		PyDict_SetItem(tempp,PyString_FromString("priority"),PyInt_FromLong(current_ele->priority));
		PyDict_SetItem(tempp,PyString_FromString("label"),PyInt_FromLong(current_ele->label));
		PyDict_SetItem(dups,PyString_FromString(current_ele->sha),tempp);
	}
	PyDict_SetItem($result,PyString_FromString("duplicates"),dups);
	
	//cleanup_fhash_data($1);
}

// input k_nn
%typemap(in,numinputs=1) (struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data)
						(struct key_min_max *in_orig_min_max, struct nn_data *k_nn_data){

	//convert the input data into the struct key_min_max that is expected by the c function
	// input data is a dict {key (oldkey} -> (mapped, min, max)} will ignore mapped
	assert(PyDict_Check($input));
	
	Py_ssize_t pos = 0;
	PyObject *key, *value;
	
	in_orig_min_max = NULL;
	
	
	while (PyDict_Next($input, &pos, &key, &value)) {
		K_TYPE oldkey = PyInt_AS_LONG(key);
		assert(PyTuple_Check(value));
	    Py_ssize_t newkeypos = 0;
	    Py_ssize_t minpos = 1;
	    Py_ssize_t maxpos = 2;    
	    double min = PyFloat_AsDouble(PyTuple_GetItem(value,minpos));
	    double max = PyFloat_AsDouble(PyTuple_GetItem(value,maxpos));
	    
	    struct key_min_max *keyminmax = malloc(sizeof(struct key_min_max));
		keyminmax->key = oldkey;
		keyminmax->minvalue = min;
		keyminmax->maxvalue = max;
		HASH_ADD(hh, in_orig_min_max, key, (sizeof(K_TYPE)), keyminmax);
    }
	
	$1 = &in_orig_min_max;
	$2 = &k_nn_data;

}

// output of get_k_nn
%typemap(argout) (struct nn_data **k_nn_data) {
	// at the output stage, need to convert the k_nn_data into a list of tupple	
	PyObject *tempp;
	
	// count the number of elements 
	int retcount;
	struct nn_data *tmpele;		
	LL_COUNT((*$1), tmpele, retcount);
	$result = PyList_New(retcount);
	// fill in the list 
	
	struct nn_data *elt;
	int i=0;
	LL_FOREACH(*$1,elt)
	{
		tempp = PyTuple_New(2);
		PyTuple_SetItem(tempp, 0, PyString_FromString(elt->fhash));
		PyTuple_SetItem(tempp, 1, PyFloat_FromDouble(elt->dist));
		PyList_SetItem($result, i, tempp);
		i ++;
	}
	
	cleanup_nn_data($1);	
}
		
// input get_normalized_svm_data_mrmr
%typemap(in,numinputs=0) (struct key_map** old_new_keymap, struct key_min_max **output_max_min)  (struct key_map* old_new_keymap, struct key_min_max *output_max_min) {
    $1 = &old_new_keymap;
    $2 = &output_max_min;
}

// outpt get_normalized_svm_data_mrmr
%typemap(argout) (char **output, size_t *OUTPUT, size_t *OUTPUT, struct key_map** old_new_keymap, struct key_min_max **output_max_min) {
	
	$result = PyTuple_New(4);
	
	PyObject *tempp;
	// output 4
	PyObject *pymapdict;
	pymapdict = PyDict_New();
	
	// output 1
	if (*($1) == NULL)
	{
		PyTuple_SetItem($result, 0, Py_None);
	}else
	{
		PyTuple_SetItem($result, 0, PyString_FromString(*($1)));
		free(*($1));
	}	
	
	// output 2 3
	PyTuple_SetItem($result, 1, PyInt_FromLong(*($2)));
	PyTuple_SetItem($result, 2, PyInt_FromLong(*($3)));

	struct key_map *current_map, *maptemp;		
	int i = 0;		
	HASH_ITER(hh, (*$4), current_map, maptemp){
		tempp = PyTuple_New(3);
		PyTuple_SetItem(tempp, 0, PyInt_FromLong(current_map->mappedkey));		
		struct key_min_max *sminmax;
		HASH_FIND(hh, (*$5), &(current_map->key), sizeof(K_TYPE), sminmax);
	
	if (sminmax != NULL) {

		PyTuple_SetItem(tempp, 1, PyInt_FromLong(sminmax->minvalue));
		PyTuple_SetItem(tempp, 2, PyInt_FromLong(sminmax->maxvalue));
	}
	else {
		PyTuple_SetItem(tempp, 1, Py_None);
		PyTuple_SetItem(tempp, 2, Py_None);
	}
		PyDict_SetItem(pymapdict,PyInt_FromLong(current_map->key),tempp);
		i++;
	}
	
	PyTuple_SetItem($result, 3, pymapdict);
	cleanup_key_map($4);
	cleanup_key_min_max($5);
}

// outpt get_predict_svm_str_from_sha, get_predict_svm_str_from_fhash, get_normalized_str
// free structures that were created
%typemap(argout) (struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new) {
	cleanup_key_min_max($1);
	cleanup_key_map($2);
}


// outpt do_minimize_dimension_get_map
%typemap(argout) (struct key_map** old_new_keymap, struct key_min_max **output_max_min, K_TYPE *OUTPUT, K_TYPE *OUTPUT) {
	
	PyObject *tempp;
	
	$result = PyTuple_New(3);
	
	PyObject *pymapdict;
	pymapdict = PyDict_New();

	struct key_map *current_map, *maptemp;		
	int i = 0;		
	HASH_ITER(hh, (*$1), current_map, maptemp){
		tempp = PyTuple_New(3);
		PyTuple_SetItem(tempp, 0, PyInt_FromLong(current_map->mappedkey));		
		struct key_min_max *sminmax;
		HASH_FIND(hh, (*$2), &(current_map->key), sizeof(K_TYPE), sminmax);
	
	if (sminmax != NULL) {

		PyTuple_SetItem(tempp, 1, PyInt_FromLong(sminmax->minvalue));
		PyTuple_SetItem(tempp, 2, PyInt_FromLong(sminmax->maxvalue));
	}
	else {
		PyTuple_SetItem(tempp, 1, Py_None);
		PyTuple_SetItem(tempp, 2, Py_None);
	}
		PyDict_SetItem(pymapdict,PyInt_FromLong(current_map->key),tempp);
		i++;
	}
	
	PyTuple_SetItem($result, 0, pymapdict);
	PyTuple_SetItem($result, 1, PyInt_FromLong(*($3)));
	PyTuple_SetItem($result, 2, PyInt_FromLong(*($4)));
	cleanup_key_map($1);
	cleanup_key_min_max($2);
}

// input: get_all_fhash_list
%typemap(in,numinputs=0) (UT_array **ret_fhash_data)
		(UT_array *ret_fhash_data)
{		
	$1 = &ret_fhash_data;
}

// output: get_all_fhash_list
%typemap(argout) (UT_array **ret_fhash_data)
{	 
	int a = utarray_len(*$1);
	if (a == 0)
	{
		$result = Py_None;
	}else
	{
		char **p;
		p=NULL;
		PyObject *retfhashlist;
		$result = PyList_New(a);
		int c = 0;	
  		while ( (p=(char**)utarray_next(*$1, p))) {	
			PyList_SetItem($result, c, PyString_FromString(*p));
			c = c + 1;
	  	}
		utarray_free(*$1);
	}
}

%include<cstring.i>
%cstring_output_allocate(char **output, free(*$1));

typedef unsigned short int K_TYPE;
typedef double V_TYPE;
typedef unsigned int UNORM_V_TYPE;

void getlibsvmdata_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, char **output);

void get_normalized_svm_data_mrmr(struct base_data_struct* orig_data_pointer_list, char **output, size_t *OUTPUT, size_t *OUTPUT, struct key_map **old_new_keymap,
struct key_min_max **output_max_min);

void do_minimize_dimension_get_map(struct base_data_struct* orig_data_pointer_list, struct key_map **old_new_keymap,struct key_min_max **output_max_min, K_TYPE *OUTPUT, K_TYPE *OUTPUT);

void get_orig_map(struct base_data_struct* orig_data_pointer_list, struct key_map **old_new_keymap,struct key_min_max **output_max_min, K_TYPE *OUTPUT, K_TYPE *OUTPUT);

void get_k_nn_fhashes(struct base_data_struct* orig_data_pointer_list, char* input_fhash, int k, struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data);

void get_k_nn(struct base_data_struct* orig_data_pointer_list, char* in_svmstr, int k, struct key_min_max **in_orig_min_max, struct nn_data **k_nn_data);

void get_fhash_from_sha(struct base_data_struct* orig_data_pointer_list, char* sha, char **output);

void get_fhash_from_md5(struct base_data_struct* orig_data_pointer_list, char* md5, char **output);

void get_data_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, struct fhash_data **retdata);

void get_normalized_str(struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char* in_svmstr,
	char** output);

void get_predict_svm_str_from_sha(struct base_data_struct* orig_data_pointer_list, struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char* sha, char **output);

void get_predict_svm_str_from_fhash(struct base_data_struct* orig_data_pointer_list, struct key_min_max **oldkey_min_max_list, struct key_map **mapping_old_to_new, char* fhash, char **output, int* OUTPUT);
		
void get_libsvm_keys_values_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash, int* dim, K_TYPE **keys, UNORM_V_TYPE **values);
	
int get_label_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash);

int is_orig_from_fhash(struct base_data_struct* orig_data_pointer_list, char* fhash);

void get_orig_counts(struct base_data_struct* orig_data_pointer_list, int *OUTPUT, int *OUTPUT);

void get_compressed_string(char *in_str, char **output);

void get_raw_data(struct base_data_struct* orig_data_pointer_list, char** output, size_t *OUTPUT, size_t *OUTPUT);

void free_all_data(struct base_data_struct* orig_data_pointer_list);

void get_current_fhash_data_list_count(struct base_data_struct* orig_data_pointer_list, int* OUTPUT);

void get_all_fhash_list(struct base_data_struct* orig_data_pointer_list, UT_array **ret_fhash_data);
