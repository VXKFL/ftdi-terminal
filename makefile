TARGET=ftdi-terminal

$(TARGET): main.o
	g++ -o $(TARGET) main.cpp -L. -lftd2xx -Wl,-rpath,/usr/local/lib -lpthread

main.o: main.cpp
	g++ -c main.cpp

clear:
	rm *.o $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)
