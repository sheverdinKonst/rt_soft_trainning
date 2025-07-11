#include <stdio.h>	// for printf(), perror() 
#include <fcntl.h>	// for open() 
#include <stdlib.h>	// for exit() 
#include <sys/mman.h>	// for mmap(), munmap()


int main( int argc, char **argv ) {
	int	fd = open("/dev/nic", O_RDONLY ); // open the device-file
	if ( fd < 0 ) { printf("error open /dev/etx_device \n" ); exit(1); }
	int	size = 0x100; int	base = 0;
	void	*vm = mmap( NULL, size, PROT_READ, MAP_SHARED, fd, 0 ); 	// Отобразить io-memory 
        // устройства в user-space
	if ( vm == MAP_FAILED ) { perror( " error mmap /dev/nic " ); exit(1); }
	printf( "\n\nRealTek 8139 Registers mapped at %p \n", vm );// Отобразить регистры
	unsigned long 	*lp = (unsigned long *)vm;
	for (int i = 0; i < 64; i++) {
		if ( ( i % 8 ) == 0 ) 
			printf( "\n%02X: ", i*4 );          
		printf( "%08lX ", lp[ i ] );
	}
	printf( "\n\n\n" );
}
