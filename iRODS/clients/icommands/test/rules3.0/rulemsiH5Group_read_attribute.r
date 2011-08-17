myTestRule {
#Input parameter is:
#  HDF5 group parameter
#Output parameter is:
#  Group parameter read
  msiH5File_open(*Path, *Flag, *Hdf5file);
  msiH5Dataset_read(*Hdf5file,*H5dataset);
  msiH5Group_read(*Hdf5group,*H5groupval);
  msiH5File_close(*Hdf5file,*H5dataset);
}
INPUT *Path="/tempZone/home/rods/hdf5/hdfTestFile",*Flag="1"
OUTPUT ruleExecOut
