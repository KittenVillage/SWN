/*
 * sphere_flash_io.c - Spherical Wavetable Navigator I/O with external FLASH memory
 *
 * Author: Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */

#include "globals.h"
#include "sphere_flash_io.h"
#include "sphere.h"
#include "spheres_internal.h"

#include "drivers/flash_S25FL127.h"
#include "math_util.h"
#include "timekeeper.h"

#include "external_flash_layout.h"

const uint32_t 	WT_SIZE = sizeof(o_waveform)*WT_DIM_SIZE*WT_DIM_SIZE*WT_DIM_SIZE;


char	user_sphere_signature[4];
char	factory_sphere_signature[4];

enum SphereTypes sphere_types[MAX_TOTAL_SPHERES];

void init_sphere_flash(void)
{
	user_sphere_signature[0]='U';
	user_sphere_signature[1]='S';
	user_sphere_signature[2]='1';
	user_sphere_signature[3]='\0';

	factory_sphere_signature[0]='F';
	factory_sphere_signature[1]='S';
	factory_sphere_signature[2]='1';
	factory_sphere_signature[3]='\0';
}

void write_fatory_spheres_to_extflash(void)
{
	#ifndef SKIP_FACTORY_SPHERES_IN_HEXFILE
	uint32_t wt_num;

	for (wt_num=0;wt_num<NUM_FACTORY_SPHERES;wt_num++)
	{
		save_sphere_to_flash(wt_num, SPHERE_TYPE_FACTORY, (int16_t *)wavetable_list[wt_num]);
	}
	#endif
}

uint32_t get_wt_addr(uint16_t wt_num)
{
	if (wt_num >= MAX_TOTAL_SPHERES)
		wt_num = (MAX_TOTAL_SPHERES-1);

	return (sFLASH_get_sector_addr(WT_SECTOR_START + wt_num));
}	


// load_extflash_wavetable() must only be called within the WT_INTERP timer interrupt.
// Calling pause_timer_IRQ(WT_INTERP_TIM_number) pauses periodic reading from flash. 
// If you have to call it from somewhere else, then you must check and prevent flash collisions
// such as requesting a write while it's already reading.
// 
void load_extflash_wavetable(uint8_t wt_num, o_waveform *waveform, uint8_t x, uint8_t y, uint8_t z)
{
	uint32_t base_addr = get_wt_addr(wt_num);
	uint32_t addr;

	x = _CLAMP_U8(x,0,2);
	y = _CLAMP_U8(y,0,2);
	z = _CLAMP_U8(z,0,2);

	//calculate where the waveform is within the sphere
	addr = base_addr + sizeof(user_sphere_signature) + ((x + (y*WT_DIM_SIZE) + (z*WT_DIM_SIZE*WT_DIM_SIZE)) * SPHERE_WAVEFORM_SIZE);

	//sFLASH_read_buffer((uint8_t *)(waveform->name), addr, WT_NAME_MONITOR_CHARSIZE);

	addr += WT_NAME_MONITOR_CHARSIZE;

	sFLASH_read_buffer((uint8_t *)(waveform->wave), addr, WT_TABLELEN*BYTEDEPTH);
}

// Write to flash

void save_sphere_to_flash(uint8_t wt_num, enum SphereTypes sphere_type, int16_t *sphere_data){

	uint32_t sz;
	uint32_t base_addr = get_wt_addr(wt_num);

	pause_timer_IRQ(WT_INTERP_TIM_number);

	sFLASH_erase_sector(base_addr);

	//Write signature
	sz = 4;
	if (sphere_type == SPHERE_TYPE_USER)
		sFLASH_write_buffer((uint8_t *)user_sphere_signature, base_addr, sz);
	else
	if (sphere_type == SPHERE_TYPE_FACTORY)
		sFLASH_write_buffer((uint8_t *)factory_sphere_signature, base_addr, sz);
	else 
		return; //error, bad sphere_type

	base_addr += sz;

	sFLASH_write_buffer((uint8_t *)sphere_data, base_addr, WT_SIZE);

	resume_timer_IRQ(WT_INTERP_TIM_number);

	sphere_types[wt_num] = sphere_type;
}

enum SphereTypes read_spheretype(uint32_t wt_num)
{
	uint32_t addr = get_wt_addr(wt_num);
	uint32_t sz;
	char	read_data[4];

	pause_timer_IRQ(WT_INTERP_TIM_number);

	sz = 4;
	sFLASH_read_buffer((uint8_t *)read_data, addr, sz);

	resume_timer_IRQ(WT_INTERP_TIM_number);

	if (   read_data[0] == user_sphere_signature[0] 
		&& read_data[1] == user_sphere_signature[1] 
		&& read_data[2] == user_sphere_signature[2] 
		&& read_data[3] == user_sphere_signature[3] )
		return SPHERE_TYPE_USER;
	else
	if (   read_data[0] == factory_sphere_signature[0] 
		&& read_data[1] == factory_sphere_signature[1] 
		&& read_data[2] == factory_sphere_signature[2] 
		&& read_data[3] == factory_sphere_signature[3] )
		return SPHERE_TYPE_FACTORY;
	else
		return SPHERE_TYPE_EMPTY;

}

//Clear sphere signatures for user spheres
void quick_clear_user_spheres(void)
{
	uint32_t addr;
	uint32_t sz;
	char	read_data[4];
	uint8_t wt_num;

	pause_timer_IRQ(WT_INTERP_TIM_number);

	for (wt_num=0; wt_num<MAX_TOTAL_SPHERES; wt_num++)
	{
		sz = 4;
 		addr = get_wt_addr(wt_num);
 		sFLASH_read_buffer((uint8_t *)read_data, addr, sz);

		if ((sphere_types[wt_num]==SPHERE_TYPE_USER) ||
			(read_data[0] == user_sphere_signature[0] && read_data[1] == user_sphere_signature[1] && read_data[2] == user_sphere_signature[2] && read_data[3] == user_sphere_signature[3]))
		{
			read_data[0] = 0x00;
			read_data[1] = 0x00;
			read_data[2] = 0x00;
			read_data[3] = 0x00;
			sFLASH_write_buffer((uint8_t *)read_data, addr, sz);

			sphere_types[wt_num] = SPHERE_TYPE_EMPTY;
		}
	}
	resume_timer_IRQ(WT_INTERP_TIM_number);
}

void read_all_spheretypes(void){

	uint8_t i;

	for (i=0; i< MAX_TOTAL_SPHERES; i++){
		sphere_types[i] = read_spheretype(i);
	}	
}

uint8_t is_sphere_filled(uint8_t wt_num){
	if (sphere_types[wt_num] != SPHERE_TYPE_EMPTY) return 1;
	else return 0;
}

uint8_t is_spheretype_factory(uint8_t wt_num){
	if 	(sphere_types[wt_num] == SPHERE_TYPE_FACTORY) return 1;
	else return 0;
}

uint8_t is_spheretype_user(uint8_t wt_num){
	if (sphere_types[wt_num] == SPHERE_TYPE_USER) return 1;
	else return 0;
}

uint8_t is_factory_sphere0_present(void){
	if (read_spheretype(0)==SPHERE_TYPE_FACTORY)
		return 1;
	else
		return 0;
}
