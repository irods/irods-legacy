myTestRule {
#Input parameter is:
#  HDF5 dataset parameter
#Output parameter is:
#  Parameter value
  msiH5File_open(*Path, *Flag, *Hdf5file);
  msiH5Dataset_read(*Hdf5Dataset,*H5dataset);
  msiH5Dataset_read_attribute(*H5parameter,*H5parvalue);
  msiH5File_close(*Hdf5file,*H5dataset);
}
INPUT *Path="/tempZone/home/rods/hdf5/hdfTestFile",*Flag="1"
OUTPUT ruleExecOut
