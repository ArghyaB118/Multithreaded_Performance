#!/bin/bash
set -ex
mkdir -p executables
g++  -std=c++11 -pthread ./parallel_mm.cpp -o ./executables/parallel_mm
chmod a+x ./executables/parallel_mm
g++ ./make-mm-data.cpp -o ./executables/make-mm-data
chmod a+x ./executables/make-mm-data

declare -a matrixwidth=( 8192 512 1024 2048 4096 16384 )
declare -a startingmemory=( 16 16 16 16 16 16 )

NUMRUNS=3
NUMINSTANCE=2


#creating nullbytes
mkdir -p data_files
for i in `seq 1 $NUMINSTANCE`;
do
	if [ ! -f "data_files/nullbytes$i" ]
	then
	  echo "First creating file for storing data."
	  dd if=/dev/urandom of=data_files/nullbytes$i count=32768 bs=1048576
	fi
done

#deleting out-sorting.txt and creating again
if [ -f "out-mm.txt" ]
then
  echo "out-mm.txt already exists. Deleting it first."
  rm out-mm.txt && touch out-mm.txt
fi

if [ -f "log.txt" ]
then
  echo "log.txt already exists. Deleting it first."
  rm log.txt && touch log.txt
fi



for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#matrixwidth[@]}-1; index++ ));
	do
		MATRIXWIDTH=${matrixwidth[$index]}
		STARTINGMEMORY_MB=${startingmemory[$index]}
		TOTALMEMORY=$((STARTINGMEMORY_MB*1024*1024))

		#code for constant memory profile funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "MM-SCAN sequential instances with $MATRIXWIDTH and $STARTINGMEMORY_MB" >> out-mm.txt 
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 1 $MATRIXWIDTH data_files/nullbytes1 1
		sleep 5
		wait

		#code for constant memory profile funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 1 $MATRIXWIDTH data_files/nullbytes1 4
		sleep 5
		wait


		# #code for constant memory profile funnel sort
		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo "MM-SCAN concurrent instances with $MATRIXWIDTH and $STARTINGMEMORY_MB" >> out-mm.txt 
		# echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/parallel_mm 1 $MATRIXWIDTH data_files/nullbytes1 1 &
		# cgexec -g memory:cache-test-arghya ./executables/parallel_mm 1 $MATRIXWIDTH data_files/nullbytes2 4
		# sleep 5
		# wait

		#code for constant memory profile funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "MM-SCAN concurrent instances with $MATRIXWIDTH and $STARTINGMEMORY_MB" >> out-mm.txt 
		echo $((2*TOTALMEMORY)) > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 1 $MATRIXWIDTH data_files/nullbytes2 1 &
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 1 $MATRIXWIDTH data_files/nullbytes1 4
		sleep 5
		wait

		#code for constant memory profile funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "MM-INPLACE sequential instances with $MATRIXWIDTH and $STARTINGMEMORY_MB" >> out-mm.txt 
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 0 $MATRIXWIDTH data_files/nullbytes1 1
		sleep 5
		wait

		#code for constant memory profile funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 0 $MATRIXWIDTH data_files/nullbytes1 4
		sleep 5
		wait


		# #code for constant memory profile funnel sort
		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo "MM-INPLACE concurrent instances with $MATRIXWIDTH and $STARTINGMEMORY_MB" >> out-mm.txt 
		# echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/parallel_mm 0 $MATRIXWIDTH data_files/nullbytes1 1 &
		# cgexec -g memory:cache-test-arghya ./executables/parallel_mm 0 $MATRIXWIDTH data_files/nullbytes2 4
		# sleep 5
		# wait

		#code for constant memory profile funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
		./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "MM-INPLACE concurrent instances with $MATRIXWIDTH and $STARTINGMEMORY_MB" >> out-mm.txt 
		echo $((2*TOTALMEMORY)) > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 0 $MATRIXWIDTH data_files/nullbytes2 1 &
		cgexec -g memory:cache-test-arghya ./executables/parallel_mm 0 $MATRIXWIDTH data_files/nullbytes1 4
		sleep 5
		wait
	done
done

