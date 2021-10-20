#include "Si468x.h"
#include "Si468x_ext.h"
#include "Si468x_platform.h"
#include "string.h"
#include "stdlib.h"
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

int Si468x_start_image(uint8_t* btldr_image, uint32_t btldr_size, uint8_t* app_image, uint32_t app_size)
{
	int ret_val;
	
	// Reset the tuner and wait for 50ms before loading the new image
	Si468x_gpio_assert_reset();
	usleep(5000);
	// Take the tuner out of reset and wait for 3ms
	Si468x_gpio_deassert_reset();
	usleep(3000);
	// Send power-up and then wait 20us
	ret_val = Si468x_powerup();
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	usleep(20);
	// Begin firmware loading phase
	ret_val = Si468x_load_init();
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	// Send the bootloader image
	ret_val = Si468x_host_load(btldr_image, btldr_size);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	// Wait for 4ms
	usleep(4000);
	// Begin firmware loading phase
	ret_val = Si468x_load_init();
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	// Send the application image (DAB)
	ret_val = Si468x_host_load(app_image, app_size);
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	// Wait for 4ms
	usleep(4000);
	// Boot the image
	ret_val = Si468x_boot();
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}

	usleep(400000);

	return SI468X_SUCCESS;
}

int Si468x_init()
{
	int ret_val;
	
	ret_val = Si468x_platform_init();
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	return 0;
}

int Si468x_deinit()
{
	int ret_val;
	
	Si468x_gpio_assert_reset();
	
	ret_val = Si468x_platform_deinit();
	if (ret_val < 0) {
		printf("Error (%d) in function %s line %d\n", ret_val, __FUNCTION__, __LINE__);
		return ret_val;
	}
	
	return 0;
}

int Si468x_get_dab_service_list(struct dab_service_list* output)
{
	int ret;
	uint8_t service_index, component_index;
	
	// clear all data from the input structure
	for (service_index = 0; service_index < MAX_DAB_SERVICES_PER_ENSAMBLE+1; service_index++) {
		output->service_list[service_index] = NULL;
		for (component_index = 0; component_index < MAX_DAB_COMPONENTS_PER_SERVICE+1; component_index++) {
			output->components_list[service_index][component_index] = NULL;
		}
	}
	if (output->raw_service_list != NULL)
		free(output->raw_service_list);
	
	// get new data
	ret = Si468x_dab_get_digital_service_list(&(output->header), &(output->raw_service_list));
	if (ret < 0)
		return ret;
	
	// fill the input structure
	uint8_t* buff_ptr = output->raw_service_list;
	struct Si468x_DAB_digital_service* tmp_service_ptr;
	
	for (service_index = 0; service_index < output->header.num_of_services; service_index++) {
		tmp_service_ptr = (struct Si468x_DAB_digital_service*)buff_ptr;
		output->service_list[service_index] = tmp_service_ptr;
		buff_ptr += sizeof(struct Si468x_DAB_digital_service);

		uint8_t number_of_components = get_service2_NUM_COMP(tmp_service_ptr->service_info_2);
		for (component_index = 0; component_index < number_of_components; component_index++) {
			output->components_list[service_index][component_index] = (struct Si468x_DAB_digital_service_component*)buff_ptr;
			buff_ptr += sizeof(struct Si468x_DAB_digital_service_component);
		}
	}
	
	return 0;
}
