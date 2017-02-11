typedef struct llist_s  llist;

llist* ll_create(int car);

void ll_print(llist* list);


int ll_len(llist* the_list);

void ll_append(llist* origlist, int elem);

void ll_insert(llist* list, int elem, char priority, ...);

llist* ll_behead(llist* nlist);

int ll_pop(llist** pt2list);

int ll_car(llist* nlist);

llist* ll_cdr(llist* nlist);
