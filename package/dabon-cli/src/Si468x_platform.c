#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <Si468x.h>
#include <Si468x_platform.h>

//#define ENABLE_DEBUG

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)


#define GPIO_FOLDER_PATH 		"/sys/class/gpio"
#define GPIO_EXPORT_PATH		GPIO_FOLDER_PATH "/export"

#define RESET_PIN			236
#define RESET_DIR_PATH		GPIO_FOLDER_PATH "/gpio" STRINGIZE(RESET_PIN) "/direction"
#define RESET_VALUE_PATH	GPIO_FOLDER_PATH "/gpio" STRINGIZE(RESET_PIN) "/value"

#define INT_PIN				231
#define INT_DIR_PATH		GPIO_FOLDER_PATH "/gpio" STRINGIZE(INT_PIN) "/direction"
#define INT_VALUE_PATH		GPIO_FOLDER_PATH "/gpio" STRINGIZE(INT_PIN) "/value"

#define SPI_DEVICE		"/dev/spidev0.0"
#define SPI_SPEED		10000000
#define SPI_BITS		8
#define SPI_DELAY		0

int spi_fd;

int Si468x_spi_init()
{
	uint8_t mode = SPI_MODE_0;
	uint8_t bits = SPI_BITS;
	uint32_t speed = SPI_SPEED;

	spi_fd = open(SPI_DEVICE, O_RDWR);
	if (spi_fd < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}

	if (ioctl(spi_fd, SPI_IOC_RD_MODE, &mode) < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}
	if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}

	if (ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}
	if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}

	if (ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}
	if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		printf("error in function %s at line %d: %s\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

int Si468x_spi_close()
{
	return 0;
}

int Si468x_send_command(uint8_t* data, uint32_t size)
{
	struct spi_ioc_transfer spi_transfer;
	int ret;
	
	if (data == NULL) {
		return -1;
	}
	
	#ifdef ENABLE_DEBUG
	dump_array(data, size);
	#endif //ENABLE_DEBUG
	
	memset(&spi_transfer, 0, sizeof(spi_transfer));
	spi_transfer.tx_buf = (unsigned long) data;
	spi_transfer.rx_buf = (unsigned long) NULL;
	spi_transfer.len = size;
	
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer);
	if (ret < 0) {
		printf("SPI ioctl error (%d) in function %s\n", ret, __func__);
		return ret;
	}
	
	return 0;
}

#define SI468X_CMD_RD_REPLY				0x00
int Si468x_read_reply(uint8_t* data, uint32_t size, struct Si468x_status* status)
{
	struct spi_ioc_transfer spi_transfer[3];
	int transactions_count = 0;
	int ret; 
	uint8_t data_out = SI468X_CMD_RD_REPLY;
	
	memset(spi_transfer, 0, 3*sizeof(struct spi_ioc_transfer));
	
	spi_transfer[0].tx_buf = (unsigned long) &data_out;
	spi_transfer[0].rx_buf = (unsigned long) NULL;
	spi_transfer[0].len = sizeof(uint8_t);
	transactions_count++;
	
	spi_transfer[1].tx_buf = (unsigned long) NULL;
	spi_transfer[1].rx_buf = (unsigned long) status;
	spi_transfer[1].len = sizeof(struct Si468x_status);
	transactions_count++;
	
	if (data != NULL) {
		spi_transfer[2].tx_buf = (unsigned long) NULL;
		spi_transfer[2].rx_buf = (unsigned long) data;
		spi_transfer[2].len = size;
		transactions_count++;
	}
	
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(transactions_count), &spi_transfer);
	if (ret < 0) {
		printf("SPI ioctl error (%d) in function %s\n", ret, __func__);
		return ret;
	}
	
#ifdef ENABLE_DEBUG
	dump_array((uint8_t*)status, sizeof(struct Si468x_status));
	dump_array(data, size);
#endif //ENABLE_DEBUG
	
	return 0;
}

int Si468x_gpio_init()
{
	int ret;
	
	// RESET pin
	ret = sysfs_write_int(GPIO_EXPORT_PATH, RESET_PIN);
	if (ret < 0) {
		printf("Error: unable to write file %s", GPIO_EXPORT_PATH);
		return ret;
	}
	ret = sysfs_write_string(RESET_DIR_PATH, "out");
	if (ret < 0) {
		printf("Error: unable to write file %s", RESET_DIR_PATH);
		return ret;
	}
	
	// INT pin
	ret = sysfs_write_int(GPIO_EXPORT_PATH, INT_PIN);
	if (ret < 0) {
		printf("Error: unable to write file %s", GPIO_EXPORT_PATH);
		return ret;
	}
	
	return 0;
}

int Si468x_gpio_close() 
{
	return 0;
}

int Si468x_gpio_assert_reset()
{
#ifdef ENABLE_DEBUG
	printf("%s\n", __FUNCTION__);
#endif //ENABLE_DEBUG
	return sysfs_write_int(RESET_VALUE_PATH, 0);
}

int Si468x_gpio_deassert_reset()
{
#ifdef ENABLE_DEBUG
	printf("%s\n", __FUNCTION__);
#endif //ENABLE_DEBUG
	return sysfs_write_int(RESET_VALUE_PATH, 1);
}

uint8_t Si468x_gpio_get_int_status()
{
#ifdef ENABLE_DEBUG
	printf("%s\n", __FUNCTION__);
#endif //ENABLE_DEBUG
	int val;
	
	if (sysfs_read_int(RESET_VALUE_PATH, &val) < 0) {
		return 0;
	}
	return (!!val);
}

int Si468x_platform_init()
{
	Si468x_spi_init();
	Si468x_gpio_init();
	
	return 0;
}

int Si468x_platform_deinit()
{
	Si468x_gpio_close();
	Si468x_spi_close();
	
	return 0;
}
