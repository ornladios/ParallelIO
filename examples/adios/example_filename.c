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

  PIOc_set_iosystem_error_handling(iosysid, PIO_RETURN_ERROR, NULL);

  for (int fmt = 0; fmt < num_flavors; fmt++) {
    sprintf(filename, "output_filename_%d.nc", fmt);

	printf("Opening file: %s by format: %d (%d) %d\n",filename,fmt,num_flavors,format[fmt]); fflush(stdout); 
    int ierr = PIOc_createfile(iosysid, &ncid, &(format[fmt]), filename, PIO_CLOBBER);
	printf("ERROR VALUE: %d\n",ierr);
	if (ierr!=PIO_NOERR) {
		printf("ERROR: File exists..\n"); fflush(stdout);
		MPI_Barrier(MPI_COMM_WORLD);
	} else {
    	PIOc_def_dim(ncid, "filename_dim", 100, &dimid);
    	PIOc_def_var(ncid, "filename", PIO_CHAR, 1, &dimid, &varid);

    	PIOc_put_att(ncid, varid, "filename_att", PIO_CHAR, strlen(filename), filename);
		char filname[256];
		sprintf(filname,"testing_attributes");
    	PIOc_put_att(ncid, varid, "filename_test", PIO_CHAR, strlen(filname), filname);
		int test_val = 3;
		PIOc_put_att(ncid, varid, "test_val", PIO_INT, 1, &test_val);
    	PIOc_enddef(ncid);

		// PIOc_put_var_text(ncid,varid,"TESTING");

    	PIOc_closefile(ncid);
	}
	fflush(stdout);
	MPI_Barrier(MPI_COMM_WORLD);
  }

  PIOc_finalize(iosysid);

  MPI_Finalize();

#ifdef TIMING    
  GPTLfinalize();
#endif 

  return 0;
}
