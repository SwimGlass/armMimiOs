#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"
#include <string.h>
#include <stdlib.h>

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x0020)


void usart_init(void)
{
    *(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
    *(RCC_APB1ENR) |= (uint32_t) (0x00020000);

    /* USART2 Configuration, Rx->PA3, Tx->PA2 */
    *(GPIOA_CRL) = 0x00004B00;
    *(GPIOA_CRH) = 0x44444444;
    *(GPIOA_ODR) = 0x00000000;
    *(GPIOA_BSRR) = 0x00000000;
    *(GPIOA_BRR) = 0x00000000;

    *(USART2_CR1) = 0x0000000C;
    *(USART2_CR2) = 0x00000000;
    *(USART2_CR3) = 0x00000000;
    *(USART2_CR1) |= 0x2000;
}

void print_str(const char *str)
{
    while (*str) {
        while (!(*(USART2_SR) & USART_FLAG_TXE));
        *(USART2_DR) = (*str & 0xFF);
        str++;
    }
}

static void print_char(const char *str)
{
    while (!(*(USART2_SR) & USART_FLAG_TXE));
    *(USART2_DR) = (*str & 0xFF);
}

int str_ncmp(char *a,char *b,int size){
    int i;
    int flag=1;
    for(i=0;i<size;i++) if(a[i]!=b[i]) flag=0;
    return flag;
}

char get_char()
{
    while(!(*(USART2_SR) & USART_FLAG_RXNE));
    return (char)(*(USART2_DR) & 0xFF);
}

static void delay(volatile int count)
{
    count *= 50000;
    while (count--);
}

static void busy_loop(void *str)
{
    while (1) {
        print_str(str);
        print_str(": Running...\n");
        delay(1000);
    }
}

void int_to_str(int num,char *buffer){
    int i,tmp;
    for(i=0;i<10;i++)
    {
        tmp = num % 10;
        buffer[9-i] = (char)('0'+tmp);
        num = num/10; 
    }
}

int str2int(char *str){
    int num = 0;
    int index = 0;
    if(str[0]=='-')
        index++;
    while(str[index] != '\0'){
        if(str[index]>='0' && str[index]<='9'){
            num = num*10 + (str[index]-'0');
            index++;
        }
        else
            return -999;
    }
    if(str[0]=='-')
        return num*(-1);
    else
        return num;

}

int str_to_int(char *str){
    int num=0;
    int i;
    print_str("QQQQ:\n");
    print_str(str);
    print_str("\n");
    for(i=0;i<sizeof(str)/sizeof(char);i++){
        if(str[i]>='0' && str[i]<='9')
            num = num*10 + (str[i]-'0');
    }
    return num;
    //if(str[0]=='-') return num*-1;
    //else return num;
}

extern int fibonacci(int x);

void fib(void *inputdata)
{
    char tmp[100];
    char fib_resault[10];
    int fib_r=0;
    print_str("\nIn fib function\n");
    print_str("Input is :\n");
    strcpy(tmp,inputdata);
    print_str(tmp);
    print_str("\n");
    fib_r = str2int(tmp);
    fib_r = atoi(tmp);
    if(fib_r == 8) print_str("\nit'8\n");
    if(fib_r == 0) print_str("\n0!!!!\n");
    int_to_str(fib_r,fib_resault);
    print_str(fib_resault);
    print_str("\n");

    // fib_r = fibonacci(fib_r);
    // int_to_str(fib_r,fib_resault);
    // print_str(fib_resault);
    // print_str("\n");
    thread_self_terminal();
}

static void shell_loop(void *str)
{
    while(1){
        char cmd[100];
        int count = 0;
        char c='\0';
        print_str(str);
        while(1){
            c = get_char();
            *(USART2_DR) = 0x0;
            if(((cmd[count]=c)=='\r')||(c=='\n')){ 
                print_str("\n\r");
                break;
            }
            else if((c==8) || (c==127)){
                if(count!=0){
                    print_str("\b \b");
                    cmd[count] = '\0';
                    count-=2;
                }
            }
            else print_char(&c);
            count++;
        }
        print_str("OUTPUT is:\n");
        print_str(cmd);
        if(str_ncmp("fib",cmd,3)){
            print_str("SUCCESS\n");
            if (thread_create(fib, (void *) cmd) == -1)
                print_str("Thread 1 creation failed\r\n");
            pendsv_handler();
        }
        else print_str("Not support this command!\n");
    }
}

void test1(void *userdata)
{
    shell_loop(userdata);
}

void test2(void *userdata)
{
    busy_loop(userdata);
}

void test3(void *userdata)
{
    busy_loop(userdata);
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
    //const char *str1 = "Task1", *str2 = "Task2", *str3 = "Task3";
    const char *str1 = "swimglass :";

    usart_init();

    if (thread_create(test1, (void *) str1) == -1)
        print_str("Thread 1 creation failed\r\n");

    //if (thread_create(test2, (void *) str2) == -1)
    //    print_str("Thread 2 creation failed\r\n");

    //if (thread_create(test3, (void *) str3) == -1)
    //    print_str("Thread 3 creation failed\r\n");

    /* SysTick configuration */
    //*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
    //*SYSTICK_VAL = 0;
    //*SYSTICK_CTRL = 0x07;

    thread_start();

    return 0;
}
