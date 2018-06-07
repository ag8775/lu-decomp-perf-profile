Files Attached :

manishk.zip contains the following three sub directories

1.luSeq : this directory contains the sequential LU decomposition code. The matA of size [SIZE][SIZE] is stored either in DRAM or in Local memory. 

2.Cyclic : this directory contains the cyclic LU decomposition code. The matA is stored in DRAM. The data is transferred from DRAM to local matrix local_matA for processing. The data is transferred to /from DRAM to local memory by either PE’s or the MTE engine. Unmask the line [ # define USING_MTE] to use the MTE to transfer the data or vice-versa. 

3.block : : this directory contains the block LU decomposition code. The matA is stored in DRAM. The data is transferred from DRAM to local matrix local_matA for processing. The data is transferred to /from DRAM to local memory by either PE’s or the MTE engine. Mask the line [ # define USING_MTE] to use the PE to transfer the date or vice-versa. 

Each directory contains the make file and the control file. Each directory also contains the statistics files generated with different values of matA [SIZE][SIZE].
For example the file ../block/MTE/UMSStats_96.txt contains the statistics of  matA of Size 96 and local mat of size 24x96 and its block decomposition and the PE’s transfer the data. 
How to compile it: 

	Just run the make file in each directory and it will create the *.l file . 

**The submitted code always produces the correct LU decomposition for all values of Matrix size. However one strange observation is that it never produces the correct decomposition when profiling is disabled 