/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


#include "exec_parser.h"

static so_exec_t *exec;
static int fd;


//initializare contor
int c = 0;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	//daca zonele data ale fiecarui segment nu au fost 
	//initializarea (c = 0), aloca meorie
	if(c == 0){
		for(int i = 0 ; i < exec->segments_no; i++){
			exec->segments[i].data = calloc(exec->segments[i].mem_size / getpagesize() + 1, sizeof(int));
			c = 1;
		}
	}

	//interez prin segmentele fisierului
	int i;
	for(i = 0; i < exec->segments_no; i++){
		
		//verificam daca pageFault este in segmentul 'i'
		if((int)info->si_addr >= exec->segments[i].vaddr && (int)info->si_addr < exec->segments[i].vaddr + exec->segments[i].mem_size){

		//salvam intr-o variabila lungimea unei pagini
		//si calculam a cata pagina in segment este aceasta
		int pageSize = getpagesize();
		int pageNr = (int)(info->si_addr - exec->segments[i].vaddr) / pageSize;

		//intr-un int buff copiam memoria de pe pozitia pageNr a
		//lui data si verificam daca este 0 (pagina nu a fost mapata)
		int buff;
		memmove(&buff, exec->segments[i].data + pageNr * sizeof(int), sizeof(int));
			if(buff == 0)
			{
				//mapam intreaga pagina
				mmap((void *)(exec->segments[i].vaddr + pageNr * pageSize), pageSize, PROT_WRITE | PROT_READ,
				MAP_FIXED | MAP_PRIVATE, fd, exec->segments[i].offset + pageNr * pageSize );
				
				//verificam daca file_size se termina in aceasta pagina
				if(exec->segments[i].vaddr + (pageNr + 1) * pageSize > exec->segments[i].vaddr +  exec->segments[i].file_size 
				&& exec->segments[i].vaddr + pageNr * pageSize <= exec->segments[i].vaddr + exec->segments[i].file_size)
				{
					//zeroim partea de pagina
					memset((void *)(exec->segments[i].vaddr + exec->segments[i].file_size), 0, (pageNr + 1) * pageSize - exec->segments[i].file_size);

				// verificam daca pagina este dupa file_size si zeroim	
				}
				else if(exec->segments[i].vaddr + pageNr * pageSize >= exec->segments[i].vaddr + exec->segments[i].file_size)
				{
					memset((void *)(exec->segments[i].vaddr + pageNr * pageSize), 0 , pageSize);

				}

				//protejam pagina
				mprotect((void *)(exec->segments[i].vaddr + pageNr * pageSize),  pageSize, exec->segments[i].perm);

				//setam zona de la pageNr cu 1 (pagina a fost mapayta)
				memset(exec->segments[i].data + pageNr * sizeof(int), 1, sizeof(int));
				return;
			}
			else if(buff == 1)
			{
				//daca pagina a fost deja mapata folosim
				//handlerul default
				signal(signum, SIG_DFL);
				raise(signum);
			}
		}
	}

	//daca adresa pageFault-ului nu se 
	//gaseste pe niciun segment folosim
	//handlerul default
	signal(signum, SIG_DFL);
	raise(signum);
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	fd = open(path, 0);
	
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
