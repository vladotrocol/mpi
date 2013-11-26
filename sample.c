  //Even rank
          if(rank%2==0){
            //Copy last row into mes
            for(hh=0;hh<params.nx;hh++){
              for(ff=0;ff<9;ff++){
                mes[9*hh+ff] = cells[ii*params.nx + hh].speeds[ff];
              }
            }
            //send mess
            MPI_Send(mes, 9*params.nx, MPI_FLOAT, rank_send, tag, MPI_COMM_WORLD);
            //recive last row from prev process into mes
            MPI_Recv(mes2, 9*params.nx, MPI_FLOAT, rank_receive, tag, MPI_COMM_WORLD, &status);
            //Put mes into first row
            for(hh=0;hh<params.nx;hh++){
              for(ff=0;ff<9;ff++){
                cells[rank*(params.ny/nprocs)*params.nx + hh].speeds[ff]=mes2[9*hh+ff];
              }
            }

          }
          //Odd rank
          else{
            //receive last row from prev process into mes
            MPI_Recv(mes, 9*params.nx, MPI_FLOAT, rank_receive, tag, MPI_COMM_WORLD, &status); 
            //put mes into fist row
            for(hh=0;hh<params.nx;hh++){
              for(ff=0;ff<9;ff++){
                cells[rank*(params.ny/nprocs)*params.nx + hh].speeds[ff]=mes[9*hh+ff];
              }
            }
            //put last row into mess
            for(hh=0;hh<params.nx;hh++){
              for(ff=0;ff<9;ff++){
                mes2[9*hh+ff] = cells[ii*params.nx + hh].speeds[ff];
              }
            }
            //send mess
            MPI_Send(mes2, 9*params.nx, MPI_FLOAT, rank_send, tag, MPI_COMM_WORLD);
          }