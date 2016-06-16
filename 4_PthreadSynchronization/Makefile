
CC = g++-4.9
CFLAGS = -O3

all: randtrack randtrack_global_lock randtrack_tm randtrack_list_lock randtrack_element_lock randtrack_reduction

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack

randtrack_global_lock: list.h hash.h defs.h randtrack_global_lock.cc
	$(CC) $(CLFAGS) randtrack_global_lock.cc -o randtrack_global_lock -lpthread
	
randtrack_tm: list.h hash.h defs.h 
	$(CC) $(CFLAGS) randtrack_tm.cc -o randtrack_tm -lpthread -fgnu-tm

randtrack_list_lock: list.h hash.h defs.h randtrack_list_lock.cc
	$(CC) $(CLFAGS) randtrack_list_lock.cc -o randtrack_list_lock -lpthread

randtrack_element_lock: list.h hash.h defs.h randtrack_element_lock.cc
	$(CC) $(CLFAGS) randtrack_element_lock.cc -o randtrack_element_lock -lpthread

randtrack_reduction: list.h hash.h defs.h randtrack_reduction.cc
	$(CC) $(CLFAGS) randtrack_reduction.cc -o randtrack_reduction -lpthread

clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock randtrack_element_lock randtrack_reduction