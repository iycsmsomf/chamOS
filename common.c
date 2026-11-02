#include "common.h"

void putchar(char ch);

void printf(const char *fmt,...){
    va_list vargs;
    va_start(vargs,fmt);
    while (*fmt){
        if (*fmt=='%'){
            fmt++;
            switch (*fmt){
                case '\0':{
                    putchar('%');
                    goto end;
                }
                case '%':{ 
                    putchar('%');
                    break;
                }
                case 's':{ //string
                    const char *s=va_arg(vargs,const char *);
                    while(*s){                //null - 0/false
                        putchar(*s);
                        s++;
                    }
                    break;
                }
                case 'd':{ //integer - since putchar does one digit at a time we need to find individual digits
                    int value=va_arg(vargs,int);
                    unsigned divisor = 1;
                    unsigned magnitude=value;
                    if (value < 0) {
                        putchar('-');
                        magnitude = -magnitude;
                    }

                    while (magnitude/divisor >=10){
                        divisor*=10;
                    }

                    while (divisor>0){
                        putchar('0' + magnitude/divisor);
                        magnitude%=divisor;
                        divisor/=10;
                    }

                    break;
                      
                }
                case 'x':{  // Print an integer in hexadecimal.
                        unsigned value = va_arg(vargs, unsigned);
                        for (int i = 7; i >= 0; i--) {
                            unsigned nibble = (value >> (i * 4)) & 0xf;
                            putchar("0123456789abcdef"[nibble]);
                        }
                }       break;


            }
        }
        else{
            putchar(*fmt);
        }

        

        fmt++;
    }
    end:
        va_end(vargs);
}
void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *memset(void *buf, char c , size_t n){               //void pointer store adress of any data type

    uint8_t *p = (uint8_t *) buf;
    while(n--){
        *p++ = c;

    }

    return buf; 

}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

