%module backend_clone_service

%include<cstring.i>
%include "../swig_callbacks/callback.i"
%include "bcs_common_methods.i"
%{
void swig_create_libsvm_str(int label, char* fhash_libsvmstr, UT_string **libsvmstr);
void swig_add_fhash(char* fhash, UT_array **ea_fhash_list);

void swig_create_libsvm_str(int label, char* fhash_libsvmstr, UT_string **libsvmstr)
{
	UT_string *s;
	utstring_new(s);
	char* tmpout[100];
	sprintf(tmpout, "%i", label);
	utstring_printf(s, tmpout);
	utstring_printf(s, fhash_libsvmstr);
	utstring_concat(*libsvmstr, s);
	utstring_free(s);
}

void initialize_array(UT_string **libsvmstr, int num)
{
	//printf("Trying to reserve utstring\n");
	size_t utlgs = utstring_len(*libsvmstr);
	//printf("The utstring len is: %u \n", utlgs);
	size_t size_init = utlgs * (num + (0.1 * (num)));
	//printf("Size of reservation of is: %u \n", size_init);
	utstring_reserve(*libsvmstr, size_init);
	//printf("Completed utstring_reserve \n");
}

void swig_add_fhash(char* fhash, UT_array **ea_fhash_list)
{
	utarray_push_back((*ea_fhash_list), &fhash);
}

%}

%typemap(in,numinputs=0) (struct base_data_struct** training_clone_list)
					(struct base_data_struct* training_clone_list) {
    $1 = &training_clone_list;
}
// argout clone_training_data
%typemap(argout) struct base_data_struct **training_clone_list {
    PyObject *o, *o2, *o3;
    o = SWIG_NewPointerObj(SWIG_as_voidptr(*$1), $*1_descriptor, SWIG_POINTER_NOSHADOW);
    if ((!$result) || ($result == Py_None)) {
        $result = o;
    } else {
        if (!PyTuple_Check($result)) {
            PyObject *o2 = $result;
            $result = PyTuple_New(1);
            PyTuple_SetItem($result,0,o2);
        }
        o3 = PyTuple_New(1);
        PyTuple_SetItem(o3,0,o);
        o2 = $result;
        $result = PySequence_Concat(o2,o3);
        Py_DECREF(o2);
        Py_DECREF(o3);
    }
}
// add_mrmr_ana
// takes in the mrmr anomaly info and builds the backend struct for that anomaly. 
// It does not update the clone data fhashes with the key. that is done in update_clone_mrmr_anomaly
%typemap(in,numinputs=1)(struct anomaly_data **anomaly_data_list)
						(struct anomaly_data *anomaly_data_list)
{
	anomaly_data_list = NULL;
	
// for mw and gs lists
	
	assert(PyTuple_Check($input));
	PyObject *mw, *gs, *mixed;
	
	mw = PyTuple_GetItem($input,0);
	gs = PyTuple_GetItem($input,1);
	mixed = PyTuple_GetItem($input, 2);
	
	// deal with mw::
	{	
		int num_mw_ana = PyDict_Size(mw);
		//log_info("Num of mw ana keys: %d", num_mw_ana);
		Py_ssize_t pos = 0;
		PyObject *key, *value;
		while (PyDict_Next(mw, &pos, &key, &value)) {
			int hashlist_id = PyInt_AS_LONG(key);
			assert(PyList_Check(value));
			int len_hashlist = (int)PyList_Size(value);
			
			// create a new anomaly object
			struct anomaly_data* newhashlist = malloc(sizeof(struct anomaly_data));
			
			newhashlist->hashlist_id = hashlist_id;
			newhashlist->anomaly_type = 1; // 1 for malware 
			newhashlist->anomaly_type_method = 1; // 1 for mrmr
			newhashlist->orig_fhash_list_mw = NULL;
			newhashlist->orig_fhash_list_gs = NULL;
			utarray_new(newhashlist->orig_fhash_list_mw, &ut_str_icd);
		//	log_info("Hash id: %d Len MW hashlist %d", hashlist_id, len_hashlist);
			int x = 0;
			for (x = 0; x < len_hashlist;x++){
				char* s = PyString_AsString(PyList_GetItem(value,x));
				utarray_push_back(newhashlist->orig_fhash_list_mw, &s);
			}
			HASH_ADD_INT(anomaly_data_list, hashlist_id, newhashlist);
		}
	}
	// deal with gs
	{		
		int num_gs_ana = PyDict_Size(gs);
		//log_info("Num of gs ana keys: %d", num_gs_ana);
		Py_ssize_t pos = 0;
		PyObject *key, *value;
		while (PyDict_Next(gs, &pos, &key, &value)) {
			int hashlist_id = PyInt_AS_LONG(key);
			
			assert(PyList_Check(value));
			int len_hashlist = (int)PyList_Size(value);
			
			// create a new anomaly object
			struct anomaly_data* newhashlist = malloc(sizeof(struct anomaly_data));
			
			newhashlist->hashlist_id = hashlist_id;
			newhashlist->anomaly_type = 2; // 2 for gs 
			newhashlist->anomaly_type_method = 1; // 1 for mrmr
			newhashlist->orig_fhash_list_mw = NULL;
			newhashlist->orig_fhash_list_gs = NULL;
			
			utarray_new(newhashlist->orig_fhash_list_gs, &ut_str_icd);
			//log_info("Hash id: %d Len GS hashlist %d", hashlist_id, len_hashlist);
			int x = 0;
			for (x = 0; x < len_hashlist;x++){
				char* s = PyString_AsString(PyList_GetItem(value,x));
				utarray_push_back(newhashlist->orig_fhash_list_gs, &s);
			}
			HASH_ADD_INT(anomaly_data_list, hashlist_id, newhashlist);
		}
	}
		// deal with mixed
	{
		int num_mixed_ana = PyDict_Size(mixed);
		//log_info("Num of mixed ana keys: %d", num_mixed_ana);
		Py_ssize_t pos = 0;
		PyObject *key, *value;
		while (PyDict_Next(mixed, &pos, &key, &value)) {
			
			int hashlist_id = PyInt_AS_LONG(key);
			
			
			// create a new anomaly object
			struct anomaly_data* newhashlist = malloc(sizeof(struct anomaly_data));
			
			newhashlist->hashlist_id = hashlist_id;
			newhashlist->anomaly_type = 3; // 3 for mixed
			newhashlist->anomaly_type_method = 1; // 1 for mrmr
			newhashlist->orig_fhash_list_mw = NULL;
			newhashlist->orig_fhash_list_gs = NULL;
			 
			utarray_new(newhashlist->orig_fhash_list_mw, &ut_str_icd);
			utarray_new(newhashlist->orig_fhash_list_gs, &ut_str_icd);
			
			// value here is a tuple of list
			assert(PyTuple_Check(value));
			
			int len_tup = (int)PyTuple_Size(value);
			
			PyObject *mw_list, *gs_list;
			mw_list = PyTuple_GetItem(value,0);
			assert(PyList_Check(mw_list));
			int num_mw = (int)PyList_Size(mw_list);
			
			gs_list = PyTuple_GetItem(value,1);
			assert(PyList_Check(gs_list));
			int num_gs = (int)PyList_Size(gs_list);
			
			// loop through mw					
			//log_info("Hash id: %d Len MW hashlist %d", hashlist_id, num_mw);
			int x = 0;
			for (x = 0; x < num_mw;x++){
				char* s = PyString_AsString(PyList_GetItem(mw_list,x));
				utarray_push_back(newhashlist->orig_fhash_list_mw, &s);
			}
			
			// loop through gs
			//log_info("Hash id: %d Len GS hashlist %d", hashlist_id, num_gs);
			x = 0;
			for (x = 0; x < num_gs;x++){
				char* s = PyString_AsString(PyList_GetItem(gs_list,x));
				utarray_push_back(newhashlist->orig_fhash_list_gs, &s);
			}
			
			HASH_ADD_INT(anomaly_data_list, hashlist_id, newhashlist);
		}
	}
		
	$1 = &anomaly_data_list;
}

// update_clone_mrmr_anomaly
// updates the clone object. each fhash is associated with a anomaly key of the mrmr anomaly.
// adds that key into the clone object 
// the anomaly_data was created in add_mrmr_anomaly_info
%typemap(in,numinputs=1)(struct strkey_to_empty **fhash_hashey)
						(struct strkey_to_empty *fhash_hashey)
{
	fhash_hashey = NULL;
	assert(PyDict_Check($input));
	Py_ssize_t pos = 0;
	PyObject *key, *value;
	
	while (PyDict_Next($input, &pos, &key, &value)) {
		char* s = PyString_AsString(key);
		if (value == Py_None)
			continue;
		
		int hashlist_id = PyInt_AS_LONG(value);
		//printf("Hash : %s mapped: %d \n", s, hashlist_id);
		
		struct strkey_to_empty *newdata = malloc(sizeof(struct strkey_to_empty));
		newdata->key = s;
		newdata->value = hashlist_id;
		HASH_ADD_KEYPTR(hh, fhash_hashey, newdata->key, strlen(s), newdata);
	}
	$1 = &fhash_hashey;
}

%typemap(argout)(struct strkey_to_empty **fhash_hashey){
	
	cleanup_strkey_to_empty($1);
}
// input: get_ea_data
%typemap(in,numinputs=0) (struct fhash_fhash **mapped_orig_fhash_dict)(struct fhash_fhash *mapped_orig_fhash_dict)
{
	mapped_orig_fhash_dict = NULL;
	$1 = &mapped_orig_fhash_dict;

}

// output: get_ea_data
%typemap(argout) (struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list, 
		struct fhash_fhash **mapped_orig_fhash_dict)
{	
	$result = PyTuple_New(8);
	
	// output 3
	PyObject *pyhashdict;
	pyhashdict = PyDict_New();
	
	// output 4 5 6
	PyObject *mixed_anomaly, *mw_anomaly, *gs_anomaly; 
	mixed_anomaly = PyDict_New();
	mw_anomaly = PyDict_New();
	gs_anomaly = PyDict_New();
	
	struct fhash_fhash *current_mapped_hash;
	struct fhash_fhash *tmp_mapped_hash;
	
	int dictindx = 0;
	
	int gs_initialized = 0;
	int mw_initialized = 0;
	
	// output 1
	UT_string *libsvmstr_gs ;
	utstring_new(libsvmstr_gs);
	
	UT_string *libsvmstr_mw ;
	utstring_new(libsvmstr_mw);
	
	// output 2
	UT_array *ea_fhash_list_gs;
	UT_array *ea_fhash_list_mw;
	utarray_new(ea_fhash_list_gs, &ut_str_icd);
	utarray_new(ea_fhash_list_mw, &ut_str_icd);
	
	// output 7 8
	int ret_no_gs_samples = 0;
	int ret_no_mw_samples = 0;
	
	HASH_ITER(hh, (*$3), current_mapped_hash, tmp_mapped_hash){
		int num_mw = utarray_len(current_mapped_hash->orig_fhash_list_mw);
		int num_gs = utarray_len(current_mapped_hash->orig_fhash_list_gs);
    		
		if (num_mw > 0 && num_gs > 0)
		{		
			PyObject *tuple_temp;
			PyObject *gs_list_temp;
			PyObject *mw_list_temp;
			tuple_temp = PyTuple_New(2);
			gs_list_temp = PyList_New(num_gs);
			mw_list_temp = PyList_New(num_mw);
			
			int mwcount = 0;
			int gscount = 0;
			
			// write the gs svm string
			swig_create_libsvm_str(LABEL_CLEAN, current_mapped_hash->mapped_fhash, &libsvmstr_gs);
			
			char **c = utarray_front(current_mapped_hash->orig_fhash_list_gs);
			swig_add_fhash(*c , &ea_fhash_list_gs);
			ret_no_gs_samples = ret_no_gs_samples + 1;
								
			
			char **p;
			p = NULL;
			// get the anomaly list and also the svm return string for gs			 		
			while ((p = (char**) utarray_next(current_mapped_hash->orig_fhash_list_gs, p)))
			{
				// fhash is *p
				// anomaly list of gs
				PyList_SetItem(gs_list_temp, gscount, PyString_FromString(*p));
				gscount = gscount + 1;
				PyDict_SetItem(pyhashdict, PyString_FromString(*p), PyInt_FromLong(dictindx));		
			}
			
			p = NULL;
			while ((p = (char**) utarray_next(current_mapped_hash->orig_fhash_list_mw, p)))
			{
				// fhash is *p
				PyList_SetItem(mw_list_temp, mwcount, PyString_FromString(*p));
				mwcount = mwcount + 1;				
			}
	
			// add them to the tuple
			PyTuple_SetItem(tuple_temp, 0, mw_list_temp);			
			PyTuple_SetItem(tuple_temp, 1, gs_list_temp);
			// add tuple to the list mixed_anomaly
			PyDict_SetItem(mixed_anomaly, PyInt_FromLong(dictindx), tuple_temp);
			dictindx = dictindx + 1;
			continue;
		}
		
		
		if (num_gs > 1)
		{
			PyObject *gs_list_temp;
			gs_list_temp = PyList_New(num_gs);
			int gscount = 0;
							
			swig_create_libsvm_str(LABEL_CLEAN, current_mapped_hash->mapped_fhash, &libsvmstr_gs);
			char **c = utarray_front(current_mapped_hash->orig_fhash_list_gs);
			swig_add_fhash(*c, &ea_fhash_list_gs);
			ret_no_gs_samples = ret_no_gs_samples + 1;
			
			char **p;
			p = NULL;
			
			while ((p = (char**) utarray_next(current_mapped_hash->orig_fhash_list_gs, p))) 
			{
				// fhash is *p
				// anomaly list of gs
				PyList_SetItem(gs_list_temp, gscount, PyString_FromString(*p));
				gscount = gscount + 1;				
				// setting the fhash in the string
				PyDict_SetItem(pyhashdict,PyString_FromString(*p),PyInt_FromLong(dictindx));		
			}					
						
			PyDict_SetItem(gs_anomaly,PyInt_FromLong(dictindx),gs_list_temp);
			dictindx = dictindx + 1;
			continue;		
		}
		
		if (num_mw == 1)
		{							
			char **p = utarray_front(current_mapped_hash->orig_fhash_list_mw);
			
			swig_create_libsvm_str(LABEL_MW, current_mapped_hash->mapped_fhash, &libsvmstr_mw);
			swig_add_fhash(*p, &ea_fhash_list_mw);			
			ret_no_mw_samples = ret_no_mw_samples + 1;
			
			// setting the fhash in the string
			PyDict_SetItem(pyhashdict,PyString_FromString(*p), Py_None);			
			continue;
		}		
		if (num_gs == 1)
		{							
			char **p = utarray_front(current_mapped_hash->orig_fhash_list_gs);
			
			swig_create_libsvm_str(LABEL_CLEAN, current_mapped_hash->mapped_fhash, &libsvmstr_gs);
			swig_add_fhash(*p, &ea_fhash_list_gs);
			ret_no_gs_samples = ret_no_gs_samples + 1;
			// setting the fhash in the string
			PyDict_SetItem(pyhashdict, PyString_FromString(*p), Py_None);			
			continue;
		}
									
		if (num_mw > 1)
		{
			PyObject *mw_list_temp;
			mw_list_temp = PyList_New(num_mw);
			int mwcount = 0;
			swig_create_libsvm_str(LABEL_MW, current_mapped_hash->mapped_fhash, &libsvmstr_mw);
			char **c = utarray_front(current_mapped_hash->orig_fhash_list_mw);									
			swig_add_fhash(*c, &ea_fhash_list_mw);
			ret_no_mw_samples = ret_no_mw_samples + 1;		
			
			char **p;
			p = NULL;
			while ((p = (char**) utarray_next(current_mapped_hash->orig_fhash_list_mw, p))) 
			{
				// fhash is *p
				PyList_SetItem(mw_list_temp, mwcount, PyString_FromString(*p));
				mwcount = mwcount + 1;
				
				// setting the fhash in the string
				PyDict_SetItem(pyhashdict, PyString_FromString(*p), PyInt_FromLong(dictindx));
			}
			PyDict_SetItem(mw_anomaly,PyInt_FromLong(dictindx),mw_list_temp);
			dictindx = dictindx + 1;
			continue;
		}
	}
	
	utstring_concat(libsvmstr_mw, libsvmstr_gs);
	utstring_free(libsvmstr_gs);

	char *ret_svm_str;
	size_t len_combined = libsvmstr_mw->i;
	size_t zerosize = 0;
	
	if (len_combined == zerosize) {
		PyTuple_SetItem($result, 0, Py_None);
	} else {
		UT_string *s;
		utstring_new(s);
		compress_string(utstring_body(libsvmstr_mw), s);
		PyTuple_SetItem($result, 0, PyString_FromString(utstring_body(s)));
		utstring_free(s);
	}
	
	utstring_free(libsvmstr_mw);
	
	// create the list of fhash to return
	
	char **p;
	p = NULL;
	
	int retsize_gs = utarray_len(ea_fhash_list_gs);
	int retsize_mw = utarray_len(ea_fhash_list_mw);
	int retsize = retsize_gs + retsize_mw;
	
	PyObject *py_ea_fhash_list;
	py_ea_fhash_list = PyList_New(retsize);
	int i = 0;
	
	while ( (p=(char**)utarray_next(ea_fhash_list_mw,p))) {
		PyList_SetItem(py_ea_fhash_list, i, PyString_FromString(*p));
		i = i + 1;
  	}
  	
  	p = NULL;  
  	while ( (p=(char**)utarray_next(ea_fhash_list_gs,p))) {
		PyList_SetItem(py_ea_fhash_list, i, PyString_FromString(*p));
		i = i + 1;
  	}
		
	PyTuple_SetItem($result, 1, py_ea_fhash_list);
	PyTuple_SetItem($result, 2, pyhashdict);
	PyTuple_SetItem($result, 3, mixed_anomaly);
	PyTuple_SetItem($result, 4, mw_anomaly);
	PyTuple_SetItem($result, 5, gs_anomaly);
	PyTuple_SetItem($result, 6, PyInt_FromLong(ret_no_gs_samples));
	PyTuple_SetItem($result, 7, PyInt_FromLong(ret_no_mw_samples));
		
	utarray_free(ea_fhash_list_gs);
	utarray_free(ea_fhash_list_mw);
	cleanup_key_map($1);
	cleanup_key_min_max($2);
	cleanup_fhash_fhash($3);
}

// input: get_crossValidation_data
%typemap(in,numinputs=1) (struct strkey_to_empty **ignore_fhash_list, char **compressed_cv_lsvm_str, UT_array **ret_fhash_data, size_t *num_gs_samples, size_t *num_mw_samples, size_t *len_str)
		(struct strkey_to_empty *ignore_fhash_list, char *compressed_cv_lsvm_str, UT_array *ret_fhash_data, size_t num_gs_samples, size_t num_mw_samples, size_t len_str)	
{		
	ignore_fhash_list =  NULL;
	
	assert(PyList_Check($input));
	
	Py_ssize_t pos = PyList_Size($input);
	Py_ssize_t i=0;
	PyObject *fhash;
	for (i = 0; i < pos; i++)
	{
		fhash = PyList_GetItem($input,i);
		char *c = PyString_AsString(fhash);
		struct strkey_to_empty *newdata = malloc(sizeof(struct strkey_to_empty));
	    newdata->key = c;
	    newdata->value = 1;
	    HASH_ADD_KEYPTR(hh, ignore_fhash_list, newdata->key, strlen(c), newdata);
	}
	
	$1 = &ignore_fhash_list;
	$2 = &compressed_cv_lsvm_str;
	$3 = &ret_fhash_data;
	$4 = &num_gs_samples;
	$5 = &num_mw_samples;
	$6 = &len_str;
}
		
// input: get_crossValidation_data
%typemap(in,numinputs=1) (struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list)
		(struct key_map* mapping_old_to_new, struct key_min_max *oldkey_min_max_list)
{		
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
	
	$1 = &mapping_old_to_new;
	$2 = &oldkey_min_max_list;
}


// output: get_crossValidation_data
%typemap(argout) (char **compressed_cv_lsvm_str, UT_array **ret_fhash_data, size_t *num_gs_samples, size_t *num_mw_samples, size_t *len_str)
{
	$result = PyTuple_New(5);
	// output 1
	if(*$1 != NULL)
	{
		PyTuple_SetItem($result, 0, PyString_FromString(*$1));
		free(*$1);
	}
	else
	{
		PyTuple_SetItem($result, 0, Py_None);
	}
	
	// output 2
	int a = utarray_len(*$2);
	if (a == 0)
	{
		PyTuple_SetItem($result, 1, Py_None);
	}else
	{
		char **p;
		p=NULL;
		PyObject *retfhashlist;
		retfhashlist = PyList_New(a);
		int c = 0;	
  		while ( (p=(char**)utarray_next(*$2, p))) {	
			PyList_SetItem(retfhashlist, c, PyString_FromString(*p));
			c = c + 1;
	  	}
		PyTuple_SetItem($result, 1, retfhashlist);
		utarray_free(*$2);
	}
	
	// output 3 4
	PyTuple_SetItem($result, 2, PyInt_FromLong(*$3));
	PyTuple_SetItem($result, 3, PyInt_FromLong(*$4));
	PyTuple_SetItem($result, 4, PyInt_FromLong(*$5));
	
}

// input _nn methods
%typemap(in,numinputs=0) (struct nn_data **k_nn_data)
						(struct nn_data *k_nn_data){
	$1 = &k_nn_data;
}

// output of get__nn methods
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



// output of get_anomaly_data_from_key 
%typemap(argout) (struct base_data_struct *training_clone_list, int anomaly_id) {
	// at the output stage, need to return the dict with the anomaly_id by checking in base_data_struct->anomaly_data_list
	
//	log_info("Inside get_anomaly_data_from_key for key %d", $2);
	
	struct anomaly_data *s;
	int ana_id = $2;
	HASH_FIND_INT($1->anomaly_data_list, &ana_id, s);
	
	
	if(s == NULL)
	{
		log_info("No anomaly key: %d",$2);
		$result  = Py_None;
	}
	else
	{
		$result = PyDict_New();		
		PyDict_SetItem($result, PyString_FromString("ana_type"), PyInt_FromLong(s->anomaly_type));
		PyDict_SetItem($result, PyString_FromString("ana_type_method"), PyInt_FromLong(s->anomaly_type_method));
		PyObject *mwlist;
		int num_mw = 0;
		if (s->orig_fhash_list_mw != NULL)
		{
			num_mw = utarray_len(s->orig_fhash_list_mw);
			//log_info("Number of Mw: %d",num_mw);
			mwlist = PyList_New(num_mw);
			int i = 0;
			char **p;
			p = NULL;  
	  		while ( (p=(char**)utarray_next(s->orig_fhash_list_mw, p))) {
				PyList_SetItem(mwlist, i, PyString_FromString(*p));
				i = i + 1;
	  		}	
		}else
		{
			mwlist = PyList_New(0);
		}
		
		PyObject *gslist;
		int num_gs = 0;
		
		if (s->orig_fhash_list_gs != NULL)
		{
			num_gs = utarray_len(s->orig_fhash_list_gs);
			//log_info("Number of GS: %d",num_gs);
			gslist = PyList_New(num_gs);
					
			int i = 0;
			char **p;
			p = NULL;  
	  		while ( (p=(char**)utarray_next(s->orig_fhash_list_gs, p))) {
				PyList_SetItem(gslist, i, PyString_FromString(*p));
				i = i + 1;
	  		}		
					
		}else
		{
			gslist = PyList_New(0);
		}
		
		PyDict_SetItem($result, PyString_FromString("mw_fhash") , mwlist);
		PyDict_SetItem($result, PyString_FromString("gs_fhash") , gslist);	
	}
}

void clone_training_data(struct base_data_struct* orig_data_pointer_list, struct base_data_struct **training_clone_list);
void dispaly_cloned_data(struct base_data_struct *training_clone_list);
void free_clone_pointer(struct base_data_struct *training_clone_list);

void add_mrmr_ana(struct base_data_struct *training_clone_list, struct anomaly_data **anomaly_data_list);

void update_clone_mrmr_anomaly(struct base_data_struct *training_clone_list, struct strkey_to_empty **fhash_hashey);

void get_anomaly_data_from_key(struct base_data_struct *training_clone_list, int anomaly_id);

void get_ea_data(struct base_data_struct* orig_data_pointer_list, struct key_map **mapping_old_to_new, struct key_min_max **oldkey_min_max_list,
	struct fhash_fhash **mapped_orig_fhash_dict);

void get_crossValidation_data(struct base_data_struct* orig_data_pointer_list, struct key_map** mapping_old_to_new, struct key_min_max ** oldkey_min_max_list,
		struct strkey_to_empty **ignore_fhash_list, char **compressed_cv_lsvm_str, UT_array **ret_fhash_data, size_t *num_gs_samples, size_t *num_mw_samples, size_t *len_str, bool is_compressed=1);
		
void update_mw_prob_fhash(struct base_data_struct* training_clone_list, char* fhash, double prob);

void get_mw_prob_fhash(struct base_data_struct* training_clone_list, char* fhash, double* OUTPUT);

void get_k_nn_fhashes_clone(struct base_data_struct* orig_data_pointer_list, char* input_fhash, int k, struct nn_data **k_nn_data);

void get_k_nn_libsvm_clone(struct base_data_struct* orig_data_pointer_list, char* in_svmstr, int k, struct nn_data **k_nn_data);
