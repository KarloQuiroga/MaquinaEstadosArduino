#include <avr/io.h>

#define F_CPU 16000000 UL#include <util/delay.h>

#include <avr/interrupt.h>

#define TIME 100
#define BUTTONS PINC
#define MODO PINC0
#define MODO_PRESS bit_is_set(BUTTONS, MODO)
#define ASC PINC1
#define ASC_PRESS bit_is_set(BUTTONS, ASC)
#define DES PINC2
#define DES_PRESS bit_is_set(BUTTONS, DES)
//Display
#define DISPLAY_PORTD PORTD //Definicion de puertos a usar
#define DISPLAY_PORTB PORTB
//Segmento A
#define SEGA PORTD2 //PORTD2
#define SEGA_ON DISPLAY_PORTD |= _BV(SEGA)
#define SEGA_OFF DISPLAY_PORTD &= ~_BV(SEGA)
//Segmento B
#define SEGB PORTD3
#define SEGB_ON DISPLAY_PORTD |= _BV(SEGB)
#define SEGB_OFF DISPLAY_PORTD &= ~_BV(SEGB)
//Segmento C
#define SEGC PORTD4
#define SEGC_ON DISPLAY_PORTD |= _BV(SEGC)
#define SEGC_OFF DISPLAY_PORTD &= ~_BV(SEGC)
//Segmento D
#define SEGD PORTD5
#define SEGD_ON DISPLAY_PORTD |= _BV(SEGD)
#define SEGD_OFF DISPLAY_PORTD &= ~_BV(SEGD)
//Segmento E
#define SEGE PORTD6
#define SEGE_ON DISPLAY_PORTD |= _BV(SEGE)
#define SEGE_OFF DISPLAY_PORTD &= ~_BV(SEGE)
//Segmento F
#define SEGF PORTD7
#define SEGF_ON DISPLAY_PORTD |= _BV(SEGF)
#define SEGF_OFF DISPLAY_PORTD &= ~_BV(SEGF)
//Segmento G
#define SEGG PORTB0
#define SEGG_ON DISPLAY_PORTB |= _BV(SEGG)
#define SEGG_OFF DISPLAY_PORTB &= ~_BV(SEGG)
//Definir el LED (digitos)
#define COMMON_DDRX DDRB
#define COMMON_PORTX PORTB
//LED 1
#define COMMON_LED1 PORTB4
#define COMMON_LED1_ON COMMON_PORTX &= ~_BV(COMMON_LED1)
#define COMMON_LED1_OFF COMMON_PORTX |= _BV(COMMON_LED1)
//LED 2
#define COMMON_LED2 PORTB3
#define COMMON_LED2_ON COMMON_PORTX &= ~_BV(COMMON_LED2)
#define COMMON_LED2_OFF COMMON_PORTX |= _BV(COMMON_LED2)
//LED 3
#define COMMON_LED3 PORTB2
#define COMMON_LED3_ON COMMON_PORTX &= ~_BV(COMMON_LED3)
#define COMMON_LED3_OFF COMMON_PORTX |= _BV(COMMON_LED3)
//LED 4
#define COMMON_LED4 PORTB1
#define COMMON_LED4_ON COMMON_PORTX &= ~_BV(COMMON_LED4)
#define COMMON_LED4_OFF COMMON_PORTX |= _BV(COMMON_LED4)
//Macro
#define _BV(bit)(1 << (bit))
//Variables y definicion de contador automatico
void contadorFun(uint16_t numero);
uint8_t millares = 0;
uint8_t centenas = 0;
uint8_t decenas = 0;
uint8_t unidades = 0;
uint8_t contador = 0000;
//contador ascendente
uint8_t contador_asc = 0000;
//contador descendente
uint8_t contador_des = 159;
//Seleccion de modo
uint8_t modoReg = 0;
//valor de potenciometro
uint8_t potValue = 0;
//valor potenciometro convertido a formato requerido 0-999
uint16_t format = 0;
//Inicializar display multiplexado - Segmentos y pantallas
void init_display(void) {
    DDRD |= (_BV(SEGA) | _BV(SEGB) | _BV(SEGC) | _BV(SEGD) | _BV(SEGE) | _BV(SEGF));
    DDRB |= _BV(SEGG);
    COMMON_DDRX |= (_BV(COMMON_LED1) | _BV(COMMON_LED2) | _BV(COMMON_LED3) |
        _BV(COMMON_LED4));
}
//Inicializar contador (TIMER)
void init_timer(void) {
    //Modo CTC
    TCCR0A &= ~_BV(WGM00);
    TCCR0A |= _BV(WGM01);
    TCCR0B &= ~_BV(WGM02);
    //Prescaler
    TCCR0B |= _BV(CS02);
    TCCR0B &= ~_BV(CS01);
    TCCR0B |= _BV(CS00);
    //Tope para 0.1s 1s
    OCR0A = 125000;
    //Enable
    TIMSK0 |= _BV(OCIE0A);
}
//Inicializar puertos
void init_ports(void) {
    DDRC &= ~_BV(MODO);
    DDRC &= ~_BV(ASC);
    DDRC &= ~_BV(DES);
}
//Inicializar ADC
void init_adc(void) {
    //Avcc como pin de referencia
    //ADMUX &=~ (1<<REFS1);
    ADMUX |= (1 << REFS0);
    //Ajustar a 8 bits
    ADMUX |= (1 << ADLAR);
    // Escoger el PIN a leer ADC5
    ADMUX &= ~(1 << MUX3);
    ADMUX |= (1 << MUX2);
    ADMUX &= ~(1 << MUX1);
    ADMUX |= (1 << MUX0);
    //Freeruning
    ADCSRA |= (1 << ADATE);
    //Enable interrupt
    ADCSRA |= (1 << ADIE);
    //velocidad de muestreo
    // 16 MHz clock / 128 = 125 kHz ADC clock debe de estar entre 50 - 200Khz
    ADCSRA |= (1 << ADPS2);
    ADCSRA |= (1 << ADPS1);
    ADCSRA |= (1 << ADPS0);
}
//Main
int main(void) {
    //MAIN
    cli();
    init_display();
    init_ports();
    init_timer();
    init_adc();
    sei();
    ADC_on();
    while (1) {
        uint16_t conversor = 0;
        //
        if (modoReg == 0) {
            conversor = (uint16_t) contador; //Convierte los contadores de uint8 a uint16
            contadorFun(conversor);
            _delay_us(10);
        } else if (modoReg == 1) {
            conversor = (uint16_t) contador_asc;
            contadorFun(conversor);
            _delay_us(10);
        } else if (modoReg == 2) {
            conversor = (uint16_t) contador_des;
            contadorFun(conversor);
            _delay_us(10);
        }
        //Ultimo estado (potenciometro).
        else if (modoReg == 3) {
            contadorFun(format);
        }
    }
}
//
void contadorFun(uint16_t numero) {
    millares = numero / 1000;
    COMMON_LED1_ON;
    COMMON_LED2_OFF;
    COMMON_LED3_OFF;
    COMMON_LED4_OFF;
    show_numbers(millares);
    _delay_us(TIME);
    COMMON_LED1_OFF;
    COMMON_LED2_ON;
    COMMON_LED3_OFF;
    COMMON_LED4_OFF;
    centenas = (numero % 1000) / 100;
    show_numbers(centenas);
    _delay_us(TIME);
    COMMON_LED1_OFF;
    COMMON_LED2_OFF;
    COMMON_LED3_ON;
    COMMON_LED4_OFF;
    decenas = (numero % 100) / 10;
    show_numbers(decenas);
    _delay_us(TIME);
    COMMON_LED1_OFF;
    COMMON_LED2_OFF;
    COMMON_LED3_OFF;
    COMMON_LED4_ON;
    unidades = numero % 10;
    show_numbers(unidades);
    _delay_us(TIME);
    COMMON_LED1_OFF;
    COMMON_LED2_OFF;
    COMMON_LED3_OFF;
    COMMON_LED4_OFF;
    _delay_us(TIME);
}
//ISR
ISR(TIMER0_COMPA_vect) {
    if (MODO_PRESS && !ASC_PRESS && !DES_PRESS) {
        modoReg++;
    } else if (modoReg == 0) {
        if (contador < 159) {
            contador++;
            _delay_us(TIME);
        } else {
            contador = 0;
        }
    } else if (modoReg == 1) {
        if (!MODO_PRESS && ASC_PRESS && !DES_PRESS) {
            contador_asc++;
            _delay_us(TIME);
            if (contador_asc > 159) {
                contador_asc = 0;
            }
        }
    } else if (modoReg == 2) {
        if (!MODO_PRESS && !ASC_PRESS && DES_PRESS) {
            contador_des--;
            _delay_us(TIME);
            if (contador_des == 255) {
                contador_des = 159;
            }
        }
    } else if (modoReg == 3) {
        double aux = 3.91764;
        format = (uint16_t) ceil(potValue * aux);
    } else if (modoReg > 3) {
        modoReg = 0;
    }
}
//ISR - POT VALUE
ISR(ADC_vect) {
    potValue = ADCH;
}
//Creación de casos para numeros del display
void show_numbers(uint16_t num) {
    switch (num) {
    case 0:
        SEGA_ON;
        SEGB_ON;
        SEGC_ON;
        SEGD_ON;
        SEGE_ON;
        SEGF_ON;
        SEGG_OFF;
        break;
    case 1:
        SEGA_OFF;
        SEGB_ON;
        SEGC_ON;
        SEGD_OFF;
        SEGE_OFF;
        SEGF_OFF;
        SEGG_OFF;
        break;
    case 2:
        SEGA_ON;
        SEGB_ON;
        SEGC_OFF;
        SEGD_ON;
        SEGE_ON;
        SEGF_OFF;
        SEGG_ON;
        break;
    case 3:
        SEGA_ON;
        SEGB_ON;
        SEGC_ON;
        SEGD_ON;
        SEGE_OFF;
        SEGF_OFF;
        SEGG_ON;
        break;
    case 4:
        SEGA_OFF;
        SEGB_ON;
        SEGC_ON;
        SEGD_OFF;
        SEGE_OFF;
        SEGF_ON;
        SEGG_ON;
        break;
    case 5:
        SEGA_ON;
        SEGB_OFF;
        SEGC_ON;
        SEGD_ON;
        SEGE_OFF;
        SEGF_ON;
        SEGG_ON;
        break;
    case 6:
        SEGA_OFF;
        SEGB_OFF;
        SEGC_ON;
        SEGD_ON;
        SEGE_ON;
        SEGF_ON;
        SEGG_ON;
        break;
    case 7:
        SEGA_ON;
        SEGB_ON;
        SEGC_ON;
        SEGD_OFF;
        SEGE_OFF;
        SEGF_OFF;
        SEGG_ON;
        break;
    case 8:
        SEGA_ON;
        SEGB_ON;
        SEGC_ON;
        SEGD_ON;
        SEGE_ON;
        SEGF_ON;
        SEGG_ON;
        break;
    case 9:
        SEGA_ON;
        SEGB_ON;
        SEGC_ON;
        SEGD_OFF;
        SEGE_OFF;
        SEGF_ON;
        SEGG_ON;
        break;
    }
}
//Potenciometro (ADC)
void ADC_on(void) {
    //Encendemos el ADC
    ADCSRA |= (1 << ADEN);
    // Iniciar la conversión
    ADCSRA |= (1 << ADSC);
}