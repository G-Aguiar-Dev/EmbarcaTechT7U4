// Inclusão de bibliotecas
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 22      // Pino do servomotor
#define PWM_WRAP 19999    // Período de 20ms (50Hz)
#define STEP_US 5         // Incremento de 5µs
#define STEP_DELAY_MS 10  // Atraso entre ajustes

// Variáveis globais
volatile int current_pulse = 500; // Inicia em 0°
volatile bool direction = true;   // true = incrementando, false = decrementando
volatile int phase = 1;           // Fase atual

// Timer de repetição para ajuste suave
bool timer_callback(repeating_timer_t *rt) {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    uint chan_num = pwm_gpio_to_channel(SERVO_PIN);

    if (phase == 4) {
        // Fase 4: Movimento suave
        if (direction) {
            current_pulse += STEP_US;
            if (current_pulse >= 2400) direction = false;
        } else {
            current_pulse -= STEP_US;
            if (current_pulse <= 500) direction = true;
        }
    }

    // Aplica o novo duty cycle
    pwm_set_chan_level(slice_num, chan_num, current_pulse);
    return true;
}

// Função para mudar a fase
void set_phase(int new_phase) {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    uint chan_num = pwm_gpio_to_channel(SERVO_PIN);

    phase = new_phase;

    switch (phase) {
        case 1:
            current_pulse = 2400; // 180°
            break;
        case 2:
            current_pulse = 1470; // 90°
            break;
        case 3:
            current_pulse = 500;  // 0°
            break;
        case 4:
            break;
    }

    // Aplica o novo duty cycle
    pwm_set_chan_level(slice_num, chan_num, current_pulse);
}

int main() {
    stdio_init_all(); // Inicializa a comunicação serial
    
    // Configuração do PWM
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);    // Clock divisor para 1µs por passo
    pwm_config_set_wrap(&config, PWM_WRAP);    // Período de 20ms (50Hz)
    pwm_init(slice_num, &config, true);
    
    // Configura timer para ajustes suaves
    repeating_timer_t timer;
    add_repeating_timer_ms(STEP_DELAY_MS, timer_callback, NULL, &timer);

    // Loop Principal
    // Executa as fases
    while (true) {
        if (phase == 1) {
            set_phase(1); // Fase 1: 180°
            sleep_ms(5000); // Aguarda 5 segundos
            set_phase(2); // Fase 2: 90°
            sleep_ms(5000); // Aguarda 5 segundos
            set_phase(3); // Fase 3: 0°
            sleep_ms(5000); // Aguarda 5 segundos
            set_phase(4); // Fase 4: Movimento suave (definitivo)
        }
        sleep_ms(10); // Aguarda 10ms
    }
    return 0;
}