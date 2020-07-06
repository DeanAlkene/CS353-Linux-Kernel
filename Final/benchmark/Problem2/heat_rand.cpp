#include <omp.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <errno.h>
#include <hbwmalloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

using namespace std;

#define PAGE_NUM 1000000
struct heat
{
    unsigned long addr;
    int access_time;
};

struct heat read_info[PAGE_NUM + 1];

// static int NX = 1024*8;
// static int NY = 1024*4;
// static int NT = 200;

// 1024 * 4 * 64 = 1024 * 4 * 8 byte = 32KB = 8 pages
// Every thread handle 8 * 8 chunks -> 128 threads
#define NX 1024*8
#define NY 1024*4
#define NT 100

#define f(x,y)     (sin(x)*sin(y))
#define randa(x,t) (0.0)
#define randb(x,t) (exp(-2*(t))*sin(x))
#define randc(y,t) (0.0)
#define randd(y,t) (exp(-2*(t))*sin(y))
#define solu(x,y,t) (exp(-2*(t))*sin(x)*sin(y))


int nx, ny, nt;
double xu, xo, yu, yo, tu, to;
double dx, dy, dt;

double dtdxsq, dtdysq;
double t;

int leafmaxcol;

void swap_ranks(double ***from_ranks, double ***to_ranks) {
  double **tmp_ranks = *from_ranks;
  *from_ranks = *to_ranks;
  *to_ranks = tmp_ranks;
}

int heat() {
	double **before, **after;
	int  c, l;

	before = (double **) malloc(nx * sizeof(double *));
  	after = (double **) malloc(nx * sizeof(double *));

  	#pragma omp parallel for schedule(static, 64) 
  	for (int i = 0; i < nx; ++i) 
  	{
  		*(before + i) = (double *) malloc(ny * sizeof(double));
    	*(after + i) = (double *) malloc(ny * sizeof(double));
  	}
  	cout << "FINISH ALLOCTION *************************" << endl;

  	#pragma omp parallel for schedule(static, 64) 
  	for (int i = 0; i < nx; ++i) 
  	{
  		int a, b, llb, lub;

  		a = i;
  		b = 0;
  		before[a][b] = randa(xu + a * dx, 0);

  		b = ny - 1;
  		before[a][b] = randb(xu + a * dx, 0);

  		if (i == 0) {
  			for (a=0, b=0; b < ny; b++){
      			before[a][b] = randc(yu + b * dy, 0);
  			}
  		} else if (i == nx - 1) {
  			for (a=nx-1, b=0; b < ny; b++){
      			before[a][b] = randd(yu + b * dy, 0);
  			}
  		} else {
  			for (b=1; b < ny-1; b++) {
      			before[a][b] = f(xu + a * dx, yu + b * dy);
    		}
  		}
  	}

  	cout << "FINISH INITTTTT *************************" << endl;

  	for (c = 1; c <= nt; c++) {
  		t = tu + c * dt;

  		#pragma omp parallel for schedule(static, 64)
	    for (int t = 0; t < nx; ++t)
	    {
			int i, a, b, llb, lub;

			int tmp = t / 64;
			if (tmp >= 64) {
				i = t % 64 + 64 * 0;
			} else if (tmp >= 32) {
				i = t % 64 + 64 * 1;
			} else if (tmp >= 16) {
				i = t % 64 + 64 * 2;
			} else if (tmp >= 8) {
				i = t % 64 + 64 * 3;
			} else if (tmp >= 4) {
				i = t % 64 + 64 * 4;
			} else if (tmp >= 2) {
				i = t % 64 + 64 * 5;
			} else if (tmp >= 1) {
				i = t % 64 + 64 * 6;
			} else {
				i = t % 64 + 64 * 7;
			}
	    
		    if (i == 0) {
		    	for (a=0, b=0; b < ny; b++){
      				before[a][b] = randc(yu + b * dy, t);
		    	}
		    } else if (i == nx - 1) {
		    	for (a=nx-1, b=0; b < ny; b++){
      				before[a][b] = randd(yu + b * dy, t);
		    	}
		    } else {
		    	after[i][ny-1] = randb(xu + a * dx, t);
		    	after[i][0] = randa(xu + a * dx, t);

		    	for (a=i, b=1; b < ny-1; b++) {
			      	after[a][b] =   dtdxsq * (before[a+1][b] - 2 * before[a][b] + before[a-1][b])
				          + dtdysq * (before[a][b+1] - 2 * before[a][b] + before[a][b-1])
			  	          + before[a][b];
			    }
		    }
	    }

  		if (c % 5 == 0)
		{
			usleep(20000);
		}

		swap_ranks(&after, &before);
  	}
	printf("I've done!\n");

    return 0;
}

void* collect(void* args)
{
	int f, i, c, actual_len;
	char write_info[100];
	// char read_info[5000000];

	sleep(1);

	f = open("/proc/heat", O_RDWR | O_TRUNC);

	ftruncate(f, 0);
    lseek(f, 0, SEEK_SET);
	sprintf(write_info, "filter %d", getpid());
	write(f, write_info, strlen(write_info));

	for (c = 0; c < 50; ++c)
	{
		ftruncate(f, 0);
		lseek(f, 0, SEEK_SET);
		sprintf(write_info, "collect %d", getpid());
		write(f, write_info, strlen(write_info));
		read(f, read_info, sizeof(read_info));
		actual_len = read_info[0].access_time;
		printf("-----------------------------HEAT AT %d-----------------------------\n", c);
		for (i = 1; i <= actual_len; ++i)
		{
			if(read_info[i].access_time > 0)
				printf("PAGE: 0x%lx\tHEAT: %d\n", read_info[i].addr, read_info[i].access_time);
		}
		printf("-----------------------------HEAT AT %d-----------------------------\n", c);
		usleep(5000);
	}
	close(f);
}

int main(int argc, char *argv[]){
	pthread_t id;
	int ret;

	nx = atoi(argv[1])*1024;
	ny = NY;
	nt = NT;
	xu = 0.0;
  	xo = 1.570796326794896558;
  	yu = 0.0;
  	yo = 1.570796326794896558;
  	tu = 0.0;
  	to = 0.0000001;
  	leafmaxcol = 8;

  	dx = (xo - xu) / (nx - 1);
  	dy = (yo - yu) / (ny - 1);
  	dt = (to - tu) / nt;	/* nt effective time steps! */

  	dtdxsq = dt / (dx * dx);
  	dtdysq = dt / (dy * dy);

	ret = pthread_create(&id, NULL, &collect, NULL);
	if (ret == 0)
	{
		printf("-----------------------------Start heat collection-----------------------------\n");
	}
	else
	{
		printf("Start collection failed!\n");
		return 0;
	}

	heat();
    
    return 0;
}