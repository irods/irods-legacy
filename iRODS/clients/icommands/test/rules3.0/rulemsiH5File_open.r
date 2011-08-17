myTestRule {
#Input parameters are: 
#  HDF5 input file
#  Flag specifying level of verboseness for metadata
#    1 - verbose
#    2 - very verbose
#Output parameter is:
#  HDF5 output file
  msiH5File_open(*Path, *Flag, *Hdf5file);
  msiH5Dataset_read(*Hdf5file,*H5dataset);
  msiH5File_close(*Hdf5file,*H5dataset);
}
INPUT *Path="/tempZone/home/rods/hdf5/hdfTestFile",*Flag="1"
OUTPUT ruleExecOut
