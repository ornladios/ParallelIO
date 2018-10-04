#include <stdio.h>
#include <mpi.h>
#include <pio.h>
#ifdef TIMING
#include <gptl.h>
#endif

#define NUM_NETCDF_FLAVORS 5

int main(int argc, char* argv[])
{
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
  char char_data = 'D';
  int int_data = 5;

#ifdef TIMING    
  GPTLinitialize();
#endif   

  MPI_Init(&argc, &argv);
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
#ifdef _ADIOS2
  format[num_flavors++] = PIO_IOTYPE_ADIOS;
#endif

  for (int fmt = 0; fmt < num_flavors; fmt++) {
    sprintf(filename, "output_scalar_char_var_%d.nc", fmt);

    PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);

    PIOc_def_var(ncid, "dummy_scalar_char_var", PIO_CHAR, 0, NULL, &varid);
    // PIOc_def_var(ncid, "dummy_scalar_int_var", PIO_INT, 0, NULL, &varid);

    PIOc_enddef(ncid);

    PIOc_put_var_text(ncid, varid, &char_data);
    //PIOc_put_var_int(ncid, varid, &int_data);

    PIOc_closefile(ncid);
  }

  PIOc_finalize(iosysid);

  MPI_Finalize();

#ifdef TIMING    
  GPTLfinalize();
#endif 

  return 0;
}
