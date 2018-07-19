#include <stdio.h>
#include <mpi.h>
#include <pio.h>
#ifdef TIMING
#include <gptl.h>
#endif

#define NUM_NETCDF_FLAVORS 5
#define NDIM 1
#define DIM_LEN 16
#define DIM_NAME "x"
#define VAR_NAME "foo"
#define START_DATA_VAL 42

int main(int argc, char* argv[])
{
  int my_rank;
  int ntasks;
  int format[NUM_NETCDF_FLAVORS];
  int niotasks;
  int ioproc_stride = 1;
  int ioproc_start = 0;
  int dimid;
  PIO_Offset elements_per_pe;
  int dim_len[1] = {DIM_LEN};
  int iosysid;
  int ncid;
  int varid;
  int ioid;
  double *buffer = NULL;
  PIO_Offset *compdof = NULL;
  char filename[NC_MAX_NAME + 1];
  int num_flavors = 0;

#ifdef TIMING    
  GPTLinitialize();
#endif   

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

  if (!(ntasks == 1 || ntasks == 2 || ntasks == 4 ||
        ntasks == 8 || ntasks == 16))
    fprintf(stderr, "Number of processors must be 1, 2, 4, 8, or 16!\n");

  niotasks = ntasks; 
  PIOc_Init_Intracomm(MPI_COMM_WORLD, niotasks, ioproc_stride, ioproc_start, PIO_REARR_SUBSET, &iosysid);

  elements_per_pe = DIM_LEN / ntasks;
  compdof = malloc(elements_per_pe * sizeof(PIO_Offset));
  
  for (int i = 0; i < elements_per_pe; i++)
    compdof[i] = my_rank * elements_per_pe + i + 1;
	
  PIOc_InitDecomp(iosysid, PIO_DOUBLE, NDIM, dim_len, (PIO_Offset)elements_per_pe, compdof, &ioid, NULL, NULL, NULL);
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
    sprintf(filename, "output_typeconvert_%d.nc", fmt);

    PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);
    PIOc_def_dim(ncid, DIM_NAME, (PIO_Offset)dim_len[0], &dimid);
    PIOc_def_var(ncid, VAR_NAME, PIO_INT, NDIM, &dimid, &varid);
    PIOc_enddef(ncid);

    buffer = malloc(elements_per_pe * sizeof(double));
    for (int i = 0; i < elements_per_pe; i++)
      buffer[i] = START_DATA_VAL + my_rank;

    PIOc_write_darray(ncid, varid, ioid, (PIO_Offset)elements_per_pe, buffer, NULL);
    PIOc_sync(ncid);

    free(buffer);
	
    PIOc_closefile(ncid);
  }
	
  PIOc_freedecomp(iosysid, ioid);

  PIOc_finalize(iosysid);

  MPI_Finalize();

#ifdef TIMING    
  GPTLfinalize();
#endif 

  return 0;
}
