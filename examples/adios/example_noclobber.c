#include <stdio.h>
#include <mpi.h>
#include <pio.h>
#include <assert.h>
#ifdef TIMING
#include <gptl.h>
#endif

#define NUM_NETCDF_FLAVORS 5

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
  char filename[NC_MAX_NAME + 1];
  int num_flavors = 0;
  int ret;

#ifdef TIMING    
  GPTLinitialize();
#endif   

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

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

  PIOc_set_iosystem_error_handling(iosysid, PIO_RETURN_ERROR, NULL);

  for (int fmt = 0; fmt < num_flavors; fmt++) {
    sprintf(filename, "output_noclobber_%d.nc", fmt);

    ret = PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);
    assert(ret == PIO_NOERR);

    ret = PIOc_closefile(ncid);
    assert(ret == PIO_NOERR);

    /* Since the file already exists (created above) and PIO_NOCLOBBER is used, an error code should be returned */
    /* For ADIOS type, no error code is returned so far */
    ret = PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_NOCLOBBER);
    if (my_rank == 0) {
      if (ret != PIO_NOERR)
        printf("\nCreate file with clobber then no clobber failed as expected, returned code is %d, format is %d\n", ret, fmt);
      else
        printf("\nCreate file with clobber then no clobber did NOT failed as expected, returned code is %d, format is %d\n", ret, fmt);
    }

    /* No close since createfile should fail */
  }

  PIOc_finalize(iosysid);

  MPI_Finalize();

#ifdef TIMING    
  GPTLfinalize();
#endif 

  return 0;
}
