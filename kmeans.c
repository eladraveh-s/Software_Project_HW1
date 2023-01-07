#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static double EPS = 0.001;
static double MAX_DIS = 2147483647.0;
int MAX_ITTER = 200;
int LINE_SIZE = 10;
int NUMS_PER_POINT = 0;
int POINTS_NUM = 0;
int CLUSTERS_NUM = 0;

double *** processFile();
double * processFirstLine();
char * getLineFromSTDIN();
int checkGotLine(char *, int *);
void enterAfter(char *);
double processNum(char *, int *);
double ** createMatrix(int, int);
int * createDupArray(int, int);
void copyDoublePFromTo(double *, double *, int);
double ** processPoints(double **, int *);
void copyDoublePPFromTo(double **, double **, int, int);
double ** doubleDoublePPLength(double **, int);
void freeMat(double **);
double * processLine();
int closestCluster(double **, double *);
double distance(double *, double *, int);
void addPoint(double *, double *);
void divideMat(double **, int, int, int *);
void itterOver(double **, double **);
void zeroMat (double **, int, int);
int didConverge(double **, double **, int, int);
void printClusters(double **);

/*
Func processes the points file given through STDIN.
return: double ***file_info, which contains the clusters(means) and all of the points read.
*/
double *** processFile() {
    double *point = processFirstLine(), **clusters = createMatrix(CLUSTERS_NUM, NUMS_PER_POINT);
    double ***file_info = (double ***) malloc((size_t) (2 * sizeof(double **))), **points;
    int *nums_entered = createDupArray(CLUSTERS_NUM, 0);
    nums_entered[0] = 1;
    copyDoublePFromTo(point, clusters[0], NUMS_PER_POINT);
    points = processPoints(clusters, nums_entered);
    free(nums_entered);
    free(point);
    file_info[0] = clusters;
    file_info[1] = points;
    return file_info;
}

/*
Func processes the first line of the file to NUMS_PER_POINT and suspected LINE_SIZE.
return: double *point - the first point in the file.
*/
double * processFirstLine() {
    int size = 2, *indp = malloc(sizeof(int));
    double *point = (double *) malloc((size_t) (size * sizeof(double)));
    char *line = getLineFromSTDIN();
    *indp = 0;
    if (line == NULL) {
        free(indp);
        return NULL;
    }
    while (line[(*indp)] != '\0') {
        point[NUMS_PER_POINT++] = processNum(line, indp);
        if (size == NUMS_PER_POINT) {point = (double *) realloc(point, (size_t) ((size *= 2) * sizeof(double)));}
        (*indp)++;
    }
    free(indp);
    free(line);
    return point;
}

/*
Func recieves a single line from STDIN.
return: given char * of line.
*/
char * getLineFromSTDIN() {
    char *line = (char *) malloc((size_t) (LINE_SIZE * sizeof(char)));
    char *pointer;
    int *indp = malloc(sizeof(int));
    *indp = 0;
    pointer = fgets(line, LINE_SIZE, stdin);
    if (pointer == NULL) {
        free(indp);
        free(line);
        return NULL;
    }
    while (!checkGotLine(line, indp)) {
        line = (char *) realloc(line, (size_t) ((LINE_SIZE * 2) * sizeof(char)));
        enterAfter(line + *indp);
        LINE_SIZE *= 2;
    }
    free(indp);
    return line;
}

/*
Func checks for the success of the memory allocatin needed to recieve a line.
param line: points to the recieved line - saved as a char array.
param indp: a pointer to the last checked index of the array.
return: 1 on success, 0 on failure.
*/
int checkGotLine(char *line, int *indp) {
    for (; *indp < LINE_SIZE; (*indp)++) {if (line[*indp] == '\0') {return line[*indp - 1] == '\n';}}
    return 0;
}


void enterAfter(char *line) {line = fgets(line, LINE_SIZE, stdin);}

/*
Func computes the numerical value of a double given as a string.
param line: the line from which we try to read the number.
param indp: the index of the first char which is a part of the number.
return: the numerical value of a double given as a string.
*/
double processNum(char *line, int *indp) {
    int size = 8, i = 0, start = *indp;
    char *num_str = (char *) malloc((size_t) (size * sizeof(char)));
    double num;
    while (line[*indp] != ',' && line[*indp] != '\n') {
        num_str[i] = line[(*indp)++];
        if (size == ++i) {num_str = (char *) realloc(num_str, (size_t) ((size *= 2) * sizeof(char)));}
    }
    num_str = (char *) realloc(num_str, (size_t) ((*indp - start + 1) * sizeof(char)));
    num_str[*indp - start] = '\0';
    num = atof(num_str);
    free(num_str);
    return num;
}

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

/*
Func processes the points given in the file to the given pointers.
param clusters: a pointer to the (empty) clusters matrix.
param nums_entered: an integer array signifyign how many numbers went into each centroid.
return: double ** matrix containing all of the points in the file.
*/
double ** processPoints(double **clusters, int *nums_entered) {
    int length = CLUSTERS_NUM, closest;
    double **means = createMatrix(CLUSTERS_NUM, NUMS_PER_POINT), *cur_point;
    double **points = createMatrix(CLUSTERS_NUM, NUMS_PER_POINT);
    copyDoublePFromTo(clusters[0], points[0], NUMS_PER_POINT);
    copyDoublePPFromTo(clusters, means, NUMS_PER_POINT, CLUSTERS_NUM);
    while ((cur_point = processLine()) != NULL) {
        if (POINTS_NUM++ == length - 1) {
            points = doubleDoublePPLength(points, length);
            if (points == NULL) {
                freeMat(points);
                freeMat(means);
                freeMat(clusters);
                free(nums_entered);
                return NULL;
            }
            length *= 2;
        }
        copyDoublePFromTo(cur_point, points[POINTS_NUM], NUMS_PER_POINT);
        free(cur_point);
        if (POINTS_NUM < CLUSTERS_NUM) {
            copyDoublePFromTo(points[POINTS_NUM], clusters[POINTS_NUM], NUMS_PER_POINT);
            copyDoublePFromTo(points[POINTS_NUM], means[POINTS_NUM], NUMS_PER_POINT);
            nums_entered[POINTS_NUM]++;
        }
        else {
            closest = closestCluster(clusters, points[POINTS_NUM]);
            addPoint(means[closest], points[POINTS_NUM]);
            nums_entered[closest]++;
        }
    }
    POINTS_NUM++;
    if (POINTS_NUM < CLUSTERS_NUM) {printf("Invalid number of clusters!\n");}
    divideMat(means, CLUSTERS_NUM, NUMS_PER_POINT, nums_entered);
    copyDoublePPFromTo(means, clusters, NUMS_PER_POINT, CLUSTERS_NUM);
    freeMat(means);
    return points;
}

/*Func copies a double ** matrix into another double ** matrix.*/
void copyDoublePPFromTo(double **from, double **to, int length_row, int num_rows) {
    int i = 0;
    for (; i < num_rows; i++) {copyDoublePFromTo(from[i], to[i], length_row);}
}

/*
Func doubles the given space to a double ** variable.
param points: the matrix we would like to "move".
param length: the amount of rows in the matrix.
return: a pointer to the memory block which is twice the size.
*/
double ** doubleDoublePPLength(double **points, int length) {
    double **tmp = createMatrix(length * 2, NUMS_PER_POINT);
    if (tmp == NULL) {return NULL;}
    copyDoublePPFromTo(points, tmp, NUMS_PER_POINT, length);
    freeMat(points);
    return tmp;
}

/*Func frees a matrix created using createMat*/
void freeMat(double **mat) {
    free(mat[0]);
    free(mat);
}

/*
Func processes a line of input from STDIN.
return: double * point - a numerical represntation of the line.
*/
double * processLine() {
    double *point = (double *) malloc((size_t) (NUMS_PER_POINT * sizeof(double)));
    int i = 0, *indp = (int *) malloc(sizeof(int));
    char *line = getLineFromSTDIN();
    *indp = 0;
    if (line == NULL) {
        free(indp);
        free(point);
        return NULL;
    }
    for (; i < NUMS_PER_POINT; i++) {
        point[i] = processNum(line, indp);
        (*indp)++;
    }
    free(line);
    free(indp);
    return point;
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
Func itterates over the points given and sorts them into te different clusters.
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

/*Func prints the final cluster-means after the end of the proccessing information.*/
void printClusters(double **clusters) {
    int i = 0, j;
    for (;i < CLUSTERS_NUM; i++) {
        for (j = 0; j < NUMS_PER_POINT - 1; j++) {printf("%.4f,", clusters[i][j]);}
        printf("%.4f", clusters[i][NUMS_PER_POINT - 1]);
        printf("\n");
    }
    freeMat(clusters);
}

int main(int argc, char *argv[]) {
    double ***file_info;
    if (argc < 2 || argc > 3) {
        printf("An error has accured\n");
        return 0;
    }
    else if (argc == 3) {
        MAX_ITTER = atoi (argv[2]);
        if (MAX_ITTER < 1 || MAX_ITTER > 1000) {
            printf("Invalid maximum itteration!\n");
            return 0;
        }
    }
    CLUSTERS_NUM = atoi (argv[1]);
    if (CLUSTERS_NUM < 1) {
        printf("Invalid number of clusters!\n");
        return 0;
    }
    file_info = processFile();
    if (file_info == NULL) {
        printf("An error has accured\n");
        return 0;
    }
    if (CLUSTERS_NUM >= POINTS_NUM) {return 0;}
    itterOver(file_info[0], file_info[1]);
    printClusters(file_info[0]);
    freeMat(file_info[1]);
    free(file_info);
    return 1;
}