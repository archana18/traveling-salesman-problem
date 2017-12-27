#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>


/* Node */

typedef struct node {
    int level;
    int *path;
    int path_length;
    int bound;
} node_t;

node_t *new_node();

void set_node_level(node_t *node, int level);
void set_node_path(node_t *node, int N, int *path, int path_length);
void set_node_bound(node_t *node, int N, int matrix[N][N], int *path, int path_length);

/* Priority Queue */

int INIT_PQ_CAPACITY = 16;

void pq_insert(node_t **priority_queue, int *priority_queue_capacity, int *priority_queue_size, node_t *new_node);
node_t *pq_remove(node_t **priority_queue, int *priority_queue_size);

/* Travel */

void initializeTravel(int N, int matrix[N][N], FILE *output);
void travel(int N, int matrix[N][N], int *minCost, int *nodeCount, int minTour[N]);

int length(int N, int matrix[N][N], int *vector, int vector_size);

int is_member(int item, int *vector, int vector_size); 

/* debug */
void printArray(int N, int array[N]);
void printGraph(int N, int graph[N][N]);
/* * * * */

char *input_filename;
FILE *input_file;
char *input;

char *output_filename;
FILE *output_file;

int problem_number;

int main(int argc, char *argv[]){
    if(argc < 2){
        //indicate correct program usage
        printf("usage: p1 <input_filename>\n");
        exit(1);
    }
    input_filename = argv[1];
    if(input_file = fopen(input_filename, "r")){
        output_filename = "output.txt";
        output_file = fopen(output_filename, "w");

        off_t input_filesize;

        //determine size of input file
        if(fseeko(input_file, 0L, SEEK_END) == -1){
            perror("<fseek() failed unexpectedly");
            exit(1);
        }

        input_filesize = ftello(input_file);
        if(input_filesize == -1){
            perror("<ftello() failed unexpectedly>");
            exit(1);
        }

        if(fseeko(input_file, 0L, SEEK_SET) == -1){
            perror("<fseek() failed unexpectedly>");
            exit(1);
        };
   
        //allocate space for input
        input = (char*)malloc(input_filesize*sizeof(char)+1);
        if(input == NULL){
            perror("<malloc() failed unexpectedly>");
            exit(1);
        }
 
        //read-in input
        int nread;
 
        nread = fread(input, sizeof(char), input_filesize, input_file);
        if(nread != input_filesize){
            perror("<fread() failed unexpectedly>");
            exit(1);
        }       
        input[input_filesize]='\0';

        //process input
        char *token;
        char *trunksave;

        int num_cities; 
        int num_paths;
 
        char *token_2;
        char *branchsave;

        int source;
        int destination;
        int cost;

        token = strtok_r(input, "\n", &trunksave);
        problem_number = 1;
        num_cities = strtol(token, NULL, 10);       

        int index;
        int matrix[num_cities][num_cities];
        num_paths = num_cities*num_cities;
        for(index = 0; index < num_paths; index++){
            token = strtok_r(NULL, "\n", &trunksave);

            token_2 = strtok_r(token, " ", &branchsave);
            source = strtol(token_2, NULL, 10);
            token_2 = strtok_r(NULL, " ", &branchsave);
            destination = strtol(token_2, NULL, 10);
            token_2 = strtok_r(NULL, " ", &branchsave);
            cost = strtol(token_2, NULL, 10);

            matrix[source][destination] = cost;
        }    
        initializeTravel(num_cities, matrix, output_file); 
                                        
        while(token = strtok_r(NULL, "\n", &trunksave)){
            problem_number++;
            num_cities = strtol(token, NULL, 10);     

            int index;
            int matrix[num_cities][num_cities];
            num_paths = num_cities*num_cities;
            for(index = 0; index < num_paths; index++){
                token = strtok_r(NULL, "\n", &trunksave);

                int source;
                int destination;
                int cost;

                token_2 = strtok_r(token, " ", &branchsave);
                source = strtol(token_2, NULL, 10);
                token_2 = strtok_r(NULL, " ", &branchsave);
                destination = strtol(token_2, NULL, 10);
                token_2 = strtok_r(NULL, " ", &branchsave);
                cost = strtol(token_2, NULL, 10);

                matrix[source][destination] = cost;                

            }    
            initializeTravel(num_cities, matrix, output_file);  
        }
                
        //cleanup  
        free(input);
        fclose(output_file);
        fclose(input_file);
        return 0;
    }

    //failed to open necessary input file
    perror("<fopen() failed unexpectedly>");
    return 1;
}

void initializeTravel(int N, int matrix[N][N], FILE *output){
    int array[N];
    
    int index;
    for(index = 0; index < N; index++){
        array[index] = index; 
    } 
 
    int minCost = INT_MAX;

    int minTour[N];
    for(index = 0; index < N; index++){
        minTour[index] = 0;
    }

    int nodeCount = 0;

    time_t start = time(NULL);
    travel(N, matrix, &minCost, &nodeCount, minTour);
    time_t end = time(NULL);

    int time = (int)(end - start);    


    /* write-out output */
    fprintf(output, "%d %d %d %d %d\n", problem_number, N, minCost, nodeCount, time);
    if(N <= 14){
        for(index = 0; index < N; index++){
            fprintf(output, "%d\n", minTour[index]);
        }
    }
}

void travel(int N, int matrix[N][N], int *minCost, int *nodeCount, int minTour[N]){
    node_t **priority_queue;
    int priority_queue_capacity;
    int priority_queue_size;

    priority_queue = (node_t **)malloc(INIT_PQ_CAPACITY * sizeof(node_t *));
    priority_queue_capacity = INIT_PQ_CAPACITY;
    priority_queue_size = 0;

    int *init_path;
    int init_path_length;

    init_path = (int *)malloc(N*sizeof(int *));
    if(init_path == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    init_path[0] = 0;
    init_path_length = 1; 
    
    node_t *v = new_node();
    set_node_level(v, 0); 
    set_node_path(v, N, init_path, init_path_length);
    set_node_bound(v, N, matrix, v->path, v->path_length); 
    
    int minLength = INT_MAX;    

    pq_insert(priority_queue, &priority_queue_capacity, &priority_queue_size, v);
    while(priority_queue_size){
        node_t *v = pq_remove(priority_queue, &priority_queue_size);
        if(v->bound < minLength){
            node_t *u = new_node();
            set_node_level(u, (v->level)+1);

            int i, j;
            for(i = 1; i < N; i++){
                if(!is_member(i, v->path, v->path_length)){
                    set_node_path(u, N, v->path, v->path_length);
                    u->path[u->path_length] = i;
                    u->path_length++;
                    if(u->level == N-2){
                        for(j = 1; j < N; j++){
                            if(!is_member(j, v->path, v->path_length)){
                                u->path[u->path_length] = j;
                                u->path_length++;
                                break;
                            }
                        }
                        u->path[u->path_length] = 1;
                        u->path_length++;
                        
                        int l = length(N, matrix, u->path, u->path_length);
                        if(l < minLength){
                            minLength = l;
                            for(j = 0; j < N; j++){
                                minTour[j] = u->path[i];
                            }
                        }

                    }
                    else{
                        set_node_bound(u, N, matrix, u->path, u->path_length);
                        if(u->bound < minLength){
                            pq_insert(priority_queue, &priority_queue_capacity, &priority_queue_size, u);
                        }
                    }
                }
            } 
            
        }  
    }
    
}

node_t *new_node(){
    node_t *node;
    node = (node_t *)malloc(sizeof(node_t *));
    if(node == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    return node;    
}

void set_node_level(node_t *node, int level){
    node->level = level;
}

void set_node_path(node_t *node, int N, int *path, int path_length){
    node->path = (int *)malloc((N+1)*sizeof(int *));
    if(node->path == NULL){
        perror("<malloc() failed unexpectedly>");
        exit(1);
    }
    
    int i;
    for(i = 0; i < path_length; i++){
        node->path[i] = path[i];
    }
    node->path_length = path_length;
}

/* work on set_bound */
void set_node_bound(node_t *node, int N, int matrix[N][N], int *path, int path_length){
   int bound = 0;

    bound = bound + length(N, matrix, path, path_length);
   
    int end_node = path_length-1;

    int min, val;
    int i, j;

    min = INT_MAX;
    for(i = 0; i < N; i++){
        if(!is_member(i, path, path_length)){
            val = matrix[end_node][i];
            if((val != 0) && (val < min)){
                min = val;
            }
        }
    }
    bound = bound + min;

    for(j = end_node + 1; j < N; j++){ 
        min = INT_MAX;
        for(i = 0; i < N; i++){
            if(!is_member(i, path, path_length) || (i == 1)){
                val = matrix[j][i];
                if((val != 0) && (val < min)){
                    min = val;
                }
            }
        }
        bound = bound + min;
    }
    node->bound = bound;
}





/*void perm(int start, int N, int matrix[N][N], int array[N], int *minCost, int *maxCost, int minTour[N]){
    if(start < N-1){       
        int index;
        for(index = start; index < N; index++){
            swap(N, array, start, index);
            perm(start+1, N, matrix, array, minCost, maxCost, minTour);
            swap(N, array, start, index);
        }
    }
    else {
        int cost = count(N, array, matrix);
  
        if (cost < (*minCost)){
            *minCost = cost; 

            int index;
            for(index = 0; index < N; index++){
                minTour[index] = array[index];
            }
        }
        if (cost > (*maxCost)){
            *maxCost = cost;
        }
    }
}*/

/* Priority Queue */

void pq_insert(node_t **priority_queue, int *priority_queue_capacity, int *priority_queue_size, node_t *node){
    if(priority_queue_capacity == priority_queue_size){
        priority_queue = (node_t **)realloc(priority_queue, (*priority_queue_capacity)*2);
        (*priority_queue_capacity) = (*priority_queue_capacity)*2;
    }
    priority_queue[(*priority_queue_size)] = node;
    (*priority_queue_size) = (*priority_queue_size)+1;
  
    int j;
    for(j = 1; j < (*priority_queue_size); j++){
        node_t *key = priority_queue[j];
        int i = j - 1;
        while(i >= 0 && priority_queue[i]->bound > key->bound){
            priority_queue[i+1] = priority_queue[i];
            i--;
        }
        priority_queue[i+1] = key;
    }
}

node_t *pq_remove(node_t **priority_queue, int *priority_queue_size){
    node_t *ret = priority_queue[0];
     
    int i;
    for(i = 1; i < (*priority_queue_size); i++){
        priority_queue[i-1] = priority_queue[i];
    }

    return ret;
}

/*void remove(){

}*/

int length(int N, int matrix[N][N], int *vector, int vector_size){
    int length = 0;
    if(vector_size == 1){
        return length;
    } 
   
    int index;
    for(index = 0; index < vector_size-1; index++){
        int X = vector[index];
        int Y = vector[index+1];
        length += matrix[X][Y];
    }
    return length; 
}

int is_member(int item, int *vector, int vector_size){
    int is_member = 0;
    
    int i;
    for(i = 0; i < vector_size; i++){
        if(item == vector[i]){
            is_member = 1;
            break;
        }
    }
    return is_member;
}

void printArray(int N, int array[N]){
    int i;
    for(i = 0; i < N; i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

void printGraph(int N, int graph[N][N]){
    int i, j;
    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            printf("%d ", graph[i][j]);    
        }
        printf("\n");
    }
}
