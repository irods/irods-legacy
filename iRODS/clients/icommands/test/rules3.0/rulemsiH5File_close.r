myTestRule {
#Input parameter is:
#  HDF5 input file
#  HDF5 output file
  msiH5File_open(*Path, *Flag, *Hdf5file);
  msiH5Dataset_read(*Hdf5file,*H5dataset);
  msiH5File_close(*Hdf5file,*H5dataset);
}
INPUT *Path="/tempZone/home/rods/hdf5/hdfTestFile",*Flag="1"
OUTPUT ruleExecOut
