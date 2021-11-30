#!/bin/bash
set -ex
mkdir -p executables
g++  -std=c++11 -pthread ./mm.cpp -o ./executables/parallel_mm
chmod a+x ./executables/parallel_mm
g++ ./make-mm-data.cpp -o ./executables/make-mm-data
chmod a+x ./executables/make-mm-data

#declare -a matrixwidth=( 512 1024 2048 4096 8192 )
#declare -a startingmemory=( 16 16 16 16 16 )
#declare -a algorithms=( 0 1 2 )
declare -a matrixwidth=( 2048 4096 )
declare -a startingmemory=( 16 16 )
declare -a algorithms=( 0 1 )


USERID=arghya
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
if [ -f "out_mm.csv" ]
then
  echo "out_mm.csv already exists. Deleting it first."
  rm out_mm.csv && touch out_mm.csv
fi

if [ -f "log.txt" ]
then
  echo "log.txt already exists. Deleting it first."
  rm log.txt && touch log.txt
fi

echo "Memory given per instance is ${startingmemory[0]} MiB" >> out_mm.csv 

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#matrixwidth[@]}-1; index++ ));
	do
		for (( k=0; k<=${#algorithms[@]}-1; k++ ))
		do
			algorithm=${algorithms[$k]}
			MATRIXWIDTH=${matrixwidth[$index]}
			STARTINGMEMORY_MB=${startingmemory[$index]}
			TOTALMEMORY=$((STARTINGMEMORY_MB*1024*1024))

			#code for constant memory profile funnel sort
			./cgroup_creation.sh cache-test-arghya $USERID
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 1
			sleep 5
			wait

			#code for constant memory profile funnel sort
			./cgroup_creation.sh cache-test-arghya $USERID
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 4
			sleep 5
			wait


			# #code for constant memory profile funnel sort
			# ./cgroup_creation.sh cache-test-arghya $USERID
			# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
			# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			# echo $((2*TOTALMEMORY)) > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			# cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 1 &
			# cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes2 cache-test-arghya 4
			# sleep 5
			# wait

			# #code for constant memory profile funnel sort
			# ./cgroup_creation.sh cache-test-arghya $USERID
			# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			# ./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
			# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			# echo $((2*TOTALMEMORY)) > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			# cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes2 cache-test-arghya 1 &
			# cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 4
			# sleep 5
			# wait

			#code for constant memory profile funnel sort
			./cgroup_creation.sh cache-test-arghya $USERID
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
			sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 1 &
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes2 cache-test-arghya 4
			sleep 5
			wait

			#code for constant memory profile funnel sort
			./cgroup_creation.sh cache-test-arghya $USERID
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
			sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes2 cache-test-arghya 1 &
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 4
			sleep 5
			wait

			#code for constant memory profile funnel sort
			./cgroup_creation.sh cache-test-arghya $USERID
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes1
			./executables/make-mm-data $((4*MATRIXWIDTH)) data_files/nullbytes2
			sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 1 &
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $((4*MATRIXWIDTH)) $STARTINGMEMORY_MB data_files/nullbytes2 cache-test-arghya 4
			sleep 5
			wait

			#code for constant memory profile funnel sort
			./cgroup_creation.sh cache-test-arghya $USERID
			./executables/make-mm-data $((4*MATRIXWIDTH)) data_files/nullbytes1
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes2
			sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
			echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $MATRIXWIDTH $STARTINGMEMORY_MB data_files/nullbytes2 cache-test-arghya 1 &
			cgexec -g memory:cache-test-arghya ./executables/parallel_mm $algorithm $((4*MATRIXWIDTH)) $STARTINGMEMORY_MB data_files/nullbytes1 cache-test-arghya 4
			sleep 5
			wait
		done
	done
done

