# define PY_SSIZE_T_CLEAN
# include <Python.h>
#include <stdio.h>

static double EPS = 0;
static double MAX_DIS = 2147483647.0;
int MAX_ITTER = 200;
int NUMS_PER_POINT = 0;
int POINTS_NUM = 0;
int CLUSTERS_NUM = 0;

double ** createMatrix(int, int);
int * createDupArray(int, int);
void copyDoublePFromTo(double *, double *, int);
void copyDoublePPFromTo(double **, double **, int, int);
void freeMat(double **);
int closestCluster(double **, double *);
double distance(double *, double *, int);
void addPoint(double *, double *);
void divideMat(double **, int, int, int *);
void itterOver(double **, double **);
void zeroMat (double **, int, int);
int didConverge(double **, double **, int, int);

/*Func returns double * pointer to a contigous block (matrix) of memory.*/
double ** createMatrix(int rows, int cols) {
    double *mat = (double *) malloc((size_t) (rows * cols * sizeof(double)));
    double **mat_loc = (double **) malloc((size_t) (rows * sizeof(double *)));
    int i = 0;
    for (; i < rows; i++) {mat_loc[i] = mat + i * cols;}
    return mat_loc;
}

/*Func returns an integer pointer (array) of a given length and value for all cells.*/
int * createDupArray(int length, int num) {
    int *dupArray = (int *) malloc((size_t) (length * sizeof(int)));
    int i = 0;
    for (; i < length; i++) {dupArray[i] = num;}
    return dupArray;
    
}

/*Func copies a double * var into another double * var until a given index.*/
void copyDoublePFromTo(double *from, double *to, int length) {
    int i = 0;
    for (; i < length; i++) {to[i] = from[i];}
}

/*Func copies a double ** matrix into another double ** matrix.*/
void copyDoublePPFromTo(double **from, double **to, int length_row, int num_rows) {
    int i = 0;
    for (; i < num_rows; i++) {copyDoublePFromTo(from[i], to[i], length_row);}
}

/*Func frees a matrix created using createMat*/
void freeMat(double **mat) {
    free(mat[0]);    
    free(mat);
}

/*Func returns the index of the closest cluster to a point.*/
int closestCluster(double **clusters, double *point) {
    int min_index = 0;
    double min_dis = MAX_DIS , cur_dis;
    int i = 0;
    for (; i < CLUSTERS_NUM; i++) {
        cur_dis = distance(clusters[i], point, NUMS_PER_POINT);
        if (cur_dis < min_dis) {
            min_index = i;
            min_dis = cur_dis;
        }
    }
    return min_index;
}

/*Func returns the distance between two given data points.*/
double distance(double *point1, double *point2, int length) {
    double sum = 0;
    int i = 0;
    for (; i < length; i++) {sum += pow(point1[i] - point2[i], 2);}
    return sqrt(sum);
}

/*Func adds a point to a given cluster-mean.*/
void addPoint(double *cluster, double *point)  {
    int i = 0;
    for (; i < NUMS_PER_POINT; i++) {cluster[i] += point[i];}
}

/*
Func divides each element in a matrix according to the value matching it's row.
param matrix: the matrix we would like to divide.
param divider: an array of ints signifying which row should be divided by which number.
*/
void divideMat(double **matrix, int rows, int cols, int *divider) {
    int i = 0, j;
    for (; i < rows; i++) {for (j = 0; j < cols; j++) {matrix[i][j] /= divider[i];}}
}

/*
Func itterates over the points given and sorts them into the different clusters.
returns: The final cluster-means after convergence/MAX_ITTER itterations.
*/
void itterOver(double **clusters, double **points) {
    int i, closest, *nums_entered;
    double **means = createMatrix(CLUSTERS_NUM, NUMS_PER_POINT);
    copyDoublePPFromTo(clusters, means, NUMS_PER_POINT, CLUSTERS_NUM);
    do {
        i = 0;
        nums_entered = createDupArray(CLUSTERS_NUM, 0);
        copyDoublePPFromTo(means, clusters, NUMS_PER_POINT, CLUSTERS_NUM);
        zeroMat(means, CLUSTERS_NUM, NUMS_PER_POINT);
        for (; i < POINTS_NUM; i++) {
            closest = closestCluster(clusters, points[i]);
            addPoint(means[closest], points[i]);
            nums_entered[closest]++;
        }
        divideMat(means, CLUSTERS_NUM, NUMS_PER_POINT, nums_entered);
        free(nums_entered);
    } while (--MAX_ITTER > 0 && !didConverge(clusters, means, NUMS_PER_POINT, CLUSTERS_NUM));
    copyDoublePPFromTo(means, clusters, NUMS_PER_POINT, CLUSTERS_NUM);
    freeMat(means);
}

/*Func sets all the values of a matrix to zero*/
void zeroMat (double **means, int rows, int cols) {
    int i = 0, j;
    for (; i < rows; i++) {for (j = 0; j < cols; j++) {means[i][j] = 0;}}
}

/*
Func checks for convergence of the means vector to the clusters vector.
return: 1 if converged, 0 if didn't converge.
*/
int didConverge(double **points1, double **points2, int row_length, int num_rows) {
    int i = 0;
    for (;i < num_rows; i++) {if (distance(points1[i], points2[i], row_length) >= EPS) {return 0;}}
    return 1;
}
/*
Builds the return object for python.
*/
PyObject* makeDoublelist(double array[], size_t size) {
    PyObject *l = PyList_New(size);
    size_t i;
    for (i = 0; i != size; ++i) {
        PyList_SET_ITEM(l, i, PyFloat_FromDouble(array[i]));
    }
    return l;
}


static PyObject* fit(PyObject *self, PyObject *args)
{
    int max_iter;
    int listLenCentroids;
    int listLenDataPoints;
    int pointLen;
    double epsilon;
    PyObject* listOfListsCentroids;
    PyObject* listInListsCentroids;
    PyObject* listOfListsDataPoints;
    PyObject* listInListsDataPoints;
    if(!PyArg_ParseTuple(args, "idOO" ,&max_iter, &epsilon, &listOfListsCentroids, &listOfListsDataPoints)){
        return NULL;
    }
    
    listLenCentroids = PyList_GET_SIZE(listOfListsCentroids);
    listLenDataPoints = PyList_GET_SIZE(listOfListsDataPoints);
    listInListsCentroids = PyList_GetItem(listOfListsCentroids, 0);
    listInListsDataPoints = PyList_GetItem(listOfListsDataPoints, 0);
    pointLen = PyList_GET_SIZE(listInListsCentroids);

    MAX_ITTER = max_iter;
    EPS = epsilon;
    NUMS_PER_POINT = pointLen;
    CLUSTERS_NUM = listLenCentroids;
    POINTS_NUM = listLenDataPoints;

    double value;
    Py_ssize_t i;
    Py_ssize_t j;

    double ** centroids = createMatrix(listLenCentroids, pointLen);
    for (i = 0; i < listLenCentroids; ++i) {
        listInListsCentroids = PyList_GetItem(listOfListsCentroids, i);
        for (j = 0; j < pointLen; ++j) {
            PyObject* node = PyList_GET_ITEM(listInListsCentroids, j);
            value = PyFloat_AsDouble(node);
            centroids[i][j] = value;
        }
    }

    double ** dataPoints = createMatrix(listLenDataPoints, pointLen);
    for (i = 0; i < listLenDataPoints; ++i) {
        listInListsDataPoints = PyList_GetItem(listOfListsDataPoints, i);
        for (j = 0; j < pointLen; ++j) {
            PyObject* node = PyList_GET_ITEM(listInListsDataPoints, j);
            value = PyFloat_AsDouble(node);
            dataPoints[i][j] = value;
        }
    }

    itterOver(centroids, dataPoints);

    PyObject* results = PyList_New(listLenCentroids);
    for (i = 0; i < listLenCentroids; i++){
        PyList_SetItem(results, i,  makeDoublelist(centroids[i], pointLen));
    }

    
    freeMat(centroids);
    freeMat(dataPoints);
    return results;
}


static PyMethodDef clusteringMethods[] = {
    {"fit",                   
      (PyCFunction) fit, 
      METH_VARARGS,           
      PyDoc_STR("fit(int max_iter, float epsilon, matrix centroids, matrix data points). Clustering given data points to given number of clusters, based on given centroids. Returns the new centroids after the clustering.")}, /*  The docstring for the function */
    {NULL, NULL, 0, NULL} 
};

static struct PyModuleDef cluseringmodule = {
    PyModuleDef_HEAD_INIT,
    "mykmeanssp", /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    clusteringMethods /* the PyMethodDef array from before containing the methods of the extension */
};


PyMODINIT_FUNC PyInit_mykmeanssp(void)
{
    PyObject *m;
    m = PyModule_Create(&cluseringmodule);
    if (!m) {
        return NULL;
    }
    return m;
}
