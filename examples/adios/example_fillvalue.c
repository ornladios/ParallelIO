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
  int elements_on_this_rank = 8;
  int dim_len;
  int iosysid;
  int ncid;
  int varid;
  int ioid;
  int *buffer = NULL;
  PIO_Offset *compdof = NULL;
  char filename[NC_MAX_NAME + 1];
  int num_flavors = 0;
  int int_fillvalue = -2; /* The buffer fillvalue to be used when writing data with holes */
  void *fillvalue = &int_fillvalue; /* fillvalue pointer to be passed to PIOc_write_darray */

#ifdef TIMING    
  GPTLinitialize();
#endif   

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

  if (ntasks != 4) {
    if (my_rank == 0)
      fprintf(stderr, "Number of processors must be 4!\n");
    MPI_Finalize();
    return 1;
  }

  buffer = malloc(elements_on_this_rank * sizeof(int));
  compdof = malloc(elements_on_this_rank * sizeof(PIO_Offset));
  for (int i = 0; i < elements_on_this_rank; i++) {
    buffer[i] = (my_rank * elements_on_this_rank) + (i + 1);
    compdof[i] = (my_rank * elements_on_this_rank) + (i + 1);
  }

  /* PIO decomposition: [0, 2, 3, 4, 5, 6, 7, 8] [0, 10, 0, 12, 13, 14, 15, 16]
                        [0, 18, 0, 20, 0, 22, 23, 24] [0, 26, 0, 28, 0, 30, 0, 32]
  */
    if (my_rank == 0) {
      /* One hole */
      compdof[0] = 0;
    }
    else if (my_rank == 1) {
      /* Two holes with stride 2 */
      compdof[0] = 0;
      compdof[2] = 0;
    }
    else if (my_rank == 2) {
      /* Three holes with stride 2 */
      compdof[0] = 0;
      compdof[2] = 0;
      compdof[4] = 0;
    }
    else if (my_rank == 3) {
      /* Four holes with stride 2 */
      compdof[0] = 0;
      compdof[2] = 0;
      compdof[4] = 0;
      compdof[6] = 0;
    }

  dim_len = 32;

  niotasks = 4;

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
    sprintf(filename, "output_fillvalue%d.nc", fmt);

    PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);
    PIOc_def_dim(ncid, DIM_NAME, (PIO_Offset)dim_len, &dimid);
    PIOc_def_var(ncid, VAR_NAME, PIO_INT, 1, &dimid, &varid);
    PIOc_enddef(ncid);

    PIOc_write_darray(ncid, varid, ioid, elements_on_this_rank, buffer, fillvalue);
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
