/**
 * @file readmap.c
 * @date 27 Jan 2017
 * @brief Read map program documentation.
 *
 * Here goes the description.
 */


int get_node ( char* temp, int len, char* buff, char* name,\
               double* lat, double* lon, int nodecount){
/** get_node
 * Nodes parser function. Stores the id, name, lattitude and longitude
 * in the corresponding array.
 *
 * NODE format:  node|@id|@name|@place|@highway|@route|@ref|@oneway|@maxspeed|node_lat|node_lon 
 */
    int id;
    int barnum=0;
    int pos=0;
    for (int i=0;i<len; i++){
        buff[pos]=temp[i];
        pos++;
        if (temp[i]=='|'){
            barnum++;
            buff[pos]='\0';
            switch (barnum){
                case 2:
                    sscanf(buff, "%d", &id); 
                    break;
                case 3:
                    buff[pos-1]='\0';
                    strcpy(name, buff); 
                    break;
                case 10:
                    sscanf(buff, "%lf", &lat[nodecount]); 
                    break;
            }
            pos=0;
        }
    }
    sscanf(buff, "%lf", &lon[nodecount]); 
    return id;
}

int get_node_by_id(int id, int* nodes, int Nnodes){
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


int do_way( char*temp, int len, char* buff, int* nodes,\
            llist** children, int Nnodes){
/** do_way 
 * Ways parser function. Locates and stores the edges, using the adjacency list
 * graph representation.
 *
 * WAY format: way|@id|@name|@place|@highway|@route|@ref|@oneway|@maxspeed|membernode|...
 */
    char oneway[100];
    int barnum=0;
    int pos=0;
    int id1,id2,node1,node2;
    char TWOWAY=1;
    char priority=0; // disable priotity list functionality
    for (int i=0;i<len; i++){
        buff[pos]=temp[i];
        pos++;
        if (temp[i]=='|'){
            barnum++;
            buff[pos]='\0';
            pos=0;
            switch (barnum){
                case 8:
                    strcpy(oneway, buff); 
                    if (strcmp("|",oneway)) TWOWAY=0;
                    break;
                case 10:
                    sscanf(buff, "%d", &id1);
                    node1= get_node_by_id(id1,nodes,Nnodes);
                    break;
            }
            if (barnum>10){
                sscanf(buff, "%d", &id2);
                node2= get_node_by_id(id2,nodes,Nnodes);
                if ((node1 != -1) && (node2 != -1)) {
                    ll_insert(children[node1],node2,priority);
                    if (TWOWAY) ll_insert(children[node2],node1,priority);
                }
                node1=node2;
                id1=id2;
            }
        }
    }
    if (barnum>=10){
        sscanf(buff, "%d", &id2);
       //puts("last!");
        node2 = get_node_by_id(id2,nodes,Nnodes);
      //printf("%d -> %d\n",node1,node2);
        if ((node1 != -1) && (node2 != -1)) {
            ll_insert(children[node1],node2,priority);
            if (TWOWAY) ll_insert(children[node2],node1,priority);
        }
    }
}


int main(int argc, char* argv[]){
/** int readmap() {}
 *
 * The main function of the reader program call the parsers in order to process
 * the file, stores the children all together in a big successor array and 
 * writes all the stuff to the file named "map.hex".
 */

    char temp[100000];
    char node[] = "node";
    char way[] = "way";
    char relation[] = "relation";

    if (argc<3) {
        fprintf(stderr, "usage:\n./readmap file #nodes #ways [Nmax]\n");
        exit(1);
    }

    FILE* fp; //fingers crossed ! ツ
    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr,"the input text file cannot be opened\n");
        exit(31);
    } 
    puts("fp loads to  not null");

    int Nnodes=atoi(argv[2]);
    printf("read number of nodes %d\n",Nnodes);
    int Nways=atoi(argv[3]);
    printf("read number of ways %d\n",Nways);
    
    int Nmax=(argc>4?atoi(argv[4]):INT_MAX);
    printf("read max iterations: %d\n",Nmax);

    int* nodes=malloc(sizeof(int)*Nnodes); //array containing node id's
    if (nodes==NULL) {
        fprintf(stderr,"when allocating nodes\n");
        exit(1);
    } 
    printf("declared nodes\n");
    double* lat=malloc(sizeof(double)*Nnodes);
    if (lat==NULL) {
        fprintf(stderr,"when allocating lattitudes\n");
        exit(1);
    } 
    printf("declared lattitudes\n");
    double* lon=malloc(sizeof(double)*Nnodes);
    if (lat==NULL) {
        fprintf(stderr,"when allocating longitudes\n");
        exit(1);
    } 
    printf("declared longitudes\n");
    char** names = malloc(sizeof(char*)*Nnodes);
    puts("allocated names pointers");
    llist** children = malloc(sizeof(llist*)*Nnodes);
    if (children==NULL) {
        fprintf(stderr,"when allocating children pointers\n");
        exit(1);
    } 
    for (int i=0; i<Nnodes; i++)
        children[i]=ll_create(-1);
    puts("children initialized");

    char buff[1000];
    int id,minid=INT_MAX,maxid=0;
    int N=0;
    int nodecount=0,waycount=0;
    time_t t0=0,t1;

/** This read map program is in fact a state machine:
 *  it has three possible states which transition between each other by a
 *  cascading switch statement.
 *  The initial state INIT deals with reading the headers of the system.
 *  As soon as a node information line is found transition to the second state
 *  is made, NODE, this state deals with reading and storing path names, ID's,
 *  latitude and longitude.
 *  When the nodes have been read it is the turn of stablishing ways between 
 *  nodes, this is done in the WAY state.
 *  Finally, after the graph's edges are defined we proceed to save all data
 *  and exit, that is, we reach the final state EXIT.
 */
    enum {INIT,NODE,WAY,EXIT};
    char mode = INIT;
    puts("inside while");
    char signal_nodes=1;
    char signal_ways=1;
    while( N<Nmax && (fgets( temp, 80000, fp) != NULL)) { 
        N++;
        if ((temp[0]==node[0])&&(temp[1]==node[1])) {mode=NODE;signal_nodes?puts("text processing collects nodes now!"):0;signal_nodes=0;}
        else if ((temp[0]==way[0])&&(temp[1]==way[1])) {mode=WAY;signal_ways?puts("turn for ways!"):0;signal_ways=0;}
        else if ((temp[0]==relation[0])&&(temp[1]==relation[1])) {mode=EXIT;puts("read the whole file!");break;}
        int len = strlen(temp);

        switch (mode){
            case NODE:
                if (Nnodes > 50 && !(nodecount%(Nnodes/50)))
                    printf("progress %d / %d nodes (%d%%)\n",nodecount,Nnodes,(nodecount/(Nnodes/100)));
                names[nodecount] = malloc(184*sizeof(char));
                id = get_node( temp, len, buff, names[nodecount],
                               lat, lon, nodecount);
                maxid=(maxid<id)?id:maxid;
                minid=(minid>id)?id:minid;
                nodes[nodecount]=id;
                nodecount++;
                break;
            case WAY:
                if (Nways > 50 && !(waycount%(Nways/50)))
                    printf("progress %d / %d waycount (%d%%)\n",waycount,Nways,(waycount/(Nways/100)));
                waycount++;
                do_way(temp, len, buff, nodes, children, Nnodes);
                break;
        }
    }
    printf("%d\n",nodecount);
    printf("%d\n",waycount);
    printf("minid is %d, maxid is %d\n",minid,maxid);
   // for (int n=0;n<Nnodes;n++){
   //     printf("Id: %d\ngoing to:\n",nodes[n]);
   //     ll_print(children[n]);
   // }
///////////////////////////////////////////////////////////////////////////////

    //WRITE SUCCESSOR TABLE
    int* successors;
    int number_of_successors=0;
    int new_number_of_successors;
    int* ith_children_batch=malloc(Nnodes*sizeof(int));
    int* nsucs=malloc(Nnodes*sizeof(int));
    for (int i=0; i<Nnodes; i++){
        ith_children_batch[i]=number_of_successors;
        int n_children=ll_len(children[i]);
        nsucs[i]=n_children;
        new_number_of_successors=number_of_successors + n_children;
        successors = realloc( successors, new_number_of_successors*sizeof(float));
        for (int k=number_of_successors; k<new_number_of_successors; k++){
            successors[k]=ll_pop(&children[i]);   
        }
        free(children[i]);
        number_of_successors = new_number_of_successors;
    }
    free(children);
    
    printf("number_of_successors = %d\n",number_of_successors);
    //for (int i=0; i<number_of_successors;i++){
    //    printf("%d ",successors[i]);
    //} 
    puts("");

    printf("successors of %d are:  ",nodes[3]);
    for (int i=ith_children_batch[3]; i<ith_children_batch[3]+nsucs[3];i++){
        printf("%d ",successors[i]);
    } 
    puts("");
    printf("successors of %d are:  ",nodes[4]);
    for (int i=ith_children_batch[4]; i<ith_children_batch[4]+nsucs[4];i++){
        printf("%d ",successors[i]);
    } 
    puts("");

    fclose(fp);

///////////////////////////////////////////////////////////////////////////////
    puts("writing binary part now!!!");
    //write binary part!!!
    //what we need to write:
    // int Nnodes; the number of nodes
    // int nodes[Nnodes]; array containing node id's from OpenStreetMap
    // char names[Nnodes][184]; their names, if any
    // double lat[Nnodes]; lattitude
    // double lon[Nnodes]; longitude
    // int number_of_successors; the total amount of successors
    // int successors[number_of_successors]; the successors themselves
    // int nsucs[Nnodes]; the number of successors of each node
    // int ith_children_batch[Nnodes]; where the ith node has to look   
    //                               for its successors in the previous table


    FILE* fin;
    // keep in mind, writing is the easy part, is it not?
    if ((fin = fopen ("map.hex", "wb")) == NULL) {
        fprintf(stderr,"the output binary data file cannot be opened\n");
        exit(31);
    } 
    // int Nnodes; the number of nodes
    if( fwrite(&Nnodes, sizeof(int), 1, fin) != 1 ) {
        fprintf(stderr,"when initializing the output binary data file\n");
        exit(32);
    }

    // int nodes[Nnodes]; array containing node id's from OpenStreetMap
    int itms;
    if( (itms=fwrite(nodes, sizeof(int), Nnodes, fin)) != Nnodes ) {
        fprintf(stderr,"when writing node IDs to the output binary data file, %d written in total\n",itms);
        exit(32);
    }
    // char names[Nnodes][184]; their names, if any
    for ( int k=0; k<Nnodes;k++){
        if( fwrite(names[k], sizeof(char), 184, fin) != 184 ) {
            fprintf(stderr,"when writing node names to the output binary data file\n");
            exit(32);
        }
    }
    // double lat[Nnodes]; lattitude
    if( fwrite(lat, sizeof(double), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when writing lattitudes to the output binary data file\n");
        exit(32);
    }
    // double lon[Nnodes]; longitude
    if( fwrite(lon, sizeof(double), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when writing longitudes to the output binary data file\n");
        exit(32);
    }
    // int number_of_successors; the total amount of successors
    if( fwrite(&number_of_successors, sizeof(int), 1, fin) != 1 ) {
        fprintf(stderr,"when writing the number of successors to the output binary data file\n");
        exit(32);
    }
    // int successors[number_of_successors]; the successors themselves
    if( fwrite(successors, sizeof(int), number_of_successors, fin) != number_of_successors ) {
        fprintf(stderr,"when writing the successors table to the output binary data file\n");
        exit(32);
    }
    // int nsucs[Nnodes]; the number of successors of each node
    if( fwrite(nsucs, sizeof(int), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when writing the number of successors table to the output binary data file\n");
        exit(32);
    }
    // int ith_children_batch[Nnodes]; where the ith node has to look   
    if( fwrite(ith_children_batch, sizeof(int), Nnodes, fin) != Nnodes ) {
        fprintf(stderr,"when writing the positions of the successors table to the output binary data file\n");
        exit(32);
    }
    fclose(fin);

    puts("bikain");
}
