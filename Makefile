CC      = g++
CFLAGS  = -Wall -O2 -std=c++11 -Werror

fs: fs.o FileSystem.o check.o 
	$(CC) ${CFLAGS} -o fs fs.o FileSystem.o check.o
 
complie: fs.o FileSystem.o

fs.o: fs.cc
	${CC} ${CFLAGS} -c fs.cc

FileSystem.o: FileSystem.cc FileSystem.h
	${CC} ${CFLAGS} -c FileSystem.cc
check.o: check.cc check.h
	${CC} ${CFLAGS} -c check.cc


clean:
	rm *.o
	rm ./fs
	

	
memory:
	valgrind --tool=memcheck --leak-check=yes ./fs

compress:
	zip fs-sim.zip FileSystem.cc FileSystem.h check.cc check.h readme.md Makefile

run1:
	./create_fs disk1
	clear
	./fs input1 > stdout 2>stderr
	diff disk1 ../demo/sample_tests/sample_test_1/disk1_result
	diff stdout ../demo/sample_tests/sample_test_1/stdout
	diff stderr ../demo/sample_tests/sample_test_1/stderr

run2:
	./create_fs disk1
	clear
	./fs input2 > stdout 2>stderr
	diff stdout ../demo/sample_tests/sample_test_2/stdout
	diff stderr ../demo/sample_tests/sample_test_2/stderr
	diff disk1 ../demo/sample_tests/sample_test_2/disk1_result


run3:
	./create_fs disk1
	./create_fs disk2
	clear
	./fs input3 > stdout 2>stderr
	diff stdout ../demo/sample_tests/sample_test_3/stdout
	diff stderr ../demo/sample_tests/sample_test_3/stderr
	diff disk1 ../demo/sample_tests/sample_test_3/disk1_result
	diff disk2 ../demo/sample_tests/sample_test_3/disk2_result

run4:
	./create_fs clean_disk
	clear
	./fs trivial-input > stdout 2>stderr
	diff stdout ../demo/sample_tests/sample_test_4/stdout
	diff stderr ../demo/sample_tests/sample_test_4/stderr
	diff clean_disk ../demo/sample_tests/sample_test_4/clean_disk_result

consistency:
	./fs input_consistency 2>consistency_stderr
	diff consistency_stderr ./consistency-check/sample-stderr


