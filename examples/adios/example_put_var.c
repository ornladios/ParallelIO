#include <stdio.h>
#include <mpi.h>
#include <pio.h>
#ifdef TIMING
#include <gptl.h>
#endif

#define NUM_NETCDF_FLAVORS 5
#define DIM_LEN 16

int main(int argc, char* argv[])
{
  int my_rank;
  int ntasks;
  int format[NUM_NETCDF_FLAVORS];
  int niotasks;
  int ioproc_stride = 1;
  int ioproc_start = 0;
  int iosysid;
  int ncid;
  int dimid;
  int varid;
  char filename[NC_MAX_NAME + 1];
  int num_flavors = 0;
  int part_len = DIM_LEN / 4; /* Split into 4 parts */
  PIO_Offset start, count;
  int *buffer = NULL;

#ifdef TIMING    
  GPTLinitialize();
#endif   

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

  if (!(ntasks == 1 || ntasks == 2 || ntasks == 4 ||
        ntasks == 8 || ntasks == 16)) {
    if (my_rank == 0)
      fprintf(stderr, "Number of processors must be 1, 2, 4, 8, or 16!\n");
    MPI_Finalize();
    return 1;
  }

  niotasks = ntasks; 
  PIOc_Init_Intracomm(MPI_COMM_WORLD, niotasks, ioproc_stride, ioproc_start, PIO_REARR_SUBSET, &iosysid);

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

  buffer = malloc(DIM_LEN * sizeof(int));
  for (int i = 0; i < DIM_LEN; i++)
    buffer[i] = i;

  count = part_len;

  for (int fmt = 0; fmt < num_flavors; fmt++) {
    sprintf(filename, "output_put_var_%d.nc", fmt);

    PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);
    PIOc_def_dim(ncid, "dummy_dim_put_val", DIM_LEN, &dimid);
    PIOc_def_var(ncid, "dummy_var_put_val", PIO_INT, 1, &dimid, &varid);
    PIOc_enddef(ncid);

    /* Put values to the 1D variable (1st part) */
	int ii = 5;
	// for (int ii=0;ii<4;ii++) {
    	start = 0;
		buffer[0] = ii;
		PIOc_setframe(ncid, varid, ii);
    	PIOc_put_vara_int(ncid, varid, &start, &count, buffer);

    	/* Put values to the 1D variable (2nd part) */
    	start = part_len;
    	PIOc_put_vara_int(ncid, varid, &start, &count, buffer + start);

    	/* Put values to the 1D variable (3rd part) */
    	start = 2 * part_len;
    	PIOc_put_vara_int(ncid, varid, &start, &count, buffer + start);

    	/* Put values to the 1D variable (4th part) */
    	start = 3 * part_len;
    	PIOc_put_vara_int(ncid, varid, &start, &count, buffer + start);
	// }

    PIOc_closefile(ncid);
  }

  free(buffer);

  PIOc_finalize(iosysid);

  MPI_Finalize();

#ifdef TIMING    
  GPTLfinalize();
#endif 

  return 0;
}
