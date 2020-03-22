/*
 * $Log: evaluate.h,v $
 * Revision 2.1  2004-03-22 23:51:58+05:30  Cprogrammer
 * header file for evaluate() function
 *
 */
#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#ifndef	lint
static char     sccsidevalh[] = "$Id: evaluate.h,v 2.1 2004-03-22 23:51:58+05:30 Cprogrammer Stab mbhangui $";
#endif

#define T_INT    0
#define T_REAL   1

/*- error code */
#define RESULT_OK               0	/*- all OK                       */
#define ERROR_SYNTAX            2	/*- invalid expression           */
#define ERROR_VARNOTFOUND       3	/*- variable not found           */
#define ERROR_NOMEM             8	/*- not enough memory available  */
#define ERROR_DIV0              9	/*- division by zero             */

/*- private memory header for tracked memory allocation */
struct memh
{
	struct memh    *next;
	void           *ptr;
};


/*- value */
struct val
{
	long            ival;		/*- if type = T_INT, this is the result */
	double          rval;		/*- if type = T_REAL, this is the result */
	char            type;		/*- either T_INT or T_REAL */
};

/*- variable */
struct var
{
	struct var     *next;		/*- next variable in table or NULL */
	struct val      val;		/*- value of variable */
	char           *name;		/*- name of variable */
};

/*- variable table */
struct vartable
{
	struct var     *first;		/*- first entry in variable table */
	struct memh    *mh;
};


/*- creates a new variable table (NULL if no memory) */
struct vartable *create_vartable();
/*- frees a variable table */
void            free_vartable(struct vartable *vt);
/*
 * given a string to evaluate (not NULL), a result to put the answer in
 * (not NULL) and optionally your own variable table (NULL for 'internal
 * only' vartable), will return an error code (and result, etc)
 */
int             evaluate(char *eval, struct val *result, struct vartable *variables);
#endif
