/*
 * $Log: tree.h,v $
 * Revision 1.1  2002-12-16 01:55:45+05:30  Manny
 * Initial revision
 *
 */
struct node
{
	void           *data;

	struct node    *l, *r;		/*- BST pointers */
	struct node    *next;		/*- Linked list for traverse etc */
};

enum llorder
{ O_NONE, O_IN, O_PRE, O_POST, O_LEVEL };

struct dict
{
	struct node    *head;
	enum llorder    order;
	int             (*compare) (void *a, void *b);
};

struct dict    *new_dict(int (*compare) (void *a, void *b));
void            free_dict(struct dict *);
int             add_unique_node(struct dict *, void *);
void            add_node(struct dict *, void *);
void           *change_node(struct dict *, void *);
void           *find_node(struct dict *, void *);
void           *delete_node(struct dict *, void *);
void            relink_inorder(struct dict *);
void            visit_nodes(struct dict *, void (*visit) (void *data));
void            balance_tree(struct dict *);
