#include "utils.h"

void *
xmalloc (size_t size)
{
  register void *value = malloc (size);
  if (value == 0) {
    fprintf(stderr,"[*]xmalloc: virtual memory exhausted");
    exit(1);
  }
  return value;
}

void *
xrealloc (void *ptr, size_t size)
{
  register void *value = realloc (ptr, size);
  if (value == 0) {
    fprintf(stderr,"[*]xrealloc: virtual memory exhausted");
    exit(1);
  }
  return value;
}

int open_file(char *file, int flags,mode_t *mode) {

    return open(file,flags,mode);
}

int open_file_ro(char *file) {

     return open_file(file,O_RDONLY,0);

}


char * getword(char *line) {

       return strtok(line,DELIMITERS);

}

void
mywrite (int fd, char const *buffer, size_t n_bytes)
{
	if (n_bytes > 0 && fwrite (buffer, 1, n_bytes, stdout) == 0)
		fatal("could not write to stdout");
}


int getwords(char *line, char **vec) {

        char *p,*q;
	char *key = NULL;
	char *value = NULL;
        p = line;

	while((*p) != '\0') {

	   if(isspace(*p)) {
		if(key)
			break;
		p++;
	   } else {
	   	if(!key) 
		   key = p;
		p++;	   
	   }
	}

	vec[0] = key;
	if((*p) == '\0')
		if(!key)
			return 0;
		else
			return 1;

	(*p++) = '\0';

	while((*p) != '\0') {

           if(isspace(*p) || (*p) == '=') {
		p++;
	   } else {
		value = p;
		break;
	   }
	}

	vec[1] = value;
	if(value)
		return 2;

}

        
        

/*
mygetline(int fd)
reads fd until it finds '\n' char. 
Allocates buffer as needed, returning the line.
Caller must free it after use.
*/

int
mygetline(int fd, char **buff)
{

       ssize_t r = 0;
       char c;
       ssize_t buffsize = 0;
       int ccount = 0;
       char *p;

       REFILLME:

       buffsize = buffsize + MALLOC_UNIT;

       if(*buff == NULL) {
          p = (char *)xmalloc(buffsize*sizeof(char *)); 
	  *buff = p;
       } else {
          xrealloc(*buff,buffsize*sizeof(char *));
       }
     
       for(;;) {

          if(ccount > buffsize - 1)
               goto REFILLME;

          r = read(fd,&c,1);

          if(r < 0) {
		perror("read");
		goto OUT;
          }
          if( (r == 0) || (c == '\n') )
		goto OUT;
          
          ccount++;
          *p++ = c;

       } 

	OUT:

       *p = '\0';

       return r;

}
/*
dupArray(char **src)

Returns a copy of **src. Finds out the size of src based on last value
being NULL.
*/
char **
dupArray(char **src)
{

	char **dst;

	unsigned int i,asize = 0;

	while(*(src+asize))
		asize++;
	dst = (char **)xmalloc((asize+2)*sizeof(char *));
	for(i=0;i<asize;i++) {
		dst[i+1] = (char *)xmalloc(strlen(src[i])+1);
		strcpy(dst[i+1],src[i]);
	}
	dst[i+1] = NULL;

	return dst;
}
