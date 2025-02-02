/*Este código foi desenvolvido para atennder aos requisitos
da atividade proposta em 27-01-2025
Residente: Jackson da Silva Carneiro*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

#include "pio_tarefa.pio.h"

#define NUM_PIXELS 25        //tamanho da matriz
#define MATRIX_PIN 7        //Matriz de led's=>GPIO7
#define red_led_pin 13  //led Vermelho => GPIO13
#define button_A 5     // Botão A => 5
#define button_B 6    //Botão B => 6

const uint8_t numbers[10][25] = // definição dos números para a matriz de leds
{
    {0, 0, 1, 0, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 0, 1, 0, 0}, // 0
    {0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 1, 0}, // 1
    {0, 0, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 1, 1}, // 2
     {0, 0, 1, 1, 0,
      0, 1, 0, 0, 1,
      0, 0, 0, 1, 0,
      0, 1, 0, 0, 1,
      0, 0, 1, 1, 0}, //3
     {0, 0, 1, 1, 0,
      0, 1, 0, 1, 0,
      1, 1, 1, 1, 1,
      0, 0, 0, 1, 0,
      0, 0, 0, 1, 0}, //4
     {0, 1, 1, 1, 1,
      0, 1, 0, 0, 0,
      0, 0, 1, 1, 0,
      0, 0, 0, 0, 1,
      0, 1, 1, 1, 0}, //5
     {0, 0, 1, 1, 0,
      0, 1, 0, 0, 0,
      0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0}, //6
     {0, 1, 1, 1, 1,
      0, 0, 0, 0, 1,
      0, 0, 0, 1, 0,
      0, 0, 1, 0, 0,
      0, 1, 0, 0, 0}, //7
     {0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 0, 1, 0, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0}, //8
     {0, 1, 1, 1, 0,
      0, 1, 0, 1, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 0, 1, 1, 0} //9
};


int num_atual=0;  //variável global para o numero atual

bool button_pressed(uint gpio){   //vefirficar se um botão foi pressionado
    return !gpio_get(gpio);
}

void draw_number(int num){
    if (num >=0 && num <=9){
        //aqui será definida umma cor para cada número
        double r, g, b;
        switch (num){
            case 0: r = 0.5; g = 0.0; b = 0.5; break;
            case 1: r = 0.0; g = 0.5; b = 0.5; break;
            case 2: r = 0.0; g = 0.0; b = 0.5; break;
            case 3: r = 0.5; g = 0.5; b = 0.5; break;
            case 4: r = 0.5; g = 0.0; b = 0.0; break;
            case 5: r = 0.0; g = 0.5; b = 0.0; break;
            case 6: r = 0.3; g = 0.6; b = 0.5; break;
            case 7: r = 0.5; g = 0.3; b = 0.3; break;
            case 8: r = 0.8; g = 0.0; b = 0.5; break;
            case 9: r = 0.2; g = 0.8; b = 0.5; break;
            default: r = 0.0; g = 0.0; b = 0.0; //leds apagados
        }
        desenho_pio(numbers[num], pio0, sm, r, g, b);
    }
}

   //rotina da interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){
    if (gpio == button_A){
        num_atual = (num_atual+1)%10; //irá somar um número na matriz de leds
        draw_number(num_atual);
    } else if (gpio == button_B){
        num_atual = (num_atual-1+10)%10; //irá subtrair umm número na matriz de leds
        draw_number(num_atual);
    }
  
}

//rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double r, double g, double b)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

void configura_GPIO(){  //Configurando GPIO's
  
    gpio_init(red_led_pin);                 // Inicializa o pino do LED
    gpio_set_dir(red_led_pin, GPIO_OUT);    // Configura o pino como saída


    gpio_init(button_A);                    // Inicializa o botão A
    gpio_set_dir(button_A, GPIO_IN);        // Configura o pino como entrada
    gpio_pull_up(button_A);                 // Habilita o pull-up interno

    gpio_init(button_B);                    // Inicializa o botão B
    gpio_set_dir(button_B, GPIO_IN);        // Configura o pino como entrada
    gpio_pull_up(button_B);                 // Habilita o pull-up interno
}

//rotina para acionar a matrix de leds - ws2812b
void desenho_pio(const uint8_t desenho[], PIO pio, uint sm, double r, double g, double b){

    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t valor_led;
        // aqui verificamos se o led deve ser aceso ou apagado
        if (desenho[i] ==1)
        {  
             //led aceso
            valor_led = matrix_rgb(r, g, b);
            
        }else{
            //led apagado
            valor_led = matrix_rgb(0.0, 0.0, 0.);
            
        }
        pio_sm_put_blocking(pio, sm, valor_led);
    }
   
}

//função principal
int main()
{
    PIO pio = pio0; 
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r = 0.0, b = 0.0 , g = 0.0;

    //coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
    ok = set_sys_clock_khz(128000, false);

    // Inicializa todos os códigos stdio padrão que estão ligados ao binário.
    stdio_init_all();



    //configurações da PIO
    uint offset = pio_add_program(pio, &pio_tarefa_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_tarefa_program_init(pio, sm, offset, MATRIX_PIN);

    //inicializa os pinos GPIO
    configura_GPIO();

    //configurando os pinos de interrupções
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, & gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, & gpio_irq_handler);
    irq_set_global_interrupts(true);

    while (true) {
                // O led vermelho piscará 5 vezes em 1 segundo
        gpio_put(red_led_pin, true);
        sleep_ms(100);
        gpio_put(red_led_pin, false);
        sleep_ms(100);
    }
    
}
