objects = main.c command.c

shell : $(objects)
	gcc -o shell $(objects) -g
.PHONY : clean
clean : 
	-rm shell
