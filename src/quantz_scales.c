/*
 * quantz_scales.c
 *
 * Author: Dan Green (danngreen1@gmail.com)
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

#include "quantz_scales.h"
#include "globals.h"
#include "led_colors.h"
#include "params_update.h"

//Todo: this could be a struct:
// struct quantizedScale {
//		enum colorCodes		color;
//		uint8_t				num_qtz_steps;
//		float				scale[MAX_NUM_QTZ_STEPS];
//		float				midpt[MAX_NUM_QTZ_STEPS];
//	};
//
//
//	struct quantizedScales qtz_scales[NUM_QTZ_SCALES];
//
//	void init_(void){...
//	...
//		qtz_scales[sclm_MAJOR].color = ledc_GOLD;
//		qtz_scales[sclm_MAJOR].num_qty_steps = 7;
//		qtz_scales[sclm_MAJOR].scale[0] = 16.35;
//  	...


const enum colorCodes qtz_scale_colors[NUM_QTZ_SCALES] = {
	ledc_WHITE,				// NO QTZ
	ledc_PURPLE,				// SEMITONE
	ledc_YELLOW,				// MAJOR
	ledc_DIM_YELLOW,			// MINOR HARMONIC
	ledc_DEEP_BLUE,				// MAJOR_PENTATONIC
	ledc_MED_BLUE,				// MINOR_PENTATONIC
	ledc_GREEN,				// OCTATONIC1
	ledc_DIM_GREEN,				// OCTATONIC2
	ledc_FULLRED,				// LYDIAN
	ledc_DIM_RED				// MIXOLYDIAN
};


const uint8_t num_qtz_steps[NUM_QTZ_SCALES] = {
	1,					// NO QTZ
	12,					// SEMITONES
	7,					// MAJOR
	7,					// MINOR HARMONIC
	5,					// MAJOR_PENTATONIC
	5,					// MINOR_PENTATONIC
	8,					// OCTATONIC1
	8,					// OCTATONIC2
	7,					// LYDIAN
	7					// MIXOLYDIAN

};

const float qtz_scales[NUM_QTZ_SCALES][MAX_NUM_QTZ_STEPS] = {

// NO QTZ
	{
		0
	},

// SEMITONES
        {
		16.3516015625,  // C0
		17.32390625,    // C#
		18.3540234375,  // D
		19.4454296875,  // D#
		20.60171875,    // E
		21.8267578125,  // F
		23.1246484375,  // F#
		24.4997265625,  // G
		25.9565625,     // G#
		27.50,          // A
		29.135234375,   // A#
		30.8676953125   // B
	},

// MAJOR
	{
		16.3516015625,	// C
		18.3540234375,	// D
		20.60171875,	// E
		21.8267578125,	// F
		24.4997265625,	// G
		27.50,		// A
		30.8676953125	// B
	},

// MINOR HARMONIC
	{
		16.3516015625,	// C
		18.3540234375,	// D
		19.4454296875,	// D#
		21.8267578125,	// F
		24.4997265625,	// G
		25.9565625,	// G#
		30.8676953125	// B
	},
        
// MAJOR_PENTATONIC
        {
		16.3516015625,	// C
		18.3540234375,	// D
		20.60171875,	// E
		24.4997265625,	// G
		27.50		// A
        },
    
//  MINOR_PENTATONIC   
        {
		16.3516015625,	// C
		19.4454296875,	// D#
		21.8267578125,	// F
		24.4997265625,	// G
		29.135234375	// A#
        },

// OCTATONIC 1
        {
		16.3516015625,	// C
		18.3540234375,	// D
		19.4454296875,	// D#
		21.8267578125,	// F
		23.1246484375,	// F#
		25.9565625,	// G#
		27.50,		// A
		30.8676953125	// B
        },

// OCTATONIC2
        {
		16.3516015625,	// C
		17.32390625,	// C#
		19.4454296875,	// D#
		20.60171875,	// E
		23.1246484375,	// F#
		24.4997265625,	// G
		27.50,		// A
		29.135234375	// A#
        },

// LYDIAN
        {
		16.3516015625,	// C
		18.3540234375,	// D
		20.60171875,	// E
		23.1246484375,	// F#
		24.4997265625,	// G
		27.50,		// A
		30.8676953125	// B
        },
    
// MIXOLYDIAN
        {
		16.3516015625,	// C
		18.3540234375,	// D
		20.60171875,	// E
		21.8267578125,	// F
		24.4997265625,	// G
		27.50,		// A
		29.135234375	// A#
        }
};


float qtz_scale_midpt[NUM_QTZ_SCALES][MAX_NUM_QTZ_STEPS];

void init_quantz_scales(void)
{
	uint8_t i,j;

	for (i=0;i<NUM_QTZ_SCALES;i++)
	{
		for (j=0; j<(num_qtz_steps[i]-1); j++)
		{
			qtz_scale_midpt[i][j] = (qtz_scales[i][j] + qtz_scales[i][j+1]) / 2.0;
		}

		//last midpoint is between last note and an octave above the first note
		qtz_scale_midpt[i][num_qtz_steps[i]-1] = (qtz_scales[i][num_qtz_steps[i]-1] + (qtz_scales[i][0]*2.0)) / 2.0; 
	}

}

float quantize_to_scale(uint8_t scale_num, float unqtz_freq, uint8_t *qtz_note, int8_t *qtz_oct){
//float quantize_to_scale(uint8_t scale_num, float unqtz_freq, uint8_t *qtz_note, uint8_t *qtz_oct, uint8_t prev_qtz_note, uint8_t prev_qtz_oct){

	uint8_t note 			= 0;
	int8_t oct  			= 0;
	float 	qtz_freq		= unqtz_freq;
	float	high_note;
	float	distance_from_midpt;

	if ((scale_num == sclm_NONE) || (scale_num>=NUM_QTZ_SCALES))
		return unqtz_freq;

	// Find octave
	high_note = qtz_scale_midpt[scale_num][ num_qtz_steps[scale_num]-1 ];
 	while( (unqtz_freq >= high_note ) && (oct <= MAX_OCT) ){
 		oct++;
 		high_note *= 2;
 	}

	if (oct < MAX_OCT){

		for (note = 0; note < num_qtz_steps[scale_num]; note++){
			distance_from_midpt = (qtz_scale_midpt[scale_num][note] * (float)(1<<oct)) - unqtz_freq;
			if( distance_from_midpt > 0.0 )
			{
				if (distance_from_midpt > (0.1	 * (float)(1<<oct)))
				{
					//Set the frequency based on note and octave
					qtz_freq  = qtz_scales[scale_num][note] * (1<<oct);
					*qtz_note = note;
					*qtz_oct = oct;
					break; //found a note, so exit loop
				} else {
					break; //Todo: implement better anti-hysteresis
				}
			}
		}
	}

	return qtz_freq;
}

