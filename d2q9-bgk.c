/*
** Code to implement a d2q9-bgk lattice boltzmann scheme.
** 'd2' inidates a 2-dimensional grid, and
** 'q9' indicates 9 velocities per grid cell.
** 'bgk' refers to the Bhatnagar-Gross-Krook collision step.
**
** The 'speeds' in each cell are numbered as follows:
**
** 6 2 5
**  \|/
** 3-0-1
**  /|\
** 7 4 8
**
** A 2D grid:
**
**           cols
**       --- --- ---
**      | D | E | F |
** rows  --- --- ---
**      | A | B | C |
**       --- --- ---
**
** 'unwrapped' in row major order to give a 1D array:
**
**  --- --- --- --- --- ---
** | A | B | C | D | E | F |
**  --- --- --- --- --- ---
**
** Grid indicies are:
**
**          ny
**          ^       cols(jj)
**          |  ----- ----- -----
**          | | ... | ... | etc |
**          |  ----- ----- -----
** rows(ii) | | 1,0 | 1,1 | 1,2 |
**          |  ----- ----- -----
**          | | 0,0 | 0,1 | 0,2 |
**          |  ----- ----- -----
**          ----------------------> nx
**
** Note the names of the input parameter and obstacle files
** are passed on the command line, e.g.:
**
**   d2q9-bgk.exe input.params obstacles.dat
**
** Be sure to adjust the grid dimensions in the parameter file
** if you choose a different obstacle file.
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include<sys/resource.h>
#include "mpi.h"

#define NSPEEDS         9
#define FINALSTATEFILE  "final_state.dat"
#define AVVELSFILE      "av_vels.dat"
#define MASTER 0


//-----------------------__Average File----------------------------

FILE *ofp;
char *mode = "r";
char outputFilename[] = "farmTimes.t";

void openFile(){
    ofp = fopen(outputFilename, "a+");

    if (ofp == NULL) {
        fprintf(stderr, "Can't open output file %s!\n",
            outputFilename);
    }
}

void printF(double what){
    fprintf(ofp, "%.6lf ", what);
    //printf("%.6lf ", what);
}

//-----------------------------------------------------------





/* struct to hold the parameter values */
typedef struct {
  int    nx;            /* no. of cells in x-direction */
  int    ny;            /* no. of cells in y-direction */
  int    maxIters;      /* no. of iterations */
  int    reynolds_dim;  /* dimension for Reynolds number */
  float density;       /* density per link */
  float accel;         /* density redistribution */
  float omega;         /* relaxation parameter */
  int rest;
} t_param;

/* struct to hold the 'speed' values */
typedef struct {
  float speeds[NSPEEDS];
} t_speed;

enum boolean { FALSE, TRUE };

/*
** function prototypes
*/

/* load params, allocate memory, load obstacles & initialise fluid particle densities */
int initialise(const char* paramfile, const char* obstaclefile,
         t_param* params, t_speed** cells_ptr, t_speed** tmp_cells_ptr, 
         int** obstacles_ptr, float** av_vels_ptr);

/* 
** The main calculation methods.
** timestep calls, in order, the functions:
** accelerate_flow(), propagate(), rebound() & collision()
*/
int timestep(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles);
int accelerate_flow(const t_param params, t_speed* cells, int* obstacles);
int propagate(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles);
int rebound(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles);
int collision(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles);
int write_values(const t_param params, t_speed* cells, int* obstacles, float* av_vels);

/* finalise, including freeing up allocated memory */
int finalise(const t_param* params, t_speed** cells_ptr, t_speed** tmp_cells_ptr,
       int** obstacles_ptr, float** av_vels_ptr);

/* Sum all the densities in the grid.
** The total should remain constant from one timestep to the next. */
float total_density(const t_param params, t_speed* cells);

/* compute average velocity */
float av_velocity(const t_param params, t_speed* cells, int* obstacles);

/* calculate Reynolds number */
float calc_reynolds(const t_param params, t_speed* cells, int* obstacles);

/* utility functions */
void die(const char* message, const int line, const char *file);
void usage(const char* exe);

/*
** main program:
** initialise, timestep loop, finalise
*/

int atyt=1;

  int rank;           /* process rank */
  int nprocs;         /* number of processes */
  int source;         /* rank of sender */
  int dest = MASTER;  /* all procs send to master */
  int tag = 0;        /* message tag */
  MPI_Status status;  /* struct to hold message status */ 
  MPI_Request request;


int main(int argc, char* argv[])
{
  char*    paramfile;         /* name of the input parameter file */
  char*    obstaclefile;      /* name of a the input obstacle file */
  t_param  params;            /* struct to hold parameter values */
  t_speed* cells     = NULL;  /* grid containing fluid densities */
  t_speed* tmp_cells = NULL;  /* scratch space */
  int*     obstacles = NULL;  /* grid indicating which cells are blocked */
  float*  av_vels   = NULL;  /* a record of the av. velocity computed for each timestep */
  int      ii;                /* generic counter */
  struct timeval timstr;      /* structure to hold elapsed time */
  struct rusage ru;           /* structure to hold CPU time--system and user */
  double tic,toc;             /* floating point numbers to calculate elapsed wallclock time */
  double usrtim;              /* floating point number to record elapsed user CPU time */
  double systim;              /* floating point number to record elapsed system CPU time */

  /* parse the command line */
  if(argc != 3) {
    usage(argv[0]);
  }
  else{
    paramfile = argv[1];
    obstaclefile = argv[2];
  }

  /* initialise our data structures and load values from file */
  initialise(paramfile, obstaclefile, &params, &cells, &tmp_cells, &obstacles, &av_vels);

  /* iterate for maxIters timesteps */
  gettimeofday(&timstr,NULL);
  tic=timstr.tv_sec+(timstr.tv_usec/1000000.0);





float tot_u_x=0; 
  int    tot_cells = 0; 


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
int start=0, end=0, ff, hh, gg;
  params.rest = params.ny%nprocs;
  for (ii=0;ii<params.maxIters;ii++) {
    timestep(params,cells,tmp_cells,obstacles);

    if(ii==1){
      atyt = 0;
    }
      if(ii==2){
      atyt = 1;
    }
   



tot_u_x=0; 
tot_cells = 0; 

 

///////////av_velocity.................................................
    int    jj,kk, tt, source;
    float local_density=0;
    float l_tot_u_x=0; 
    int    l_tot_cells = 0; 


 if(params.rest!=0){
    if(rank>=params.rest){
      start = params.rest*(params.ny/nprocs+1) + (rank-params.rest)*(params.ny/nprocs);
      end= start+params.ny/nprocs-1;
    }
    else{
      start = rank*(params.ny/nprocs+1);
      end = start+params.ny/nprocs;
    }
   }

    for(kk=start;kk<=end;kk++)
    
      {
        for(jj=0;jj<params.nx;jj++)
        {
                  if(!obstacles[kk*params.nx + jj])
                  {
                    local_density= 0.0;
                    for(tt=0;tt<NSPEEDS;tt++)
                    {
                      local_density += cells[kk*params.nx + jj].speeds[tt];
                    }
          
                    l_tot_u_x += (cells[kk*params.nx + jj].speeds[1] + 
                        cells[kk*params.nx + jj].speeds[5] + 
                        cells[kk*params.nx + jj].speeds[8]
                        - (cells[kk*params.nx + jj].speeds[3] + 
                           cells[kk*params.nx + jj].speeds[6] + 
                           cells[kk*params.nx + jj].speeds[7])) / 
                    local_density;

                    l_tot_cells+=1;
                  }

        }
      }


    if(rank!=MASTER){
      MPI_Send(&l_tot_u_x, 1, MPI_FLOAT, dest, tag, MPI_COMM_WORLD);
      MPI_Send(&l_tot_cells, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);}
    else{
      tot_cells = l_tot_cells;
      tot_u_x = l_tot_u_x;
      for (source =1; source < nprocs; source++) {
        MPI_Recv(&l_tot_u_x, 1, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &status);
        tot_u_x+=l_tot_u_x;
        MPI_Recv(&l_tot_cells, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
        tot_cells+=l_tot_cells;
      }
      av_vels[ii] = tot_u_x / (float)tot_cells;
    }




    }


























 
  float mes[9*params.nx];
  float mes2[9*params.nx];
 
 int xx, kk, jj;

      if(params.rest!=0){
    if(rank>=params.rest){
      start = params.rest*(params.ny/nprocs+1) + (rank-params.rest)*(params.ny/nprocs);
      end= start+params.ny/nprocs-1;
    }
    else{
      start = rank*(params.ny/nprocs+1);
      end = start+params.ny/nprocs;
    }
   }

  for(xx=0;xx<params.ny;xx++){
    if(rank!=master){
      if(xx>=start&&xx<=end){
        for(jj=0;jj<params.nx;jj++) {
           for(kk=0;kk<NSPEEDS;kk++) {
             cell_c[jj*NSPEEDS+kk]=cells[ii*params.nx+jj].speeds[kk];
            }
         }  
         MPI_Send(cell_c, NSPEEDS*params.nx, MPI_FLOAT,0, 0, MPI_COMM_WORLD);
      }
    }

    if(rank==master){
      for(source=1;source<nprocs;source++){
        if(params.rest!=0){
          if(source>=params.rest){
            start = params.rest*(params.ny/nprocs+1) + (source-params.rest)*(params.ny/nprocs);
            end= start+params.ny/nprocs-1;
          }
          else{
            start = source*(params.ny/nprocs+1);
            end = start+params.ny/nprocs;
          }
        }
        if(ii>=start&&ii<+end)
        MPI_Recv(mes2, NSPEEDS*params.nx, MPI_FLOAT, source, tag, MPI_COMM_WORLD, &status);
          for(hh=0;hh<params.nx;hh++){
            for(ff=0;ff<NSPEEDS;ff++){
              cells[gg*params.nx + hh].speeds[ff]=mes2[NSPEEDS*hh+ff];
            }
          }
        }     
      }
    }
  }
  
  ///////////av_velocity.................................................

  MPI_Finalize();




  gettimeofday(&timstr,NULL);
  toc=timstr.tv_sec+(timstr.tv_usec/1000000.0);
  getrusage(RUSAGE_SELF, &ru);
  timstr=ru.ru_utime;        
  usrtim=timstr.tv_sec+(timstr.tv_usec/1000000.0);
  timstr=ru.ru_stime;        
  systim=timstr.tv_sec+(timstr.tv_usec/1000000.0);

  if(rank==0){
  /* write final values and free memory */
  printf("==done==\n");
  printf("Reynolds number:\t\t%.12E\n",calc_reynolds(params,cells,obstacles));
  printf("Elapsed time:\t\t\t%.6lf (s)\n", toc-tic);
  printf("Elapsed user CPU time:\t\t%.6lf (s)\n", usrtim);
  printf("Elapsed system CPU time:\t%.6lf (s)\n", systim);
  write_values(params,cells,obstacles,av_vels);
  finalise(&params, &cells, &tmp_cells, &obstacles, &av_vels);
  //openFile(); // OPEN THE MULTIPLE OUTPUT FILE
  //printf("\n\n%f\n", toc-tic);
  //printF(toc-tic);
 // fclose(ofp);
}
  return EXIT_SUCCESS;
}

int timestep(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles)
{
  //accelerate_flow(params,cells,obstacles);
  propagate(params,cells,tmp_cells, obstacles);
  rebound(params,cells,tmp_cells,obstacles);
  collision(params,cells,tmp_cells,obstacles);
  return EXIT_SUCCESS; 
}


int propagate(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles)
{

  int ii,jj;            /* generic counters */
  int x_e,x_w,y_n,y_s;  /* indices of neighbouring cells */
  float w1,w2;  /* weighting factors */
  
  /* compute weighting factors */
  w1 = params.density * params.accel / 9.0;
  w2 = params.density * params.accel / 36.0;
  
  float halo_north[9*params.nx];
  float halo_north_r[9*params.nx];
  float halo_south[9*params.nx];
  float halo_south_r[9*params.nx];


    




      int hh, ff;
      int rank_right = (rank + 1) % nprocs;
      int rank_left = rank-1;
      if(rank_left<0)
        rank_left = nprocs-1;
      int h_south;
      int h_north;




   int start=0, end=0;
   // if(rank<params.rest && params.rest!=0){
   //    start = rank*(params.ny/nprocs)+rank%params.rest;
   //    end = (rank+1)*(params.ny/nprocs) + (rank)%params.rest;
   // }
   // else if(params.rest!=0){
   //  start= rank*(params.ny/nprocs)+params.rest-1;
   //  end = (rank+1)*(params.ny/nprocs)+params.rest-1;
   // }

   if(params.rest!=0){
    if(rank>=params.rest){
      start = params.rest*(params.ny/nprocs+1) + (rank-params.rest)*(params.ny/nprocs);
      end= start+params.ny/nprocs-1;
    }
    else{
      start = rank*(params.ny/nprocs+1);
      end = start+params.ny/nprocs;
    }
   }

      if(start==0){
        h_south = params.ny-1;
      }
      else{
        h_south = start-1;
      }

      if(end==params.ny-1){
        h_north = 0;
      }
      else{
        h_north = end+1;
      }

      // if(atyt==0){
      //   printf("r: %d, s: %d, e: %d, hs: %d, hn: %d\n", rank, start, end, h_south, h_north);
      // }




      int kk;

        //copy last row into south_halo
        for(hh=0;hh<params.nx;hh++){
            for(ff=0;ff<9;ff++){
              halo_south[9*hh+ff] = cells[(end)*params.nx + hh].speeds[ff];
            }
          }

        //send south halo to the right and receive it into north halo from the left
        MPI_Sendrecv(halo_south, 9*params.nx, MPI_FLOAT, rank_right, tag, halo_south_r, 9*params.nx, MPI_FLOAT, rank_left, tag, MPI_COMM_WORLD, &status);
        //printf("\n\nSpeed: %f\n", halo_north[0]);

        //printf("%f, %f\n",cells[h_south*params.nx + hh].speeds[0], cells[h_south*params.nx + hh].speeds[1]);
        //copy north halo into first row of cells
        for(hh=0;hh<params.nx;hh++){
            for(ff=0;ff<9;ff++){
              cells[h_south*params.nx + hh].speeds[ff]=halo_south_r[9*hh+ff];
            }
          }
        //printf("%f, %f\n",cells[h_south*params.nx + hh].speeds[0], cells[h_south*params.nx + hh].speeds[1]);

        //copy fist row into north halo
        for(hh=0;hh<params.nx;hh++){
          for(ff=0;ff<9;ff++){
            halo_north[9*hh+ff] = cells[start*params.nx + hh].speeds[ff];
          }
        }

        //send the north halo to the left and receive into south halo from the right
        MPI_Sendrecv(halo_north, 9*params.nx, MPI_FLOAT, rank_left, tag, halo_north_r, 9*params.nx, MPI_FLOAT, rank_right, tag, MPI_COMM_WORLD, &status);
        //        MPI_Sendrecv_replace(mes, 9*params.nx, MPI_FLOAT, rank_right, tag,
        //         rank_left, tag, MPI_COMM_WORLD, &status);
        
        //copy south halo into last row of cells
        for(hh=0;hh<params.nx;hh++){
          for(ff=0;ff<9;ff++){
            cells[h_north*params.nx + hh].speeds[ff]=halo_south_r[9*hh+ff];
          }
        }
        

        jj=0;
  
  ii=h_south;
    if( !obstacles[ii*params.nx + jj] && 
  (cells[ii*params.nx + jj].speeds[3] - w1) > 0.0 &&
  (cells[ii*params.nx + jj].speeds[6] - w2) > 0.0 &&
  (cells[ii*params.nx + jj].speeds[7] - w2) > 0.0 ) {
      /* increase 'east-side' densities */
      cells[ii*params.nx + jj].speeds[1] += w1;
      cells[ii*params.nx + jj].speeds[5] += w2;
      cells[ii*params.nx + jj].speeds[8] += w2;
      /* decrease 'west-side' densities */
      cells[ii*params.nx + jj].speeds[3] -= w1;
      cells[ii*params.nx + jj].speeds[6] -= w2;
      cells[ii*params.nx + jj].speeds[7] -= w2;
    }



  for(jj=0;jj<params.nx;jj++) {  
      y_n = (ii + 1) % params.ny;
      y_s = (ii == 0) ? (ii + params.ny - 1) : (ii - 1);


      x_e = (jj + 1) % params.nx;
      x_w = (jj == 0) ? (jj + params.nx - 1) : (jj - 1);

      /* propagate densities to neighbouring cells, following
      ** appropriate directions of travel and writing into
      ** scratch space grid */  
      tmp_cells[ii *params.nx + jj].speeds[0]  = cells[ii*params.nx + jj].speeds[0]; /* central cell, */
                                                                                     /* no movement   */
      tmp_cells[ii *params.nx + x_e].speeds[1] = cells[ii*params.nx + jj].speeds[1]; /* east */
      tmp_cells[y_n*params.nx + jj].speeds[2]  = cells[ii*params.nx + jj].speeds[2]; /* north */
      tmp_cells[ii *params.nx + x_w].speeds[3] = cells[ii*params.nx + jj].speeds[3]; /* west */
      tmp_cells[y_s*params.nx + jj].speeds[4]  = cells[ii*params.nx + jj].speeds[4]; /* south */
      tmp_cells[y_n*params.nx + x_e].speeds[5] = cells[ii*params.nx + jj].speeds[5]; /* north-east */
      tmp_cells[y_n*params.nx + x_w].speeds[6] = cells[ii*params.nx + jj].speeds[6]; /* north-west */
      tmp_cells[y_s*params.nx + x_w].speeds[7] = cells[ii*params.nx + jj].speeds[7]; /* south-west */      
      tmp_cells[y_s*params.nx + x_e].speeds[8] = cells[ii*params.nx + jj].speeds[8]; /* south-east */      

}

  jj=0;
  ii=h_north;
    if( !obstacles[ii*params.nx + jj] && 
  (cells[ii*params.nx + jj].speeds[3] - w1) > 0.0 &&
  (cells[ii*params.nx + jj].speeds[6] - w2) > 0.0 &&
  (cells[ii*params.nx + jj].speeds[7] - w2) > 0.0 ) {
      /* increase 'east-side' densities */
      cells[ii*params.nx + jj].speeds[1] += w1;
      cells[ii*params.nx + jj].speeds[5] += w2;
      cells[ii*params.nx + jj].speeds[8] += w2;
      /* decrease 'west-side' densities */
      cells[ii*params.nx + jj].speeds[3] -= w1;
      cells[ii*params.nx + jj].speeds[6] -= w2;
      cells[ii*params.nx + jj].speeds[7] -= w2;
    }



/* determine indices of axis-direction neighbours
      ** respecting periodic boundary conditions (wrap around) */
  for(jj=0;jj<params.nx;jj++) {  
      y_n = (ii + 1) % params.ny;
      y_s = (ii == 0) ? (ii + params.ny - 1) : (ii - 1);


      x_e = (jj + 1) % params.nx;
      x_w = (jj == 0) ? (jj + params.nx - 1) : (jj - 1);

      /* propagate densities to neighbouring cells, following
      ** appropriate directions of travel and writing into
      ** scratch space grid */  
      tmp_cells[ii *params.nx + jj].speeds[0]  = cells[ii*params.nx + jj].speeds[0]; /* central cell, */
                                                                                     /* no movement   */
      tmp_cells[ii *params.nx + x_e].speeds[1] = cells[ii*params.nx + jj].speeds[1]; /* east */
      tmp_cells[y_n*params.nx + jj].speeds[2]  = cells[ii*params.nx + jj].speeds[2]; /* north */
      tmp_cells[ii *params.nx + x_w].speeds[3] = cells[ii*params.nx + jj].speeds[3]; /* west */
      tmp_cells[y_s*params.nx + jj].speeds[4]  = cells[ii*params.nx + jj].speeds[4]; /* south */
      tmp_cells[y_n*params.nx + x_e].speeds[5] = cells[ii*params.nx + jj].speeds[5]; /* north-east */
      tmp_cells[y_n*params.nx + x_w].speeds[6] = cells[ii*params.nx + jj].speeds[6]; /* north-west */
      tmp_cells[y_s*params.nx + x_w].speeds[7] = cells[ii*params.nx + jj].speeds[7]; /* south-west */      
      tmp_cells[y_s*params.nx + x_e].speeds[8] = cells[ii*params.nx + jj].speeds[8]; /* south-east */      

}


  for(ii=start;ii<=end;ii++) {
    jj=0;
 
    if( !obstacles[ii*params.nx + jj] && 
  (cells[ii*params.nx + jj].speeds[3] - w1) > 0.0 &&
  (cells[ii*params.nx + jj].speeds[6] - w2) > 0.0 &&
  (cells[ii*params.nx + jj].speeds[7] - w2) > 0.0 ) {
      /* increase 'east-side' densities */
      cells[ii*params.nx + jj].speeds[1] += w1;
      cells[ii*params.nx + jj].speeds[5] += w2;
      cells[ii*params.nx + jj].speeds[8] += w2;
      /* decrease 'west-side' densities */
      cells[ii*params.nx + jj].speeds[3] -= w1;
      cells[ii*params.nx + jj].speeds[6] -= w2;
      cells[ii*params.nx + jj].speeds[7] -= w2;
    }





    for(jj=0;jj<params.nx;jj++) {
      /* determine indices of axis-direction neighbours
      ** respecting periodic boundary conditions (wrap around) */
      y_n = (ii + 1) % params.ny;
      y_s = (ii == 0) ? (ii + params.ny - 1) : (ii - 1);


      x_e = (jj + 1) % params.nx;
      x_w = (jj == 0) ? (jj + params.nx - 1) : (jj - 1);

      /* propagate densities to neighbouring cells, following
      ** appropriate directions of travel and writing into
      ** scratch space grid */  
      tmp_cells[ii *params.nx + jj].speeds[0]  = cells[ii*params.nx + jj].speeds[0]; /* central cell, */
                                                                                     /* no movement   */
      tmp_cells[ii *params.nx + x_e].speeds[1] = cells[ii*params.nx + jj].speeds[1]; /* east */
      tmp_cells[y_n*params.nx + jj].speeds[2]  = cells[ii*params.nx + jj].speeds[2]; /* north */
      tmp_cells[ii *params.nx + x_w].speeds[3] = cells[ii*params.nx + jj].speeds[3]; /* west */
      tmp_cells[y_s*params.nx + jj].speeds[4]  = cells[ii*params.nx + jj].speeds[4]; /* south */
      tmp_cells[y_n*params.nx + x_e].speeds[5] = cells[ii*params.nx + jj].speeds[5]; /* north-east */
      tmp_cells[y_n*params.nx + x_w].speeds[6] = cells[ii*params.nx + jj].speeds[6]; /* north-west */
      tmp_cells[y_s*params.nx + x_w].speeds[7] = cells[ii*params.nx + jj].speeds[7]; /* south-west */      
      tmp_cells[y_s*params.nx + x_e].speeds[8] = cells[ii*params.nx + jj].speeds[8]; /* south-east */      
    }
  }
    

  return EXIT_SUCCESS;
}

int rebound(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles)
{
  int ii,jj;  /* generic counters */
    //#pragma omp parallel for private(jj)
  /* loop over the cells in the grid */
     int start=0, end=0;
   if(params.rest!=0){
    if(rank>=params.rest){
      start = params.rest*(params.ny/nprocs+1) + (rank-params.rest)*(params.ny/nprocs);
      end= start+params.ny/nprocs-1;
    }
    else{
      start = rank*(params.ny/nprocs+1);
      end = start+params.ny/nprocs;
    }
   }
  for(ii=start;ii<=end;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      /* if the cell contains an obstacle */
      if(obstacles[ii*params.nx + jj]) {
  /* called after propagate, so taking values from scratch space
  ** mirroring, and writing into main grid */
  cells[ii*params.nx + jj].speeds[1] = tmp_cells[ii*params.nx + jj].speeds[3];
  cells[ii*params.nx + jj].speeds[2] = tmp_cells[ii*params.nx + jj].speeds[4];
  cells[ii*params.nx + jj].speeds[3] = tmp_cells[ii*params.nx + jj].speeds[1];
  cells[ii*params.nx + jj].speeds[4] = tmp_cells[ii*params.nx + jj].speeds[2];
  cells[ii*params.nx + jj].speeds[5] = tmp_cells[ii*params.nx + jj].speeds[7];
  cells[ii*params.nx + jj].speeds[6] = tmp_cells[ii*params.nx + jj].speeds[8];
  cells[ii*params.nx + jj].speeds[7] = tmp_cells[ii*params.nx + jj].speeds[5];
  cells[ii*params.nx + jj].speeds[8] = tmp_cells[ii*params.nx + jj].speeds[6];
      }
    }
  }

  return EXIT_SUCCESS;
}

int collision(const t_param params, t_speed* cells, t_speed* tmp_cells, int* obstacles)
{
  int ii,jj;                 /* generic counters */
    /* square of speed of sound */
  const float w0 = 0.4444444444;    /* weighting factor */
  const float w1 = 0.1111111111;    /* weighting factor */
  const float w2 = 0.0277777777;   /* weighting factor */
  float u_x,u_y;               /* av. velocities in x and y directions */
  float u_sq;                  /* squared velocity */
  float local_density;         /* sum of densities in a particular cell */
  
  
  /* loop over the cells in the grid
  ** NB the collision step is called after
  ** the propagate step and so values of interest
  ** are in the scratch-space grid */
  //#pragma omp parallel private(ii, jj, u_x, u_y, u_sq, local_density)
  //#pragma omp for
    int start=0, end=0;
   if(params.rest!=0){
    if(rank>=params.rest){
      start = params.rest*(params.ny/nprocs+1) + (rank-params.rest)*(params.ny/nprocs);
      end= start+params.ny/nprocs-1;
    }
    else{
      start = rank*(params.ny/nprocs+1);
      end = start+params.ny/nprocs;
    }
   }
  for(ii=start;ii<=end;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      /* don't consider occupied cells */
      if(!obstacles[ii*params.nx + jj]) {
          /* compute local density total */
              local_density = tmp_cells[ii*params.nx + jj].speeds[0]+tmp_cells[ii*params.nx + jj].speeds[1]+tmp_cells[ii*params.nx + jj].speeds[2]+tmp_cells[ii*params.nx + jj].speeds[3]+tmp_cells[ii*params.nx + jj].speeds[4]+tmp_cells[ii*params.nx + jj].speeds[5]+tmp_cells[ii*params.nx + jj].speeds[6]+tmp_cells[ii*params.nx + jj].speeds[7]+tmp_cells[ii*params.nx + jj].speeds[8];
              
          /* compute x velocity component */
          u_x = (tmp_cells[ii*params.nx + jj].speeds[1] + tmp_cells[ii*params.nx + jj].speeds[5] + tmp_cells[ii*params.nx + jj].speeds[8]- (tmp_cells[ii*params.nx + jj].speeds[3] + tmp_cells[ii*params.nx + jj].speeds[6] + tmp_cells[ii*params.nx + jj].speeds[7]))/(local_density);
          
          /* compute y velocity component */
          u_y = (tmp_cells[ii*params.nx + jj].speeds[2] +tmp_cells[ii*params.nx + jj].speeds[5] + tmp_cells[ii*params.nx + jj].speeds[6]- (tmp_cells[ii*params.nx + jj].speeds[4] + tmp_cells[ii*params.nx + jj].speeds[7] +tmp_cells[ii*params.nx + jj].speeds[8]))/(local_density);
         
          /* velocity squared */ 
          u_sq = (u_x * u_x + u_y * u_y)*1.5;
  //unrolling everything
  
  
  
  
  cells[ii*params.nx + jj].speeds[0] = (tmp_cells[ii*params.nx + jj].speeds[0]+params.omega*((w0 * local_density * (1.0 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[0]));
  
  cells[ii*params.nx + jj].speeds[1] = (tmp_cells[ii*params.nx + jj].speeds[1]+params.omega*((w1 * local_density * (1.0 + u_x *3 + (u_x * u_x)*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[1]));
  

  
  cells[ii*params.nx + jj].speeds[2] = (tmp_cells[ii*params.nx + jj].speeds[2]+params.omega*((w1 * local_density * (1.0 + u_y*3 + (u_y * u_y)*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[2]));
  
  cells[ii*params.nx + jj].speeds[3] = (tmp_cells[ii*params.nx + jj].speeds[3]+params.omega*((w1 * local_density * (1.0 -u_x*3 + (u_x * u_x)*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[3]));
  

  
  cells[ii*params.nx + jj].speeds[4] = (tmp_cells[ii*params.nx + jj].speeds[4]+params.omega*((w1 * local_density * (1.0 -u_y*3 + (u_y*u_y)*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[4]));
  
  cells[ii*params.nx + jj].speeds[5] = (tmp_cells[ii*params.nx + jj].speeds[5]+params.omega*((w2 * local_density * (1.0 + (u_x+u_y)*3 + ((u_x+u_y) * (u_x+u_y))*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[5]));
  
  
  cells[ii*params.nx + jj].speeds[6] = (tmp_cells[ii*params.nx + jj].speeds[6]+params.omega*((w2 * local_density * (1.0 + (u_y-u_x)*3 + ((u_y-u_x) * (u_y-u_x))*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[6]));
  
  cells[ii*params.nx + jj].speeds[7] = (tmp_cells[ii*params.nx + jj].speeds[7]+params.omega*((w2 * local_density * (1.0 + (-u_y-u_x)*3 + ((-u_y-u_x) * (-u_y-u_x))*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[7]));
  
  cells[ii*params.nx + jj].speeds[8] = (tmp_cells[ii*params.nx + jj].speeds[8]+params.omega*((w2 * local_density * (1.0 + (u_x-u_y)*3 + ((u_x-u_y) * (u_x-u_y))*4.5 - u_sq)) - tmp_cells[ii*params.nx + jj].speeds[8]));  
  
      }
      
    }
  }

  return EXIT_SUCCESS; 
}

int initialise(const char* paramfile, const char* obstaclefile,
         t_param* params, t_speed** cells_ptr, t_speed** tmp_cells_ptr, 
         int** obstacles_ptr, float** av_vels_ptr)
{
  char   message[1024];  /* message buffer */
  FILE   *fp;            /* file pointer */
  int    ii,jj;          /* generic counters */
  int    xx,yy;          /* generic array indices */
  int    blocked;        /* indicates whether a cell is blocked by an obstacle */ 
  int    retval;         /* to hold return value for checking */
  float w0,w1,w2;       /* weighting factors */

  /* open the parameter file */
  fp = fopen(paramfile,"r");
  if (fp == NULL) {
    sprintf(message,"could not open input parameter file: %s", paramfile);
    die(message,__LINE__,__FILE__);
  }

  /* read in the parameter values */
  retval = fscanf(fp,"%d\n",&(params->nx));
  if(retval != 1) die ("could not read param file: nx",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->ny));
  if(retval != 1) die ("could not read param file: ny",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->maxIters));
  if(retval != 1) die ("could not read param file: maxIters",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->reynolds_dim));
  if(retval != 1) die ("could not read param file: reynolds_dim",__LINE__,__FILE__);
  retval = fscanf(fp,"%f\n",&(params->density));
  if(retval != 1) die ("could not read param file: density",__LINE__,__FILE__);
  retval = fscanf(fp,"%f\n",&(params->accel));
  if(retval != 1) die ("could not read param file: accel",__LINE__,__FILE__);
  retval = fscanf(fp,"%f\n",&(params->omega));
  if(retval != 1) die ("could not read param file: omega",__LINE__,__FILE__);

  /* and close up the file */
  fclose(fp);

  /* 
  ** Allocate memory.
  **
  ** Remember C is pass-by-value, so we need to
  ** pass pointers into the initialise function.
  **
  ** NB we are allocating a 1D array, so that the
  ** memory will be contiguous.  We still want to
  ** index this memory as if it were a (row major
  ** ordered) 2D array, however.  We will perform
  ** some arithmetic using the row and column
  ** coordinates, inside the square brackets, when
  ** we want to access elements of this array.
  **
  ** Note also that we are using a structure to
  ** hold an array of 'speeds'.  We will allocate
  ** a 1D array of these structs.
  */

  /* main grid */
  *cells_ptr = (t_speed*)malloc(sizeof(t_speed)*(params->ny*params->nx));
  if (*cells_ptr == NULL) 
    die("cannot allocate memory for cells",__LINE__,__FILE__);

  /* 'helper' grid, used as scratch space */
  *tmp_cells_ptr = (t_speed*)malloc(sizeof(t_speed)*(params->ny*params->nx));
  if (*tmp_cells_ptr == NULL) 
    die("cannot allocate memory for tmp_cells",__LINE__,__FILE__);
  
  /* the map of obstacles */
  *obstacles_ptr = malloc(sizeof(int*)*(params->ny*params->nx));
  if (*obstacles_ptr == NULL) 
    die("cannot allocate column memory for obstacles",__LINE__,__FILE__);

  /* initialise densities */
  w0 = params->density * 4.0/9.0;
  w1 = params->density      /9.0;
  w2 = params->density      /36.0;

  for(ii=0;ii<params->ny;ii++) {
    for(jj=0;jj<params->nx;jj++) {
      /* centre */
      (*cells_ptr)[ii*params->nx + jj].speeds[0] = w0;
      /* axis directions */
      (*cells_ptr)[ii*params->nx + jj].speeds[1] = w1;
      (*cells_ptr)[ii*params->nx + jj].speeds[2] = w1;
      (*cells_ptr)[ii*params->nx + jj].speeds[3] = w1;
      (*cells_ptr)[ii*params->nx + jj].speeds[4] = w1;
      /* diagonals */
      (*cells_ptr)[ii*params->nx + jj].speeds[5] = w2;
      (*cells_ptr)[ii*params->nx + jj].speeds[6] = w2;
      (*cells_ptr)[ii*params->nx + jj].speeds[7] = w2;
      (*cells_ptr)[ii*params->nx + jj].speeds[8] = w2;
    }
  }

  /* first set all cells in obstacle array to zero */ 
  for(ii=0;ii<params->ny;ii++) {
    for(jj=0;jj<params->nx;jj++) {
      (*obstacles_ptr)[ii*params->nx + jj] = 0;
    }
  }

  /* open the obstacle data file */
  fp = fopen(obstaclefile,"r");
  if (fp == NULL) {
    sprintf(message,"could not open input obstacles file: %s", obstaclefile);
    die(message,__LINE__,__FILE__);
  }

  /* read-in the blocked cells list */
  while( (retval = fscanf(fp,"%d %d %d\n", &xx, &yy, &blocked)) != EOF) {
    /* some checks */
    if ( retval != 3)
      die("expected 3 values per line in obstacle file",__LINE__,__FILE__);
    if ( xx<0 || xx>params->nx-1 )
      die("obstacle x-coord out of range",__LINE__,__FILE__);
    if ( yy<0 || yy>params->ny-1 )
      die("obstacle y-coord out of range",__LINE__,__FILE__);
    if ( blocked != 1 ) 
      die("obstacle blocked value should be 1",__LINE__,__FILE__);
    /* assign to array */
    (*obstacles_ptr)[yy*params->nx + xx] = blocked;
  }
  
  /* and close the file */
  fclose(fp);

  /* 
  ** allocate space to hold a record of the avarage velocities computed 
  ** at each timestep
  */
  *av_vels_ptr = (float*)malloc(sizeof(float)*params->maxIters);

  return EXIT_SUCCESS;
}

int finalise(const t_param* params, t_speed** cells_ptr, t_speed** tmp_cells_ptr,
       int** obstacles_ptr, float** av_vels_ptr)
{
  /* 
  ** free up allocated memory
  */
  free(*cells_ptr);
  *cells_ptr = NULL;

  free(*tmp_cells_ptr);
  *tmp_cells_ptr = NULL;

  free(*obstacles_ptr);
  *obstacles_ptr = NULL;

  free(*av_vels_ptr);
  *av_vels_ptr = NULL;

  return EXIT_SUCCESS;
}

float av_velocity(const t_param params, t_speed* cells, int* obstacles)
{
  int    ii,jj,kk;       /* generic counters */
  int    tot_cells = 0;  /* no. of cells used in calculation */
  /* total density in cell */
  float tot_u_x;        /* accumulated x-components of velocity */
  float local_density;
  /* initialise */
  tot_u_x = 0.0;
  //#pragma omp parallel for private(jj, kk, local_density) reduction(+:tot_u_x, tot_cells)
  


  /* loop over all non-blocked cells */
  for(ii=0;ii<params.ny;ii++) {
    for(jj=0;jj<params.nx;jj++) {
              if(!obstacles[ii*params.nx + jj]) {
              local_density= 0.0;
              for(kk=0;kk<NSPEEDS;kk++) {
                local_density += cells[ii*params.nx + jj].speeds[kk];
              }
      
              tot_u_x += (cells[ii*params.nx + jj].speeds[1] + 
                    cells[ii*params.nx + jj].speeds[5] + 
                    cells[ii*params.nx + jj].speeds[8]
                    - (cells[ii*params.nx + jj].speeds[3] + 
                       cells[ii*params.nx + jj].speeds[6] + 
                       cells[ii*params.nx + jj].speeds[7])) / 
                local_density;

              tot_cells+=1;
            }
          
    }
  }

  return tot_u_x / (float)tot_cells;
}

float calc_reynolds(const t_param params, t_speed* cells, int* obstacles)
{
  const float viscosity = 1.0 / 6.0 * (2.0 / params.omega - 1.0);
  
  return av_velocity(params,cells,obstacles) * params.reynolds_dim / viscosity;
}

float total_density(const t_param params, t_speed* cells)
{
  int ii,jj,kk;        /* generic counters */
  float total = 0.0;  /* accumulator */

  for(ii=0;ii<params.ny;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      for(kk=0;kk<NSPEEDS;kk++) {
  total += cells[ii*params.nx + jj].speeds[kk];
      }
    }
  }
  
  return total;
}

int write_values(const t_param params, t_speed* cells, int* obstacles, float* av_vels)
{
  FILE* fp;                     /* file pointer */
  int ii,jj,kk;                 /* generic counters */
  const float c_sq = 1.0/3.0;  /* sq. of speed of sound */
  float local_density;         /* per grid cell sum of densities */
  float pressure;              /* fluid pressure in grid cell */
  float u_x;                   /* x-component of velocity in grid cell */
  float u_y;                   /* y-component of velocity in grid cell */

  fp = fopen(FINALSTATEFILE,"w");
  if (fp == NULL) {
    die("could not open file output file",__LINE__,__FILE__);
  }

  for(ii=0;ii<params.ny;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      /* an occupied cell */
      if(obstacles[ii*params.nx + jj]) {
  u_x = u_y = 0.0;
  pressure = params.density * c_sq;
      }
      /* no obstacle */
      else {
  local_density = 0.0;
  for(kk=0;kk<NSPEEDS;kk++) {
    local_density += cells[ii*params.nx + jj].speeds[kk];
  }
  /* compute x velocity component */
  u_x = (cells[ii*params.nx + jj].speeds[1] + 
         cells[ii*params.nx + jj].speeds[5] +
         cells[ii*params.nx + jj].speeds[8]
         - (cells[ii*params.nx + jj].speeds[3] + 
      cells[ii*params.nx + jj].speeds[6] + 
      cells[ii*params.nx + jj].speeds[7]))
    / local_density;
  /* compute y velocity component */
  u_y = (cells[ii*params.nx + jj].speeds[2] + 
         cells[ii*params.nx + jj].speeds[5] + 
         cells[ii*params.nx + jj].speeds[6]
         - (cells[ii*params.nx + jj].speeds[4] + 
      cells[ii*params.nx + jj].speeds[7] + 
      cells[ii*params.nx + jj].speeds[8]))
    / local_density;
  /* compute pressure */
  pressure = local_density * c_sq;
      }
      /* write to file */
      fprintf(fp,"%d %d %.12E %.12E %.12E %d\n",ii,jj,u_x,u_y,pressure,obstacles[ii*params.nx + jj]);
    }
  }

  fclose(fp);

  fp = fopen(AVVELSFILE,"w");
  if (fp == NULL) {
    die("could not open file output file",__LINE__,__FILE__);
  }
  for (ii=0;ii<params.maxIters;ii++) {
    fprintf(fp,"%d:\t%.12E\n", ii, av_vels[ii]);
  }

  fclose(fp);

  return EXIT_SUCCESS;
}

void die(const char* message, const int line, const char *file)
{
  fprintf(stderr, "Error at line %d of file %s:\n", line, file);
  fprintf(stderr, "%s\n",message);
  fflush(stderr);
  exit(EXIT_FAILURE);
}

void usage(const char* exe)
{
  fprintf(stderr, "Usage: %s <paramfile> <obstaclefile>\n", exe);
  exit(EXIT_FAILURE);
} 