#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../shared/ioct_driver.h"

#include <sys/ioctl.h>

int main() 
{
	int fd;
	char buf_wr[100];
	char buf_read[100];
	fd = open("/dev/foo", O_RDWR);
	if (fd < 0 )
	{
		printf("Can not open /dev/foo fd < 0 \n");
		return (0);
	}
	printf("open /dev/foo sucsesful  %d\n", fd);
	
	int buffer_flag;
	int res_read = 0;
	/*printf("------------------ READ --------------------\n");
	

	ioctl(fd, IOC_GET, &buffer_flag);
	printf("buffer_flag = %d\n ", buffer_flag);

	res_read = read(fd,buf_read,sizeof(buf_read));
	printf("read byte 1 = %d\n", res_read);
	printf("READ: = %s", buf_read);
*/

	//printf("------------------ WRITE --------------------\n");
	ioctl(fd, IOC_GET, &buffer_flag);
	printf(">>>>>>>> buffer_flag = %d\n ", buffer_flag);

	//memcpy(buf_wr, "test read from sym device", 100);
	//printf("sizeof(buf) %ld\n", sizeof(buf_wr));
	//printf("WRITE: >>> %s <<<\n", buf_wr);
	//int res_wr = write(fd, buf_wr, sizeof(buf_wr));
	//printf("res_wr = %d\n", res_wr);

    sleep(2);
    
	//printf("------------------ READ --------------------\n");
	ioctl(fd, IOC_GET, &buffer_flag);
	//printf(">>>>>>>>  buffer_flag = %d\n ", buffer_flag);
	//res_read = read(fd,buf_read,sizeof(buf_read));
	//printf("read byte 2 = %d\n", res_read);
	//printf("READ: >>> %s <<<\n", buf_read);
	close(fd);
}
