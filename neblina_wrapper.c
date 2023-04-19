#include "Python.h"
#include "neblina.h"
#include "neblina_std.h"
#include "neblina_vector.h"
#include "neblina_matrix.h"
#include "neblina_smatrix.h"
#include "neblina_complex.h"
#include "libneblina.h"

static PyObject* py_init_engine(PyObject* self, PyObject* args){
    cl_int err;
    cl_uint num_platforms;
    int device;
    
    if (!PyArg_ParseTuple(args, "i", &device)) return NULL;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if (err == CL_SUCCESS) {
            //std::cout << "Success. Platforms available: " << num_platforms
            //        << std::endl;
    } else {
            //std::cout << "Error. Platforms available: " << num_platforms
            //        << std::endl;
    }

    InitCLEngine(device);

    Py_RETURN_NONE;
}

static PyObject* py_stop_engine(PyObject* self, PyObject* args){
    ReleaseCLInfo(clinfo);

    Py_RETURN_NONE;
}

static void py_complex_delete(PyObject* self) {
    complex_t* comp = (complex_t*)PyCapsule_GetPointer(self, "py_complex_new");
    complex_delete(comp);
}


static PyObject* py_complex_new(PyObject* self, PyObject* args){
    double real;
    double imag;
    if (!PyArg_ParseTuple(args, "dd", &real, &imag)) return NULL;

    complex_t * a = complex_new(real, imag);

    PyObject* po = PyCapsule_New((void*)a, "py_complex_new", py_complex_delete);

    return po;
}

static void py_vector_delete(PyObject* self) {
    vector_t* vec = (vector_t*)PyCapsule_GetPointer(self, "py_vector_new");
    //printf("vec %p\n",vec);
    //printf("vec->value %p\n",&(vec->value));
    //free ((void *)vec->value.f);
    //free ((void *)vec);
    vector_delete(vec);
}


static PyObject* py_vector_new(PyObject* self, PyObject* args){
    int len;
    int data_type;
    if (!PyArg_ParseTuple(args, "ii", &len,&data_type)) return NULL;
    //printf("create %d\n",len);
    vector_t * a = vector_new(len, data_type);
    //printf("malloc %p\n",a);
    PyObject* po = PyCapsule_New((void*)a, "py_vector_new", py_vector_delete);
    //printf("capsule_new %p\n",po);
    return po;
}

static PyObject* py_vector_set(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    int n;
    double real;
    double imag;
    if(!PyArg_ParseTuple(args, "Oidd:py_vector_set", &pf, &n, &real, &imag)) return NULL;

    //printf("print %d\n",n);
    //printf("pf %p\n",pf);
    vector_t * vec = (vector_t *)PyCapsule_GetPointer(pf, "py_vector_new");
    //printf("vec %p\n",vec);
    if (vec->type == T_COMPLEX) {
        vec->value.f[2*n] = real;
        vec->value.f[2*n+1] = imag;
    } else {
        vec->value.f[n] = real;
    }
    //printf("%lf\n",vec->value.f[n]);
    Py_RETURN_NONE;
}

static PyObject* py_vector_get(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    int n;
    if(!PyArg_ParseTuple(args, "Oi:py_vector_set", &pf, &n)) return NULL;

    //printf("print %d\n",n);
    //printf("pf %p\n",pf);
    vector_t * vec = (vector_t *)PyCapsule_GetPointer(pf, "py_vector_new");
    //printf("vec %p\n",vec);
    //printf("%lf\n",vec->value.f[n]);
    PyObject * result = PyFloat_FromDouble((double)vec->value.f[n]);
    return result;
}

static PyObject* py_move_vector_device(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_move_vector_device", &pf)) return NULL;

    //printf("pf %p\n",pf);
    
    vector_t * vec = (vector_t *)PyCapsule_GetPointer(pf, "py_vector_new");
    //printf("vec %p\n",vec);
    vecreqdev(vec);
    Py_RETURN_NONE;
}

static PyObject* py_move_vector_host(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_move_vector_device", &pf)) return NULL;

    //printf("pf %p\n",pf);
    
    vector_t * vec = (vector_t *)PyCapsule_GetPointer(pf, "py_vector_new");
    //printf("vec %p\n",vec);
    int n = (vec->type==T_FLOAT?vec->len:2*vec->len);
    vector_t * out = vector_new(vec->len, vec->type);
    cl_int status = clEnqueueReadBuffer(clinfo.q, (cl_mem)vec->extra, CL_TRUE, 0, n * sizeof (double), out->value.f, 0, NULL, NULL);
    CLERR
    PyObject* po = PyCapsule_New((void*)out, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_vec_add(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    PyObject* b = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_vec_add", &a, &b)) return NULL;

    //printf("a %p\n",a);
    //printf("b %p\n",b);
    
    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
        //printf("vec_a %p\n",vec_a);
    vector_t * vec_b = (vector_t *)PyCapsule_GetPointer(b, "py_vector_new");
        //printf("vec_b %p\n",vec_b);

    
    //TODO completar o vec_add
    object_t ** in = convertToObject(vec_a,vec_b);
    
    vector_t * r = (vector_t *) vec_add((void **) in, NULL );
    

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_matvec_mul(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    PyObject* b = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_matvec_mul", &a, &b)) return NULL;

    //printf("a %p\n",a);
    //printf("b %p\n",b);
    
    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
        //printf("vec_a %p\n",vec_a);
    matrix_t * mat_b = (matrix_t *)PyCapsule_GetPointer(b, "py_matrix_new");
        //printf("mat_b %p\n",mat_b);

    
    object_t ** in = convertToObject3(vec_a, mat_b);
    
    vector_t * r = (vector_t *) matvec_mul3((void **) in, NULL );

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_sparse_matvec_mul(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    PyObject* b = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_sparse_matvec_mul", &a, &b)) return NULL;

    //printf("a %p\n",a);
    //printf("b %p\n",b);
    
    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
//        printf("vec_a %p\n",vec_a);
    smatrix_t * smat_b = (smatrix_t *)PyCapsule_GetPointer(b, "py_sparse_matrix_new");
//        printf("smat_b %p\n",smat_b);

    
    object_t ** in = convertToObject4(vec_a, smat_b);
    
    vector_t * r = (vector_t *) matvec_mul3((void **) in, NULL );

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_vec_prod(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    PyObject* b = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_vec_add", &a, &b)) return NULL;

    //printf("a %p\n",a);
    //printf("b %p\n",b);
    
    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
        //printf("vec_a %p\n",vec_a);
    vector_t * vec_b = (vector_t *)PyCapsule_GetPointer(b, "py_vector_new");
        //printf("vec_b %p\n",vec_b);

    
    object_t ** in = convertToObject(vec_a,vec_b);
    
    vector_t * r = (vector_t *) vec_prod((void **) in, NULL );
    

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_vec_add_off(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    int offset;
    if(!PyArg_ParseTuple(args, "iO:py_vec_add_off", &offset, &a)) return NULL;

    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");

    object_t ** in = convertToObject2(offset, vec_a);
    
    vector_t * r = (vector_t *) vec_add_off((void **) in, NULL );   

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_vec_sum(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    if(!PyArg_ParseTuple(args, "O:py_vec_sum", &a)) return NULL;

    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
    
    object_t ** in = convertToObject(vec_a, NULL);
    
    object_t * r = (object_t *) vec_sum((void **) in, NULL );

    PyObject * result = PyFloat_FromDouble((double)r->value.f);
    
    return result;
}

static PyObject* py_vec_conj(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    if(!PyArg_ParseTuple(args, "O:py_vec_sum", &a)) return NULL;

    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
    
    object_t ** in = convertToObject(vec_a, NULL);
    
    vector_t * r = (vector_t *) vec_conj((void **) in, NULL );

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

//

static void py_matrix_delete(PyObject* self) {
    matrix_t* mat = (matrix_t*)PyCapsule_GetPointer(self, "py_matrix_new");
    //printf("mat %p\n",mat);
    //printf("mat->value %p\n",&(mat->value));
    //free ((void *)mat->value.f);
    //free ((void *)mat);
    matrix_delete(mat);
}


static PyObject* py_matrix_new(PyObject* self, PyObject* args){
    int rows;
    int cols;
    int data_type;
    if (!PyArg_ParseTuple(args, "iii", &rows,&cols,&data_type)) return NULL;
    //printf("create %d\n",rows);
    //printf("create %d\n",cols);
    matrix_t * a = matrix_new(rows, cols, data_type);
    //printf("malloc %p\n",a);
    PyObject* po = PyCapsule_New((void*)a, "py_matrix_new", py_matrix_delete);
    //printf("capsule_new %p\n",po);
    return po;
}

static PyObject* py_matrix_set(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    int i;
    int j;
    double real;
    double imag;
    if(!PyArg_ParseTuple(args, "Oiidd:py_matrix_set", &pf, &i, &j, &real, &imag)) return NULL;

    //printf("print (%d,%d)\n",i,j);
    //printf("pf %p\n",pf);
    matrix_t * mat = (matrix_t *)PyCapsule_GetPointer(pf, "py_matrix_new");
    //printf("mat %p\n",mat);
    int idx = (i*mat->ncol + j);
    if (mat->type == T_COMPLEX) {
        mat->value.f[2 * idx] = real;
        mat->value.f[2 * idx + 1] = imag;
    } else {
        mat->value.f[idx] = real;
    }
    //printf("%lf\n",mat->value.f[i*mat->ncol + j]);
    Py_RETURN_NONE;
}

static PyObject* py_matrix_get(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    int i;
    int j;
    if(!PyArg_ParseTuple(args, "Oii:py_matrix_get", &pf, &i, &j)) return NULL;

    //printf("print (%d,%d)\n",i,j);
    //printf("pf %p\n",pf);
    matrix_t * mat = (matrix_t *)PyCapsule_GetPointer(pf, "py_matrix_new");
    //printf("mat %p\n",mat);
    //printf("%lf\n",mat->value.f[i*mat->ncol + j]);
    PyObject * result = PyFloat_FromDouble((double)mat->value.f[i*mat->ncol + j]);
    return result;
}


static PyObject* py_move_matrix_device(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_move_matrix_device", &pf)) return NULL;

    //printf("pf %p\n",pf);
    
    matrix_t * mat = (matrix_t *)PyCapsule_GetPointer(pf, "py_matrix_new");
    //printf("mat %p\n",mat);
    matreqdev(mat);
    Py_RETURN_NONE;
}

static PyObject* py_move_matrix_host(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_move_matrix_device", &pf)) return NULL;

    //printf("pf %p\n",pf);
    
    matrix_t * mat = (matrix_t *)PyCapsule_GetPointer(pf, "py_matrix_new");
    //printf("mat %p\n",mat);
    int n = (mat->type==T_FLOAT?mat->nrow*mat->ncol:2*mat->nrow*mat->ncol);
    matrix_t * out = matrix_new(mat->nrow, mat->ncol, mat->type);
    cl_int status = clEnqueueReadBuffer(clinfo.q, mat->extra, CL_TRUE, 0, n * sizeof (double), out->value.f, 0, NULL, NULL);
    CLERR
    PyObject* po = PyCapsule_New((void*)out, "py_matrix_new", py_matrix_delete);
    return po;
}

static void py_sparse_matrix_delete(PyObject* self) {
    smatrix_t* mat = (smatrix_t*)PyCapsule_GetPointer(self, "py_sparse_matrix_new");
    //printf("smat %p\n",mat);
    //free ((void *)mat->m);
    //free ((void *)mat);
    smatrix_delete(mat);
}


static PyObject* py_sparse_matrix_new(PyObject* self, PyObject* args){
    int rows;
    int cols;
    int data_type;
    if (!PyArg_ParseTuple(args, "iii", &rows,&cols,&data_type)) return NULL;
    //printf("create %d\n",rows);
    //printf("create %d\n",cols);
    smatrix_t * a = smatrix_new(rows, cols, data_type);
    //printf("malloc %p\n",a);
    PyObject* po = PyCapsule_New((void*)a, "py_sparse_matrix_new", py_sparse_matrix_delete);
    //printf("capsule_new %p\n",po);
    return po;
}

static PyObject* py_sparse_matrix_set(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    int i;
    int j;
    double real;
    double imag;
    if(!PyArg_ParseTuple(args, "Oiidd:py_sparse_matrix_set", &pf, &i, &j, &real, &imag)) return NULL;

    //printf("print (%d,%d)\n",i,j);
    //printf("pf %p\n",pf);
    smatrix_t * mat = (smatrix_t *)PyCapsule_GetPointer(pf, "py_sparse_matrix_new");
    //printf("smat %p\n",mat);
    if(mat->type == T_COMPLEX) {
        smatrix_set_complex_value(mat,i,j,real, imag);
    } else if(mat->type == T_FLOAT) {
        smatrix_set_real_value(mat,i,j,real);
    }
    Py_RETURN_NONE;
}

static PyObject* py_sparse_matrix_pack(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_sparse_matrix_set", &pf)) return NULL;

    //printf("pf %p\n",pf);
    smatrix_t * mat = (smatrix_t *)PyCapsule_GetPointer(pf, "py_sparse_matrix_new");
    if (mat->type == T_FLOAT) {
        smatrix_pack(mat);
    } else {
        smatrix_pack_complex(mat);
    }
    Py_RETURN_NONE;
}

static PyObject* py_move_sparse_matrix_device(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_move_sparse_matrix_device", &pf)) return NULL;

    smatrix_t * smat = (smatrix_t *)PyCapsule_GetPointer(pf, "py_sparse_matrix_new");
    smatreqdev(smat);
    Py_RETURN_NONE;
}

static PyObject* py_move_sparse_matrix_host(PyObject* self, PyObject* args) {
    
    PyObject* pf = NULL;
    if(!PyArg_ParseTuple(args, "O:py_move_sparse_matrix_device", &pf)) return NULL;

    smatrix_t * smat = (smatrix_t *)PyCapsule_GetPointer(pf, "py_sparse_matrix_new");
    smatreqhost(smat); //should use this function? It seems that it creates the object in the stack
    //printf("mat %p\n",mat);
//    int n = (mat->type==T_FLOAT?mat->nrow*mat->ncol:2*mat->nrow*mat->ncol);
//    matrix_t * out = matrix_new(mat->nrow, mat->ncol, mat->type);
//    cl_int status = clEnqueueReadBuffer(clinfo.q, mat->mem, CL_TRUE, 0, n * sizeof (double), out->value.f, 0, NULL, NULL);
//    CLERR
    PyObject* po = PyCapsule_New((void*)smat, "py_sparse_matrix_new", py_sparse_matrix_delete);
    return po;
}

static PyObject* py_mat_add(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    PyObject* b = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_mat_add", &a, &b)) return NULL;

    //printf("a %p\n",a);
    //printf("b %p\n",b);
    
    matrix_t * mat_a = (matrix_t *)PyCapsule_GetPointer(a, "py_matrix_new");
        //printf("vec_a %p\n",vec_a);
    matrix_t * mat_b = (matrix_t *)PyCapsule_GetPointer(b, "py_matrix_new");
        //printf("vec_b %p\n",vec_b);

    
    //TODO completar o vec_add
    object_t ** in = convertMatMatToObject(mat_a,mat_b);
    
    matrix_t * r = (matrix_t *) mat_add((void **) in, NULL );
    

    PyObject* po = PyCapsule_New((void*)r, "py_matrix_new", py_matrix_delete);
    return po;
}

static PyObject* py_mat_mul(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    PyObject* b = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_mat_mul", &a, &b)) return NULL;

    //printf("a %p\n",a);
    //printf("b %p\n",b);
    
    matrix_t * mat_a = (matrix_t *)PyCapsule_GetPointer(a, "py_matrix_new");
        //printf("vec_a %p\n",vec_a);
    matrix_t * mat_b = (matrix_t *)PyCapsule_GetPointer(b, "py_matrix_new");
        //printf("vec_b %p\n",vec_b);

    
    //TODO completar o vec_add
    object_t ** in = convertMatMatToObject(mat_a,mat_b);
    
    matrix_t * r = (matrix_t *) mat_mul((void **) in, NULL );
    

    PyObject* po = PyCapsule_New((void*)r, "py_matrix_new", py_matrix_delete);
    return po;
}

static PyObject* py_scalar_mat_mul(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    double scalar;
    if(!PyArg_ParseTuple(args, "dO:py_scalar_mat_mul", &scalar,&a)) return NULL;

    matrix_t * mat_a = (matrix_t *)PyCapsule_GetPointer(a, "py_matrix_new");
    
    object_t ** in = convertScaMatToObject(scalar, mat_a);
    
    matrix_t * r = (matrix_t *) mat_mulsc((void **) in, NULL );

    PyObject* po = PyCapsule_New((void*)r, "py_matrix_new", py_matrix_delete);
    return po;
}

static PyObject* py_scalar_vec_mul(PyObject* self, PyObject* args) {
    
    PyObject* a = NULL;
    double scalar;
    if(!PyArg_ParseTuple(args, "dO:py_scalar_vec_mul", &scalar,&a)) return NULL;

    //printf("a %p\n",a);
    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
    //printf("vec_a %p\n",vec_a);
    
    object_t ** in = convertScaVecToObject(scalar, vec_a);
    
    vector_t * r = (vector_t *) vec_mulsc((void **) in, NULL );
    //    printf("r %p\n",r);
    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_complex_scalar_vec_mul(PyObject* self, PyObject* args) {
    
    PyObject* scalar = NULL;
    PyObject* a = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_complex_scalar_vec_mul", &scalar,&a)) return NULL;

    complex_t * complex_scalar = (complex_t *)PyCapsule_GetPointer(scalar, "py_complex_new");
    vector_t * vec_a = (vector_t *)PyCapsule_GetPointer(a, "py_vector_new");
    
    vector_t * r = NULL;
    if (vec_a->type == T_FLOAT) {
        r = (vector_t *) vec_mul_complex_scalar ( complex_scalar, vec_a); 
    } else if (vec_a->type == T_COMPLEX) {
        r = (vector_t *) mul_complex_scalar_complex_vec( complex_scalar, vec_a);
    }

    PyObject* po = PyCapsule_New((void*)r, "py_vector_new", py_vector_delete);
    return po;
}

static PyObject* py_complex_scalar_mat_mul(PyObject* self, PyObject* args) {
    
    PyObject* scalar = NULL;
    PyObject* a = NULL;
    if(!PyArg_ParseTuple(args, "OO:py_complex_scalar_mat_mul", &scalar,&a)) return NULL;

    complex_t * complex_scalar = (complex_t *)PyCapsule_GetPointer(scalar, "py_complex_new");
    matrix_t * mat_a = (matrix_t *)PyCapsule_GetPointer(a, "py_matrix_new");
    
    matrix_t * r = NULL;
    if (mat_a->type == T_FLOAT) {
        r = (matrix_t *) mul_complex_scalar_float_mat ( complex_scalar, mat_a); 
    } else if (mat_a->type == T_COMPLEX) {
        r = (matrix_t *) mul_complex_scalar_complex_mat( complex_scalar, mat_a);
    }

    PyObject* po = PyCapsule_New((void*)r, "py_matrix_new", py_matrix_delete);
    return po;
}


static PyMethodDef mainMethods[] = {
    {"init_engine", py_init_engine, METH_VARARGS, "init_engine"},
    {"stop_engine", py_stop_engine, METH_VARARGS, "stop_engine"},
    {"vector_new", py_vector_new, METH_VARARGS, "vector_new"},
    {"vector_set", py_vector_set, METH_VARARGS, "vector_set"},
    {"vector_get", py_vector_get, METH_VARARGS, "vector_get"},
    {"move_vector_device", py_move_vector_device, METH_VARARGS, "move_vector_device"},
    {"move_vector_host", py_move_vector_host, METH_VARARGS, "move_vector_host"},
    {"matrix_new", py_matrix_new, METH_VARARGS, "matrix_new"},
    {"matrix_set", py_matrix_set, METH_VARARGS, "matrix_set"},
    {"matrix_get", py_matrix_get, METH_VARARGS, "matrix_get"},
    {"move_matrix_device", py_move_matrix_device, METH_VARARGS, "move_matrix_device"},
    {"move_matrix_host", py_move_matrix_host, METH_VARARGS, "move_matrix_host"},
    {"sparse_matrix_new", py_sparse_matrix_new, METH_VARARGS, "sparse_matrix_new"},
    {"sparse_matrix_set", py_sparse_matrix_set, METH_VARARGS, "sparse_matrix_set"},
    {"sparse_matrix_pack", py_sparse_matrix_pack, METH_VARARGS, "sparse_matrix_pack"},
    {"move_sparse_matrix_device", py_move_sparse_matrix_device, METH_VARARGS, "move_sparse_matrix_device"},
    {"move_sparse_matrix_host", py_move_sparse_matrix_host, METH_VARARGS, "move_sparse_matrix_host"},
    {"vec_add", py_vec_add, METH_VARARGS, "vec_add"},
    {"matvec_mul", py_matvec_mul, METH_VARARGS, "matvec_mul"},
    {"sparse_matvec_mul", py_sparse_matvec_mul, METH_VARARGS, "sparse_matvec_mul"},
    {"vec_prod", py_vec_prod, METH_VARARGS, "vec_prod"},
    {"vec_add_off", py_vec_add_off, METH_VARARGS, "vec_add_off"},
    {"vec_sum", py_vec_sum, METH_VARARGS, "vec_sum"},
    {"vec_conj", py_vec_conj, METH_VARARGS, "vec_conj"},
    {"mat_add", py_mat_add, METH_VARARGS, "mat_add"},
    {"mat_mul", py_mat_mul, METH_VARARGS, "mat_mul"},
    {"scalar_mat_mul", py_scalar_mat_mul, METH_VARARGS, "scalar_mat_mul"},
    {"scalar_vec_mul", py_scalar_vec_mul, METH_VARARGS, "scalar_vec_mul"},
    {"complex_scalar_vec_mul", py_complex_scalar_vec_mul, METH_VARARGS, "complex_scalar_vec_mul"},
    {"complex_scalar_mat_mul", py_complex_scalar_mat_mul, METH_VARARGS, "complex_scalar_mat_mul"},
    {"complex_new", py_complex_new, METH_VARARGS, "complex_new"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef neblina = {
    PyModuleDef_HEAD_INIT,
    "neblina", "Neblina Core",
    -1,
    mainMethods
};

PyMODINIT_FUNC PyInit_neblina(void) {
    return PyModule_Create(&neblina);
}


