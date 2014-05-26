%module backend_cluster_service

%include<cstring.i>
%include "../swig_callbacks/callback.i"
%include "bcs_common_methods.i"

%typemap(in,numinputs=0) (struct base_data_struct** orig_data_pointer_list)
					(struct base_data_struct* orig_data_pointer_list) {
    $1 = &orig_data_pointer_list;
}
// argout clone_training_data
%typemap(argout) struct base_data_struct **orig_data_pointer_list {
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
%typemap(in,numinputs=1) (int *enumid)
					(int enumid) {
	enumid = PyInt_AsLong($input);
    $1 = &enumid;
}

// output: get_hashes_from_enumid
%typemap(argout) (struct base_data_struct* orig_data_pointer_list, int *enumid)
{	
	int request_id = *$2;
	int ennum_len = utarray_len($1->enum_hashes_list);
	bool error = false;
	
	if(request_id >= ennum_len)
	{
		error = true;
	}else
	{
		int running_count = 0;
		int num_ele_ret = 0;
		
		num_ele_ret = ennum_len - request_id;
		
		$result = PyTuple_New(2);
		
		PyObject *sha_md_list = PyList_New(num_ele_ret);
		int i=0;
		for(i = request_id;i<ennum_len;i++)
		{
			struct enum_hashes *ele; 
			ele = (struct enum_hashes*)utarray_eltptr($1->enum_hashes_list, i);
			//ele = (struct enum_hashes*)utarray_front($1->enum_hashes_list);
			
			if(i != ele->enum_id)
			{
				log_info("Enum id mismatch! Abort");
				error = true;
				break;
			}
			PyObject *sha_md5_tuple;
			sha_md5_tuple = PyTuple_New(2);
			PyTuple_SetItem(sha_md5_tuple, 0, PyString_FromString(ele->sha));
			PyTuple_SetItem(sha_md5_tuple, 1, PyString_FromString(ele->md5));
			
			PyList_SetItem(sha_md_list, running_count,sha_md5_tuple);
			running_count = running_count + 1;
		}
		PyTuple_SetItem($result, 0, sha_md_list);
		PyTuple_SetItem($result, 1, PyInt_FromLong(ennum_len));
	}
	if(error == true)
	{
		$result = PyTuple_New(2);
		PyTuple_SetItem($result, 0,Py_None);
		PyTuple_SetItem($result, 1, PyInt_FromLong(request_id));
	}
		
}

%include<cstring.i>
%cstring_output_allocate(char **output, free(*$1));

void add_data_fhash_data_list(struct base_data_struct* orig_data_pointer_list, char* fhash, char* md5, char* sha, char* base_sha, double timestamp, int priority,
	int label, char* svmdatastr, int is_orig, bool* OUTPUT, int* OUTPUT, bool* OUTPUT);

void add_additional_data_fhash_data_list(struct base_data_struct* orig_data_pointer_list, char* fhash, char* md5, char* sha, char* base_sha, double timestamp,
	int priority, int label);

void delete_fhash_data(struct base_data_struct* orig_data_pointer_list, char* fhash);

void initialize_backend_cluster_service(struct base_data_struct** orig_data_pointer_list);

void get_hashes_from_enumid(struct base_data_struct* orig_data_pointer_list, int *enumid);