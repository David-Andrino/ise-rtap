#include "dsp.h"

#include "../DSP/audioConfig.h"
#include "../main.h"
#include "arm_math.h"
#include "eqCoefficients.h"

#define BIQUAD_STAGES 2
#define DSP_SHIFT_MARGIN 8

// Handlers
static TIM_HandleTypeDef htim = {0};

static arm_biquad_cas_df1_32x64_ins_q31 lowStageHandlers [2];
static arm_biquad_casd_df1_inst_q31     highStageHandlers[2];

// Buffers para el DSP
static q31_t midBuffer[DSP_BUFSIZE];
static q63_t lowStateBuffers [2][4 * BIQUAD_STAGES];
static q31_t highStateBuffers[3][4 * BIQUAD_STAGES];

// Configuracion del DSP
// Arranca con valores invalidos para forzar primer inicio
static int8_t bandGains[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; 
static uint8_t volume = 10; // Almacenado en tanto por 10

static int dsp_tim_init(void);
static void dsp_configure_filters(int8_t* bands, uint8_t vol);

int dsp_init(void) {
    int8_t bands[] = { 1, 3, 0, 0, -2 };
    dsp_configure_filters(bands, 4);
    return dsp_tim_init();
}

int dsp_tim_init(void) {
    __DSP_TIM_ENABLE();
    htim.Instance = DSP_TIM_INSTANCE;

    htim.Init.Prescaler = DSP_TIM_PRESCALER;
    htim.Init.Period = DSP_TIM_PERIOD;

    if (HAL_TIM_Base_Init(&htim)) {
        return -1;
    }

    TIM_ClockConfigTypeDef sClockConfig = {.ClockSource = TIM_CLOCKSOURCE_INTERNAL};
    if (HAL_TIM_ConfigClockSource(&htim, &sClockConfig)) {
        return -1;
    }

    TIM_MasterConfigTypeDef sMasterConfig = {
        .MasterOutputTrigger = TIM_TRGO_UPDATE,
        .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE,
    };
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig)) {
        return -1;
    }

    if (HAL_TIM_Base_Start(&htim)) {
        return -1;
    }

    return 0;
}

void dsp_configure_filters(int8_t* bands, uint8_t vol) {
    volume = vol;
    for (int band = 0; band < 5; band++) {
        if (bandGains[band] != bands[band]) {
            bandGains[band] = bands[band];
            
            if (band < 2) { 
                arm_biquad_cas_df1_32x64_init_q31(                      // Mejor precision en bajas frecuencias
                    &lowStageHandlers[band],                            // Handler
                    BIQUAD_STAGES,                                      // Etapas (2)
                    &coeffTable[GET_COEFF_IDX(band, bandGains[band])],  // Coeficientes (ver eqCoefficients.h)
                    lowStateBuffers[band],                              // Buffer de estado
                    COEFF_SHIFT                                         // Desplazamiento de coeficientes
                );
            } else {
                arm_biquad_cascade_df1_init_q31(                        // Mas velocidad en altas frecuencias
                    &highStageHandlers[band - 2],                       // Handler
                    BIQUAD_STAGES,                                      // Etapas (2)
                    &coeffTable[GET_COEFF_IDX(band, bandGains[band])],  // Coeficientes (ver eqCoefficients.h)
                    highStateBuffers[band],                             // Buffer de estado
                    COEFF_SHIFT                                         // Desplazamiento de coeficientes
                );
            }
        }
    }
}

void processSamples(uint16_t* in, uint16_t* out) {
    if (volume == 0) { // Volumen 0 -> No procesar salida (menos consumo)
        arm_fill_q15(2048, (q15_t*)out, DSP_BUFSIZE);
        return;
    }
    
    arm_offset_q15((q15_t*)in, -2048, (q15_t*)out, DSP_BUFSIZE);
    
    arm_q15_to_q31((q15_t*)out, midBuffer, DSP_BUFSIZE);
    
    arm_shift_q31(midBuffer, -DSP_SHIFT_MARGIN, midBuffer, DSP_BUFSIZE);
    
    if (volume < 10) { // Aplicar volumen
        arm_scale_q31(midBuffer, volCoeffs[volume], 0, midBuffer, DSP_BUFSIZE);
    }
    
    // Aplicar filtros
    for (int i = 0; i < 2; i++) {
        arm_biquad_cas_df1_32x64_q31(&lowStageHandlers[i], midBuffer, midBuffer, DSP_BUFSIZE);
    }
    for (int i = 0; i < 3; i++) {
        arm_biquad_cascade_df1_q31(&highStageHandlers[i], midBuffer, midBuffer, DSP_BUFSIZE);
    }
    
    arm_shift_q31(midBuffer, DSP_SHIFT_MARGIN, midBuffer, DSP_BUFSIZE);
    
    arm_q31_to_q15(midBuffer, (q15_t*)out, DSP_BUFSIZE); 
    
    
    // Recuperar offset
    arm_offset_q15((q15_t*)out, 2048, (q15_t*)out, DSP_BUFSIZE);
}
