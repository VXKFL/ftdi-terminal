TARGET=ftdi-terminal
DIR=build

$(DIR)/$(TARGET): $(DIR) main.cpp
	g++ -o $(DIR)/$(TARGET) main.cpp -L. -lftd2xx -Wl,-rpath,/usr/local/lib -lpthread

$(DIR):
	mkdir $(DIR)

.PHONY: clear
.PHONY: run

clear:
	rm -Rf build $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)
