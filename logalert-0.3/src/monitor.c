
#include "monitor.h"

// tail logic comes from core-utils - tail.c
#define IS_TAILABLE_FILE_TYPE(Mode) \
	  (S_ISREG (Mode) || S_ISFIFO (Mode) || S_ISCHR (Mode))


unsigned int active = YES;

struct File_spec
{
  char *name;
  int fd;
  uint readall;
  off_t size;
  dev_t dev;
  ino_t ino;
  long lastchange;
};


void
match_sleep(unsigned int tsleep)
{
        active = NO;
        alarm(tsleep);
}

void
recompile(struct entry_conf *p)
{

	const char *error;
	int erroroff;

#ifdef HAVE_PCRE_H
	p->regex = pcre_compile(p->pattern,0,&error,&erroroff,NULL);
	if(!p->regex) {
	     fprintf(stderr,"error in regex: /%s/ ---> '%s`\n",p->pattern,error);
	     exit(1);
	}
#else
	p->regex = (regex_t *)malloc(sizeof(regex_t));
	if(regcomp(p->regex, p->pattern, REG_EXTENDED) != 0) {
	     fprintf(stderr,"error in regex: /%s/\n",p->pattern);
	     exit(1);
	}
#endif

}

int
rematch(struct entry_conf *cur_cfg, char *line)
{
#ifdef HAVE_PCRE_H
	return pcre_exec(cur_cfg->regex,NULL,line,strlen(line),0,0,cur_cfg->rg_sub,MAXRGSUB);
#else
        return (regexec(cur_cfg->regex, line, 0, NULL, REG_NOTEOL) == 0 ) ? 1 : -1;
#endif
}

void
recheck(struct File_spec *fs)
{
    struct stat new_stats;
    int fd;
    //int is_stdin = (EQS (fs->name, "-"));
    uint new_file = 0;
    uint fail = 0;


    //fd = (is_stdin ? STDIN_FILENO : open_file_ro (fs->name));
    fd = open_file_ro (fs->name);

    //if (fd == -1 || fstat (fd, &new_stats) < 0) {
    if (fd < 0 || fstat (fd, &new_stats) < 0) {
	    perror(fs->name);
	    fail = 1;
    }
    if (!IS_TAILABLE_FILE_TYPE (new_stats.st_mode)) {
	    fail = 1;
    }

    if(fail) {
	    close(fd);
	    close(fs->fd);
	    fd = -1;
    }
    else if (fs->ino != new_stats.st_ino || fs->dev != new_stats.st_dev) {
	    new_file = 1;
    } else {
	    if (fs->fd == -1)
		    new_file = 1;
	    else 
		    close(fd);
    }

    if (new_file) {
	    fs->fd = fd;
	    fs->size = 0; /* Start at the beginning of the file...  */
	    if(!fs->readall) { //or not
		    fs->readall++;
		    fs->size = new_stats.st_size;
	    }
	    fs->dev = new_stats.st_dev;
	    fs->ino = new_stats.st_ino;
	    lseek (fs->fd, fs->size, SEEK_SET);
    }


}


ssize_t
read_data(int fd, struct entry_conf *cur_cfg)
{

	char buff[MAXBUFFSIZE];
	unsigned int bsize = MAXBUFFSIZE;
	ssize_t bread,all = 0;

	while(1) {

		bread = read(fd,&buff,bsize);

		if(bread == -1) {
			debug("Error while reading file");
			break;
		}
		if(bread == 0)
			break;
 
		if(active) {
			int nmatch;
			nmatch = rematch(cur_cfg,(char *)&buff);
			if (cur_cfg->matchcount)
				cur_cfg->matchcount--;
			if(nmatch >= 0 && cur_cfg->matchcount == 0) {
				shell_exec(cur_cfg,buff,nmatch);
				if (cur_cfg->matchsleep)
		        	match_sleep(cur_cfg->matchsleep);
			}
		}

		if(cur_cfg->verbose)
			mywrite(STDOUT_FILENO,(char *) &buff, bread);
		all += bread;
	}
	return all;
}


int 
monitor_file(struct entry_conf *cur_cfg)
{

    int retry = 0;
    struct File_spec fs;
    struct stat fst;
    int is_stdin = (EQS (cur_cfg->watchfile, "-"));


    fs.name = cur_cfg->watchfile;
    fs.readall =  cur_cfg->readall;
    
    recompile(cur_cfg);

    if(is_stdin) {
	    fs.fd = STDIN_FILENO;
	    printf("stdin!\n");
	    do {
       	    	fs.size = read_data(fs.fd,cur_cfg);
	    } while(fs.size != 0);
	    return 1;
    }


    fs.fd = -1;

    while(retry < cur_cfg->retry) {


	if(fs.fd < 0) {
		retry++;
		recheck(&fs);
		continue;
	}

        if(stat(cur_cfg->watchfile,&fst) < 0) {
		fs.fd = -1;
		continue;
        }


	if(fst.st_size < fs.size) { //truncated
		debug("file %s truncated\n",cur_cfg->watchfile);
		lseek(fs.fd, (off_t) fst.st_size, SEEK_SET);
		fs.size = fst.st_size;
		fs.lastchange = fst.st_mtime;
	} else if (fst.st_size > fs.size) {

		fs.size += read_data(fs.fd,cur_cfg);
		if(fs.size < 0) {
			retry++;
			recheck(&fs);
			continue;
		}
		fs.size = fst.st_size;
		fs.lastchange = fst.st_mtime;

	} else {
            sleep(1);
        }
    }
    fprintf(stderr,"[!] MAXRETRY[%d] reached while trying to monitor %s - sorry, exiting\n", cur_cfg->retry,cur_cfg->watchfile);
}

