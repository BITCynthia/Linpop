# Linpop makefile

OBJS = linpop.o chat_dlg.o client_socket.o linpopwnd.o treeview.o user_info_set.o
CFLAGS = `pkg-config gtk+-2.0 --cflags`
LIBS = `pkg-config gtk+-2.0 --libs` -lpthread
CC = gcc
EXEC = Linpop

all: $(EXEC)
	
$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(CFLAGS) $(LIBS)
	rm -f *.o

clean: all
	rm -f $(EXEC) *.o

# end of makefile
