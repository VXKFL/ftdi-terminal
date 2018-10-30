#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ftd2xx.h"
#include <termios.h>

// zero is the default stdin file descriptor (fd) / socket
#define FD 0
#define COVER_COMMAND true
#define FTDI_RETURN '\r'

// bool check_for_input(void) {
// 	int count;
// 	if(ioctl(FD, FIONREAD, &count) < 0) return false;
// 	if(count) return true;
// 	return false;
// }
//
// char* read_from_console(void) {
// 	static char buffer[256];
// 	int i = read(FD, buffer, 256 - 1);
// 	buffer[i-1] = 0;
// 	return buffer;
// }

bool to_ftdi(void* ft_handle, std::string s) {
	unsigned int written;
	bool status = FT_Write(ft_handle, (void*) s.data(), s.length(), &written);
	if(written != s.length() || status) return true;
	return 0;
}

char* read_ftdi(void* ft_handle) {
	static char buffer[512];
	unsigned int rx_queue, tx_queue, status, returned;
	FT_GetStatus(ft_handle, &rx_queue, &tx_queue, &status);
	FT_Read(ft_handle, (void*) buffer, rx_queue, &returned);
	buffer[rx_queue] = 0;

	for(int i = 0; i < rx_queue; i++) {
		if(buffer[i] == FTDI_RETURN) buffer[i] = '\n';
	}

	return buffer;
}

int main(int argc, char* argv[]) {

	// SETUP CONSOLE INPUT

	int flags = fcntl(FD, F_GETFL, 0);
	fcntl(FD, F_SETFL, flags | O_NONBLOCK);										// set stdin (FD = 0) input to non blocking mode

	struct termios old_tio, new_tio;
	unsigned char c;
	tcgetattr(STDIN_FILENO,&old_tio); 											// get the terminal settings for stdin
	new_tio=old_tio; 															// we want to keep the old setting to restore them a the end
	new_tio.c_lflag &=(~ICANON & ~ECHO); 										// disable canonical mode (buffered i/o) and local echo
	tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);									// set the new settings immediately

	// END SETUP CONSOLE INPUT

	// SETUP FTDI

	void* ft_handle;
    unsigned int ft_status;
    unsigned int num_devs;

	ft_status = FT_ListDevices(&num_devs, NULL, FT_LIST_NUMBER_ONLY);								if(!ft_status && num_devs == 1) std::cout << num_devs << " device found." << std::endl;
	 																								else if (!ft_status) std::cout << num_devs << " devices found." << std::endl;
																									else std::cout << "Error: " << ft_status << std::endl;
	ft_status = FT_Open(0, &ft_handle);																if(!ft_status) std::cout << "Device opened."; 			else std::cout << "Error: " << ft_status << " ";
	ft_status = FT_SetBaudRate(ft_handle, 1000000);													if(!ft_status) std::cout << " Baudrate set."; 			else std::cout << "Error: " << ft_status << " ";
	ft_status = FT_SetDataCharacteristics(ft_handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);	if(!ft_status) std::cout << " Bitconfig set."; 			else std::cout << "Error: " << ft_status << " ";
	ft_status = FT_SetFlowControl(ft_handle, FT_FLOW_NONE, 0, 0);									if(!ft_status) std::cout << " FlowControl set."; 		else std::cout << "Error: " << ft_status << " ";
																									std::cout << std::endl;
	// END SETUP FTDI

	// do {
	// 	c = getchar();
	// 	if(c != 255)printf("%i ", c);
	//
	// } while(c != 'q');

	char repeat = -1;
	while(true) {
		std::cout << read_ftdi(ft_handle) << std::flush;
		c=getchar();
		if(c != 255) {
			std::string s;
			while(c != '\n' && repeat) {										// repeat till enter or escape is pressed
				switch(c) {
					case 127: std::cout << "\b \b"; if(s.length()) s.pop_back(); break;
					case 27: 													// crappy programmed but working: after 27 are usually some characters following created by for example the arrow keys
						repeat = -1;
						do { c = getchar(); repeat++; } while(c != 255);
						break;
					default: std::cout << c; s.push_back(c);
				}
				do c = getchar(); while(c == 255 && repeat);					// wait for next input
			}

			if(COVER_COMMAND) {
				std::cout << "\r";
				for(int i = 0; i < s.length(); i++) std::cout << " ";
				std::cout << "\r" << std::flush;
			} else std::cout << std::endl;

			s.push_back(FTDI_RETURN);

			// for(int i = 0; i < s.length(); i++) {
			// 	std::cout << (int) s[i] << " ";
			// }

			// std::cout << std::endl;

			if(!repeat) break;
			if(to_ftdi(ft_handle, s)) {
				std::cout << "Error: Writing to ftdi failed." << std::endl;
			}
			// sleep(1);
		}
	}


	// REST CONSOLE SETTINGS

	tcsetattr(STDIN_FILENO,TCSANOW,&old_tio); 									// restore the former settings
	fcntl(FD, F_SETFL, flags & ~O_NONBLOCK);
	FT_Close(ft_handle);

	// END REST CONSOLE SETTINGS

	return 0;
}
