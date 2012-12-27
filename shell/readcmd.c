/*
 * Copyright (C) 2002, Simon Nieuviarts
 *               2008, Matthieu Moy
 *               2010, Grégory Mounié
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "readcmd.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>

#ifdef USE_GNU_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

static void memory_error(void)
{
	errno = ENOMEM;
	perror(0);
	exit(1);
}


static void *xmalloc(size_t size)
{
	void *p = malloc(size);
	if (!p) memory_error();
	return p;
}


static void *xrealloc(void *ptr, size_t size)
{
	void *p = realloc(ptr, size);
	if (!p) memory_error();
	return p;
}

#ifndef USE_GNU_READLINE
/* Read a line from standard input and put it in a char[] */
static char *readline(char *prompt)
{
	size_t buf_len = 16;
	char *buf = xmalloc(buf_len * sizeof(char));

	printf(prompt);
	if (fgets(buf, buf_len, stdin) == NULL) {
		free(buf);
		return NULL;
	}

	do {
		size_t l = strlen(buf);
		if ((l > 0) && (buf[l-1] == '\n')) {
			l--;
			buf[l] = 0;
			return buf;
		}
		if (buf_len >= (INT_MAX / 2)) memory_error();
		buf_len *= 2;
		buf = xrealloc(buf, buf_len * sizeof(char));
		if (fgets(buf + l, buf_len - l, stdin) == NULL) return buf;
	} while (1);
}
#endif

#define READ_CHAR *(*cur_buf)++ = *(*cur)++
#define SKIP_CHAR (*cur)++

static void read_single_quote(char ** cur, char ** cur_buf) {
	SKIP_CHAR;
	while(1) {
		char c = **cur;
		switch(c) {
                case '\'':
                        SKIP_CHAR;
                        return;
                case '\0':
                        fprintf(stderr, "Missing closing '\n");
                        return;
                default:
                        READ_CHAR;
                        break;
                }
	}
}

static void read_double_quote(char ** cur, char ** cur_buf) {
	SKIP_CHAR;
	while(1) {
		char c = **cur;
		switch(c) {
		case '"':
			SKIP_CHAR;
			return;
		case '\\':
			SKIP_CHAR;
			READ_CHAR;
			break;
                case '\0':
                        fprintf(stderr, "Missing closing \"\n");
                        return;
		default:
			READ_CHAR;
			break;
		}
	}
}

static void read_word(char ** cur, char ** cur_buf) {
	while(1) {
		char c = **cur;
		switch (c) {
		case '\0':
		case ' ':
		case '\t':
		case '<':
		case '>':
		case '|':	
			**cur_buf = '\0';
			return;
		case '\'':
			read_single_quote(cur, cur_buf);
			break;
		case '"':
			read_double_quote(cur, cur_buf);
			break;
		case '\\':
			SKIP_CHAR;
			READ_CHAR;
			break;
		default:
			READ_CHAR;
			break;
		}
	}
}

/* Split the string in words, according to the simple shell grammar. */
static char **split_in_words(char *line)
{
	char *cur = line;
	char *buf = malloc(strlen(line) + 1);
	char *cur_buf;
	char **tab = 0;
	size_t l = 0;
	char c;

	while ((c = *cur) != 0) {
		char *w = 0;
		switch (c) {
		case ' ':
		case '\t':
			/* Ignore any whitespace */
			cur++;
			break;
		case '&':
		        w = "&";
			cur++;
			break;
		case '<':
			w = "<";
			cur++;
			break;
		case '>':
			w = ">";
			cur++;
			break;
		case '|':
			w = "|";
			cur++;
			break;
			
		default:
			/* Another word */
			cur_buf = buf;
			read_word(&cur, &cur_buf);
			w = strdup(buf);
		}
		if (w) {
			tab = xrealloc(tab, (l + 1) * sizeof(char *));
			tab[l++] = w;
		}
	}
	tab = xrealloc(tab, (l + 1) * sizeof(char *));
	tab[l++] = 0;
	free(buf);
	return tab;
}


static void freeseq(char ***seq)
{
	int i, j;

	for (i=0; seq[i]!=0; i++) {
		char **cmd = seq[i];

		for (j=0; cmd[j]!=0; j++) free(cmd[j]);
		free(cmd);
	}
	free(seq);
}


/* Free the fields of the structure but not the structure itself */
static void freecmd(struct cmdline *s)
{
	if (s->in) free(s->in);
	if (s->out) free(s->out);
	if (s->seq) freeseq(s->seq);
}


struct cmdline *readcmd(char *prompt)
{
	static struct cmdline *static_cmdline = 0;
	struct cmdline *s = static_cmdline;
	char *line;
	char **words;
	int i;
	char *w;
	char **cmd;
	char ***seq;
	size_t cmd_len, seq_len;

	line = readline(prompt);
	if (line == NULL) {
		if (s) {
			freecmd(s);
			free(s);
		}
		return static_cmdline = 0;
	}
#ifdef USE_GNU_READLINE
	else 
	  add_history(line);
#endif

	cmd = xmalloc(sizeof(char *));
	cmd[0] = 0;
	cmd_len = 0;
	seq = xmalloc(sizeof(char **));
	seq[0] = 0;
	seq_len = 0;
	
	glob_t globbuf; 
	  
	words = split_in_words(line);
	free(line);

	if (!s)
		static_cmdline = s = xmalloc(sizeof(struct cmdline));
	else
		freecmd(s);
	s->err = 0;
	s->in = 0;
	s->out = 0;
	s->seq = 0;
	s->bg = 0;

	i = 0;
	while ((w = words[i++]) != 0) {
		switch (w[0]) {
		case '<':
			/* Tricky : the word can only be "<" */
			if (s->in) {
				s->err = "only one input file supported";
				goto error;
			}
			if (words[i] == 0) {
				s->err = "filename missing for input redirection";
				goto error;
			}
			s->in = words[i++];
			break;
		case '>':
			/* Tricky : the word can only be ">" */
			if (s->out) {
				s->err = "only one output file supported";
				goto error;
			}
			if (words[i] == 0) {
				s->err = "filename missing for output redirection";
				goto error;
			}
			s->out = words[i++];
			break;
		case '&':
			/* Tricky : the word can only be "&" */
			if (cmd_len == 0) {
				s->err = "misplaced ampersand";
				goto error;
			}
			if (s->bg == 1) {
				s->err = "only one ampersand supported";
				goto error;
			}
			s->bg = 1;
			break;
		case '|':
			/* Tricky : the word can only be "|" */
			if (cmd_len == 0) {
				s->err = "misplaced pipe";
				goto error;
			}

			seq = xrealloc(seq, (seq_len + 2) * sizeof(char **));
			seq[seq_len++] = cmd;
			seq[seq_len] = 0;

			cmd = xmalloc(sizeof(char *));
			cmd[0] = 0;
			cmd_len = 0;
			break;
		
		//--------------------Traitement des jokers---------------------//		

		default:
			 
			 //On utilise les options GLOB_TILDE et GLOB_BRACE pour gerer le tilde et les crochets, GLOB_NOCHECK permet d'avoir toujours un résultat//
			 if(glob(words[i-1],GLOB_TILDE|GLOB_BRACE|GLOB_NOCHECK,NULL,&globbuf)==0){	 
			 
			      int k;
			      for(k=0;k<globbuf.gl_pathc;k++) {
			      cmd = xrealloc(cmd, (cmd_len + 2) * sizeof(char *));
			      
			      cmd[cmd_len] = xmalloc( (strlen(globbuf.gl_pathv[k])+1) * sizeof(char));    
			      strcpy(cmd[cmd_len++],globbuf.gl_pathv[k]);	      
			      cmd[cmd_len] = 0;

			      }	
			 }			
							
			//Programme initial, à restaurer si necessaire//
			/*cmd = xrealloc(cmd, (cmd_len + 2) * sizeof(char *));
			cmd[cmd_len++] = w;
			cmd[cmd_len] = 0;*/
		}
	}

	if (cmd_len != 0) {
		seq = xrealloc(seq, (seq_len + 2) * sizeof(char **));
		seq[seq_len++] = cmd;
		seq[seq_len] = 0;
	} else if (seq_len != 0) {
		s->err = "misplaced pipe";
		i--;
		goto error;
	} else
		free(cmd);
	free(words);
	s->seq = seq;
	return s;
error:
	while ((w = words[i++]) != 0) {
		switch (w[0]) {
		case '<':
		case '>':
		case '|':
			break;
		default:
			free(w);
		}
	}
	free(words);
	freeseq(seq);
	for (i=0; cmd[i]!=0; i++) free(cmd[i]);
	free(cmd);
	if (s->in) {
		free(s->in);
		s->in = 0;
	}
	if (s->out) {
		free(s->out);
		s->out = 0;
	}
	return s;
}



/////////*Fonctions agissants sur la structure de liste *///////////////////

//Crée une liste vide//
Liste creer_liste(void){
  return NULL;
}


//Test si la liste est vide//
int est_vide(Liste L) { 
  return !L; 
 }
 
     
//Ajoute un élément en fin de liste //
Liste ajout_queue(char **cmd,int pidfils,int rang, Liste L) { 
  
  //Creation d'une nouvelle liste contenant les infos sur le processus en tache de fond//
  Liste tache=calloc(1,sizeof(*tache));
  tache->pid=pidfils;
  tache->rang=rang;
  tache->suiv=NULL;
  
  //Copie de la chaine de donnée cmd//
   tache->cmd = xmalloc(sizeof(char *));
   tache->cmd[0] = 0;
   size_t tache_len = 0;
	
   int i;
   for (i=0; cmd[i]!=0; i++) {
	tache->cmd = xrealloc(tache->cmd, (tache_len + 2) * sizeof(char *));
	tache->cmd[tache_len]= xmalloc( (strlen(cmd[i])+1) * sizeof(char)); 
	strcpy(tache->cmd[tache_len++],cmd[i]);	
	tache->cmd[tache_len] = 0;
   }	
  //On va placer la tâche en fin de liste
  if(est_vide(L)) { return tache;}
  else 
    {
      Liste Pe;
      for(Pe=L;!est_vide(Pe->suiv);Pe=Pe->suiv);
      Pe->suiv=tache;
      return L;
    }
}


//Visualise la liste des processus en tâche de fond//
void visualiser(Liste L)
{	int j;  
 
    if(L==NULL) { printf("Pas de processus en tâche de fond\n");}
    else{		        
	    
		
	Liste parcours=L;	       
	printf("Processus lancés en tâche de fond:\n");		    
	while(!est_vide(parcours)){	    			    
		printf("PID: %d    CMD:", parcours->pid);
		for (j=0; parcours->cmd[j]!=0; j++) {
                       printf("'%s' ", parcours->cmd[j]);
		}
		
		printf("\n");
		
		parcours=parcours->suiv;
		       		      
		
	 }
     }    	    
 
        
}


//Libere un élément de la liste//
void freeTache(Liste Tache){
	char ** delete=Tache->cmd;
	int i;
	char *suppr=delete[0];
	for(i=1; suppr!=0; i++) {
		free(suppr);
		suppr=delete[i];
	}
	
	free(delete);
}
	
