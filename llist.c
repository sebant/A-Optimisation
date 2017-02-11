typedef struct llist_s {
  int car;
  void *cdr;
} llist;

/**
 * Dummy linked list utility functions to be used as auxiliar by the
 * main program.
 *
 *
 */


llist* ll_create(int car){
    llist *list;      
    int n=5;
    list = malloc(sizeof(llist)); 
    if (list == NULL) {
        fprintf(stderr,"Couldn't allocate list\n");
        exit(1);
    }
    list->car = car;  
    list->cdr = 0;  
    return list;
}

void ll_print(llist* list){
    printf("( ");
    if (list->car!=-1) {
        llist* ho = list;
        while (ho){
            printf("%d, ",ho->car);
            ho=(llist*)(ho->cdr);
        }
        printf("\b");
    }
    printf("\b)\n");
}


int ll_len(llist* list){
    int lenght=0;
    if (list->car!=-1) {
        lenght++;
        llist * ho = list; 
        while (ho->cdr){
            ho=(llist*)(ho->cdr);
            lenght++;
        }
    }
    return lenght;
}

void ll_append(llist* origlist, int elem){
    llist* newlist = ll_create(elem);
    llist* ho = origlist;
    while (ho->cdr){
        ho=(llist*)(ho->cdr);
    }
    ho->cdr=newlist;
}

void ll_insert(llist* list, int elem, char priority, ...){
    llist* newlist;
    va_list ap;
    va_start(ap, priority);
    if (priority==1) {
        //puts("a priority list");
        double*p=va_arg(ap,double*);
        if (p[elem] == p[list->car]){
            ;
        } else if (list->car==-1) {
            list->car=elem;
        } else if (p[elem] < p[list->car]){
            newlist = ll_create(list->car);
            list->car = elem;
            newlist->cdr = list->cdr;
            list->cdr = newlist;
        } else {
            llist* ho = list;
            while ((ho->cdr) && (p[elem] > p[((llist*)(ho->cdr))->car])){
                ho=(llist*)(ho->cdr);
            }
            if ((ho->cdr) && p[elem] == p[((llist*)(ho->cdr))->car]){
                ;
            } else {
                newlist = ll_create(elem);
                newlist->cdr = ho->cdr;
                ho->cdr = newlist; 
            }
        }
    } else if (priority==0){
        if (elem == list->car){
            ;
        } else if (list->car==-1) {
            list->car=elem;
        } else {
            newlist = ll_create(list->car);
            list->car = elem;
            newlist->cdr = list->cdr;
            list->cdr = newlist;
        }
    }
    va_end(ap);
}

llist* ll_behead(llist* list){
    llist* foo = list;
    list = list->cdr;
    free(foo);
    return list;
}

int ll_pop(llist** pt2list){
    int car = (*pt2list)->car;
    if ((*pt2list)->cdr != NULL) {
        llist* foo = (*pt2list);
        (*pt2list) = (llist*)((*pt2list)->cdr);
        free(foo);
    } else {
        (*pt2list)->car=-1;
    }
    return car;
}

int ll_car(llist* list){
    return list->car;
}

llist* ll_cdr(llist* list){
    return (llist*) list->cdr;
}
