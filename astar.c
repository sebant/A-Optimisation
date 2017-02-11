/**         A* Algorithm
 *
 * This file implements the A* algorithm that look for a path linking
 * nodes (start and goal) given as command  line arguments.
 * It has a main function that read the binary file map.hex containing the 
 * graph and calls "astar" to find the route.
 * If the path is found, then it prints it to stdin.
 */

enum {UNDEFINED,OPEN,CLOSED};
// Possible status for the nodes with respect Queue/OPEN list.

int get_node_by_id(int id, int* nodes, int Nnodes){
/**   int get_node_by_id(int id, int* nodes, int Nnodes)
 * 
 * Binary search algorithm thak looks for node id in the node array "nodes"
 * which has Nnodes elements.
 * Returns the correct id if found, -1 otherwise.
 */
    int start=-1,end=Nnodes;
    int pivot;
    while(start<end-1){
        pivot=(start+end)/2;
        if (id==nodes[pivot])
            return pivot;
        else if (id<nodes[pivot])
            end=pivot;
        else
            start=pivot;
    }
    return -1;
}

double astar( int node_start, int node_goal, int* prev,
           int Nnodes, // the number of nodes
           int nodes[Nnodes], // array containing node id's from OpenStreetMap
           char* names[], // their names, if any
           double lat[], // lattitude
           double lon[], // longitude
           int number_of_successor, // the total amount of successors
           int successors[], // the successors themselves
           int nsucs[], // the number of successors of each node
           int ith_children_batch[]){ // where the ith node has to look
/**   double astar()
 *
 * Function implementing the ASart or A* algorithm.
 * The goal node is denoted by node_goal and the source node is denoted by
 * node_start.
 * The OPEN list is maintained as a linked list, and the statuses, f's, and g's
 * are stored in separate arrays.
 * If it doesn't find the goal node exits with error, else it returns the 
 * path distance.
 */

    double* f=malloc(Nnodes*sizeof(double));
    if (f == NULL) {
        fprintf(stderr,"Couldn't allocate space for 'f()'.\n");
        exit(1);
    }
    for (int i=0; i<Nnodes;i++)
        f[i]=1<<31;
    double* g=malloc(Nnodes*sizeof(double));
    if (g == NULL) {
        fprintf(stderr,"Couldn't allocate space for 'g()'.\n");
        exit(1);
    }
    for (int i=0; i<Nnodes;i++)
        g[i]=1<<31;

    char* status=calloc(Nnodes,sizeof(char));
    if (g == NULL) {
        fprintf(stderr,"Couldn't allocate status array.\n");
        exit(1);
    }
    int node_current;
    llist* open_list=ll_create(node_start); // linked list OPEN list implementation
    if (g == NULL) {
        fprintf(stderr,"Couldn't create the OPEN (linked) list.\n");
        exit(1);
    }
    char priority=1; // 'priority' signals it is  priority list
    int opened_items=1;
    g[node_start]=0;
    prev[node_start]=node_start;

    int iterations=0;
    while (opened_items) { // the OPEN list is not empty
        iterations++;
        node_current = ll_pop(&open_list); 
        // Take from the open list the node node_current with the lowest
        // expected distance.
        opened_items--;
        if (Nnodes>=100 && !(iterations%(Nnodes/100))){
        // Signal progress every 1% of total nodes expanded:
            printf("%d/%d expanded\n",iterations,Nnodes);
            printf("up to now dist = %g;\
                    heuristic haversine = %g;\
                    heuristic equirectangular = %g\n",
                    g[node_current],
                    h(lat[node_current],lon[node_current],
                      lat[node_goal],lon[node_goal]),
                    d(lat[node_current],lon[node_current],
                      lat[node_goal],lon[node_goal]));
        }

        if (node_current == node_goal) break; // we have found the solution.
        for (int i=ith_children_batch[node_current];
                 i<ith_children_batch[node_current]+nsucs[node_current];
                 i++){
            // each node_successor of node_current
            int node_successor=successors[i];
            double successor_current_cost = g[node_current] +
                                            d(lat[node_current],lon[node_current],
                                              lat[node_successor],lon[node_successor]);

            if (status[node_successor] == OPEN) {
                // node_successor is in the OPEN list
                if (g[node_successor] <= successor_current_cost) continue;
            } else if (status[node_successor] == CLOSED) {
                if (g[node_successor] <= successor_current_cost) continue;
            }

            status[node_successor]=OPEN; 
            g[node_successor] = successor_current_cost;
            // Set g(node_successor) = successor_current_cost
            f[node_successor] = g[node_successor] + 1*
                                h(lat[node_successor],lon[node_successor],
                                  lat[node_goal],lon[node_goal]);
            ll_insert(open_list,node_successor,priority,f);
            opened_items++;
            prev[node_successor]=node_current;
            // Set the parent of node_successor to node_current
        }
        status[node_current]=CLOSED;
    }

    if (node_current != node_goal) {
        fprintf(stderr,"the OPEN list is empty\n");
        exit(1);
    } else {
        puts("we got there!");
        printf("dist = %gkm.\n",g[node_goal]);
    }
    return g[node_goal];
}


int main(int argc, char** argv){
    FILE* fin;
    // keep in mind, reading is the easy part, is it not?
    if ((fin = fopen ("map.hex", "rb")) == NULL) {
        fprintf(stderr,"the input binary data file cannot be opened\n");
        exit(31);
    }
    // int Nnodes; the number of nodes
    int Nnodes;
    if( fread(&Nnodes, sizeof(int), 1, fin) != 1 ) {
        fprintf(stderr,"when initializing the output binary data file\n");
        exit(32);
    }
    printf("number of nodes %d\n",Nnodes);
    // int nodes[Nnodes]; array containing node id's from OpenStreetMap
    int* nodes = malloc(sizeof(int)*Nnodes);
    if (nodes==NULL) {
        fprintf(stderr,"when allocating nodes\n");
        exit(1);
    } 
    puts("nodes' pointers allocated");
    if( fread(nodes, sizeof(int), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when reading nodes from the input binary data file\n");
        exit(32);
    }
    // char names[Nnodes][184]; their names, if any
    char** names = malloc(sizeof(char*)*Nnodes);
    if (names==NULL) {
        fprintf(stderr,"when allocating name slots\n");
        exit(1);
    } 
    puts("names' pointers allocated");
    for (int i=0; i<Nnodes; i++){
        if( (names[i] = calloc(184,sizeof(char))) == NULL ) {
            fprintf(stderr,"when allocating %dth name\n",i);
            exit(1);
        }
        if (Nnodes > 50 && !(i%(Nnodes/50)))
            printf("%d out of %d names initialized (%d%%)\n",i,Nnodes,(i/(Nnodes/100)));
    }
    for ( int k=0; k<Nnodes;k++){
        if( fread(names[k], sizeof(char), 184, fin) != 184 ) {
            fprintf(stderr,"when reading node names from the input binary data file\n");
            exit(32);
        }
        if (Nnodes > 50 && !(k%(Nnodes/50)))
            printf("%d out of %d names read (%d%%)\n",k,Nnodes,(k/(Nnodes/100)));
    }
    puts("names read well");
    // double lat[Nnodes]; lattitude
    double* lat=malloc(sizeof(double)*Nnodes);
    if (lat==NULL) {
        fprintf(stderr,"when allocating lattitudes\n");
        exit(1);
    } 
    puts("lattitudes' pointers allocated");
    if( fread(lat, sizeof(double), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when reading lattitudes to the output binary data file\n");
        exit(32);
    }
    // double lon[Nnodes]; longitude
    double* lon=malloc(sizeof(double)*Nnodes);
    if (lon==NULL) {
        fprintf(stderr,"when allocating longitudes\n");
        exit(1);
    } 
    puts("longitudes' pointers allocated");
    if( fread(lon, sizeof(double), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when reading longitudes to the output binary data file\n");
        exit(32);
    }
    // int number_of_successors; the total amount of successors
    int number_of_successors;
    if( fread(&number_of_successors, sizeof(int), 1, fin) != 1 ) {
        fprintf(stderr,"when reading the number of successors to the output binary data file\n");
        exit(32);
    }
    // int successors[number_of_successors]; the successors themselves
    int* successors=malloc(number_of_successors*sizeof(int));
    if (successors==NULL) {
        fprintf(stderr,"when allocating successors' pointers\n");
        exit(1);
    } 
    puts("successors table allocated");
    if( fread(successors, sizeof(int), number_of_successors, fin) != number_of_successors ) {
        fprintf(stderr,"when reading the successors table to the output binary data file\n");
        exit(32);
    }
    // int nsucs[Nnodes]; the number of successors of each node
    int* nsucs=malloc(sizeof(int)*Nnodes);
    if (nsucs==NULL) {
        fprintf(stderr,"when allocating number of successors' pointers\n");
        exit(1);
    } 
    puts("number of successors table allocated");
    if( fread(nsucs, sizeof(int), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when reading the number of successors table to the output binary data file\n");
        exit(32);
    }
    // int ith_children_batch[Nnodes]; where the ith node has to look
    int* ith_children_batch = malloc(sizeof(int)*Nnodes);
    if (nsucs==NULL) {
        fprintf(stderr,"when allocating children batch index table\n");
        exit(1);
    } 
    puts("children batch allocated");
    if( fread(ith_children_batch, sizeof(int), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when reading the positions of the successors table to the output binary data file\n");
        exit(32);
    }
    fclose(fin);
    puts("all read well!");

/** Done  with reading stuff, now it's time to process the graph:
 */

    int id_start=nodes[0];
    int id_goal=nodes[4];
    if (argc>2){
        id_start=atoi(argv[1]);
        id_goal=atoi(argv[2]);
    }
    int node_start =  get_node_by_id(id_start, nodes, Nnodes);
    int node_goal =  get_node_by_id(id_goal, nodes, Nnodes);
    printf("we are going from %d to %d, aka %d and %d\n",node_start,node_goal,id_start,id_goal);
    printf("(%g,%g) --->  (%g,%g)\n",lat[node_start],lon[node_start],lat[node_goal],lon[node_goal]);

    double tot_distance = -1.0;
    int* previous = calloc(Nnodes,sizeof(int));
    if( (node_start != -1) && (node_goal != -1)) {
    tot_distance =
    astar( node_start, node_goal, previous,
           Nnodes, //the number of nodes
           nodes, //array containing node id's from OpenStreetMap
           (char**) names, // their names, if any
           lat, //; lattitude
           lon, //; longitude
           number_of_successors, //; the total amount of successors
           successors, //; the successors themselves
           nsucs, //; the number of successors of each node
           ith_children_batch); //where the ith node has to look
    }

    free(successors); //; the successors themselves
    free(nsucs); //; the number of successors of each node
    free(ith_children_batch); //where the ith node has to look

    puts("way was:");
    int node=node_goal;
    printf("reached %d (%g,%g), %s\n",nodes[node],lat[node],lon[node],names[node]);
    node=previous[node];
    while (previous[node]!=node){
        printf("through %d (%g,%g), %s\n",nodes[node],lat[node],lon[node],names[node]);
        node=previous[node];
    }
    printf("from %d (%g,%g), %s\n",nodes[node],lat[node],lon[node],names[node]);
    
    printf("dist = %gkm.\n",tot_distance);


// finally some cleaning is required, zer esango luke zure amak?
    free(nodes);
    free(lat);
    free(lon);
    free(previous);
    puts("now we clear names");
    for(int i=0; i<Nnodes;i++) {
        free(names[i]);
        if (Nnodes > 50 && !(i%(Nnodes/50)))
            printf("%d out of %d names freed (%d%%)\n",i,Nnodes,(i/(Nnodes/100)));
    }
    free(names);
    puts("all right, I'm going to sleep now!ツ");
}
