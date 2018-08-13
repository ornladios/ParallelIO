#include <stdio.h>
#include <mpi.h>
#include <pio.h>
#ifdef TIMING
#include <gptl.h>
#endif

#define NUM_NETCDF_FLAVORS 5
#define DIM_NAME "x"
#define VAR_NAME "foo"

int main(int argc, char* argv[])
{
  int my_rank;
  int ntasks;
  int format[NUM_NETCDF_FLAVORS];
  int niotasks;
  int ioproc_stride = 1;
  int ioproc_start = 0;
  int dimid;
  int elements_on_this_rank;
  int dim_len;
  int iosysid;
  int ncid;
  int varid;
  int ioid;
  int *buffer = NULL;
  PIO_Offset *compdof = NULL;
  char filename[NC_MAX_NAME + 1];
  int num_flavors = 0;

#ifdef TIMING    
  GPTLinitialize();
#endif   

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

  if (!(ntasks == 2 || ntasks == 4)) {
    if (my_rank == 0)
      fprintf(stderr, "Number of processors must be 2 or 4!\n");
    MPI_Finalize();
    return 1;
  }

  /* Write 1D array, different procs have different number
     of elements to write locally 
     Even procs write out 2 elements with buffer data [1 2]
     Odd procs write out 1 element with buffer data [1]
  */
  if (my_rank % 2 == 0)
    elements_on_this_rank = 2;
  else
    elements_on_this_rank = 1;

  buffer = malloc(elements_on_this_rank * sizeof(int));
  for (int i = 0; i < elements_on_this_rank; i++)
    buffer[i] = i + 1;

  compdof = malloc(elements_on_this_rank * sizeof(PIO_Offset));

  if (ntasks == 2) {
    /* PIO decomposition: [1, 2] [3] */
    if (my_rank == 0) {
      compdof[0] = 1;
      compdof[1] = 2;
    }
    else if (my_rank == 1)
      compdof[0] = 3;

    dim_len = 3;

    niotasks = 2;
  }
  else if (ntasks == 4) {
    /* PIO decomposition: [1, 2] [3] [4, 5] [6] */
    if (my_rank == 0) {
      compdof[0] = 1;
      compdof[1] = 2;
    }
    else if (my_rank == 1)
      compdof[0] = 3;
    else if (my_rank == 2) {
      compdof[0] = 4;
      compdof[1] = 5;
    }
    else if (my_rank == 3)
      compdof[0] = 6;

    dim_len = 6;

    niotasks = 4;
  }

  PIOc_Init_Intracomm(MPI_COMM_WORLD, niotasks, ioproc_stride, ioproc_start, PIO_REARR_SUBSET, &iosysid);

  PIOc_InitDecomp(iosysid, PIO_INT, 1, &dim_len, elements_on_this_rank, compdof, &ioid, NULL, NULL, NULL);
  free(compdof);

#ifdef _PNETCDF
  format[num_flavors++] = PIO_IOTYPE_PNETCDF;
#endif
  format[num_flavors++] = PIO_IOTYPE_NETCDF;
#ifdef _NETCDF4
  format[num_flavors++] = PIO_IOTYPE_NETCDF4C;
  format[num_flavors++] = PIO_IOTYPE_NETCDF4P;
#endif
#ifdef _ADIOS
  format[num_flavors++] = PIO_IOTYPE_ADIOS;
#endif

  for (int fmt = 0; fmt < num_flavors; fmt++) {
    sprintf(filename, "output_weirda_%d.nc", fmt);

    PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);
    PIOc_def_dim(ncid, DIM_NAME, (PIO_Offset)dim_len, &dimid);
    PIOc_def_var(ncid, VAR_NAME, PIO_INT, 1, &dimid, &varid);
    PIOc_enddef(ncid);

    PIOc_write_darray(ncid, varid, ioid, elements_on_this_rank, buffer, NULL);
    PIOc_sync(ncid);

    PIOc_closefile(ncid);
  }

  free(buffer);

  PIOc_freedecomp(iosysid, ioid);

  PIOc_finalize(iosysid);

  MPI_Finalize();

#ifdef TIMING
  GPTLfinalize();
#endif 

  return 0;
}
