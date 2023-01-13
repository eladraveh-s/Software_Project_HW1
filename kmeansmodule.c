# define PY_SSIZE_T_CLEAN
# include <Python.h>
#include <stdio.h>

static PyObject* fit(PyObject *self, PyObject *args)
{
    int k;
    int max_iter;
    int listLen;
    int innerListLen;
    PyObject* listOfLists;
    PyObject* listInLists;
    PyArg_ParseTuple(args, "O", &listOfLists);

    if (!PyList_CheckExact(listOfLists)) {
        PyErr_SetString(PyExc_RuntimeError, "Received non-list type object.");
        return NULL;
    }
    
    listLen = PyList_GET_SIZE(listOfLists);
    listInLists = PyList_GetItem(listOfLists, 0);
    innerListLen = PyList_GET_SIZE(listInLists);
    double value;
    Py_ssize_t i;
    Py_ssize_t j;

    double* centroids = (double*)malloc((listLen * innerListLen) * sizeof(double));
    for (i = 0; i < listLen; ++i) {
        listInLists = PyList_GetItem(listOfLists, i);
        for (j = 0; j < innerListLen; ++j) {
            PyObject* node = PyList_GET_ITEM(listInLists, j);
            value = PyFloat_AsDouble(node);
            *(centroids + i*innerListLen + j) = value;
        }
    }


/* This builds the answer ("d" = Convert a C double to a Python floating point number) back into a python object */
    return Py_BuildValue("d", *centroids); /*  Py_BuildValue(...) returns a PyObject*  */
}



static PyMethodDef clusteringMethods[] = {
    {"fit",                   /* the Python method name that will be used */
      (PyCFunction) fit, /* the C-function that implements the Python function and returns static PyObject*  */
      METH_VARARGS,           /* flags indicating parameters
accepted for this function */
      PyDoc_STR("we do clustering")}, /*  The docstring for the function */
    {NULL, NULL, 0, NULL}     /* The last entry must be all NULL as shown to act as a
                                 sentinel. Python looks for this entry to know that all
                                 of the functions for the module have been defined. */
};

static struct PyModuleDef cluseringmodule = {
    PyModuleDef_HEAD_INIT,
    "kmeanssp", /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    clusteringMethods /* the PyMethodDef array from before containing the methods of the extension */
};


PyMODINIT_FUNC PyInit_kmeanssp(void)
{
    PyObject *m;
    m = PyModule_Create(&cluseringmodule);
    if (!m) {
        return NULL;
    }
    return m;
}
