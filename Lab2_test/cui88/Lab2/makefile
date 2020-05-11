bin=httpserver
cc=g++ -std=c++11
LDFLAGS=-lpthread 

.PHONY:all
all:$(bin) 

$(bin):httpserver.cc
	$(cc) -o $@ $^ $(LDFLAGS) 

.PHONY:clean
clean:
	rm -f $(bin) 
