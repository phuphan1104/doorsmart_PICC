#include <main.h>

#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, parity=N, bits=8)
//#include<DoorLock.h>
#define LCD_RS_PIN     PIN_D0
#define LCD_RW_PIN     PIN_D1
#define LCD_ENABLE_PIN PIN_D2
#define LCD_DATA4      PIN_D4
#define LCD_DATA5      PIN_D5
#define LCD_DATA6      PIN_D6
#define LCD_DATA7      PIN_D7
#include <lcd.c>

char received[13];
char compare_string[] = "ABCD12FD3B56" ;// RFID
int v = 0;

int i;
char matkhau[16]={'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

void write_eeprom_pass()
{
   for(i=0;i<16;i++)
   {
      write_eeprom(i,matkhau[i]);
   }
}

char keypad()
{
   output_low(pin_C4);
   output_high(pin_C5);
   output_high(pin_C3);
   output_high(pin_C2);
   if(input(pin_B4)==0) return '7';
   else if(input(pin_B5)==0) return '4';
   else if(input(pin_B6)==0) return '1';
   else if(input(pin_B7)==0) return '*';
   
   output_high(pin_C4);
   output_low(pin_C5);
   output_high(pin_C3);
   output_high(pin_C2);
   if(input(pin_B4)==0) return '8';
   else if(input(pin_B5)==0) return '5';
   else if(input(pin_B6)==0) return '2';
   else if(input(pin_B7)==0) return '0';
   
   output_high(pin_C4);
   output_high(pin_C5);
   output_low(pin_C3);
   output_high(pin_C2);
   if(input(pin_B4)==0) return '9';
   else if(input(pin_B5)==0) return '6';
   else if(input(pin_B6)==0) return '3';
   else if(input(pin_B7)==0) return '#';
   
   output_high(pin_C4);
   output_high(pin_C5);
   output_high(pin_C3);
   output_low(pin_C2);
   if(input(pin_B4)==0) return 'A';
   else if(input(pin_B5)==0) return 'B';
   else if(input(pin_B6)==0) return 'C';
   else if(input(pin_B7)==0) return 'D';
   
   return 0xff;
}

char key_deboucing()
{
   char key;
   do
   {
      key = keypad(); 
   } while(key==0xff);
   
   while(keypad()!=0xff);
   return key;
}

void enter_pw()
{
   for(i=0;i<16;i++)
   {
      matkhau[i] = key_deboucing();
      if(matkhau[i]=='#') break;
      lcd_gotoxy(i+1,2);
      printf(lcd_putc,"%c",matkhau[i]);
   }
}

void empty_pw()
{
   for(i=0;i<16;i++)
   {
      matkhau[i]='0';   
   }
}

int compare()
{
   for(i=0;i<16;i++)
   {
      if(read_eeprom(i) != matkhau[i]) return 0;   
   }
   return 1;
}

void open()
{
   lcd_putc('\f');
   lcd_gotoxy(5,1);
   printf(lcd_putc,"Welcome");
   output_high(pin_C0);
   for(i=9;i>0;i--)
   {
      lcd_gotoxy(3,2);
      printf(lcd_putc,"Close in %d",i);
      delay_ms(1000);
   }
   output_low(pin_C0);
}

void main()
{
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_RDA);

   lcd_init();
   set_tris_b(0xff);
   port_b_pullups(true);

   while(TRUE)
   {  
      if(v == 12) {
         received[v] = '\0';
         printf("\nReceived: %s\n", received);
         if(strcmp(received, compare_string) == 0) {
            printf("RFID corect. Welcome!!! \n");
            open();
         } else {
            printf("RFID NOT corect. \n");
         }
         v = 0;    
      }
      
      lcd_gotoxy(1,1);
      printf(lcd_putc,"NhapMatKhau *");
      lcd_gotoxy(1,2);
      printf(lcd_putc,"DoiMatKhau #");
      char check = keypad();
      delay_ms(300);
      if(check=='*')
      {
         lcd_putc('\f');
         lcd_gotoxy(1,1);
         printf(lcd_putc,"XinMoiNhapMK"); 
         enter_pw();
         if(compare()==1)
         {  
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Corect        ");
            empty_pw();
            delay_ms(1000);
            lcd_putc('\f');
            open();
         }
         else if(compare()==0)
         {  
            lcd_putc('\f');
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Not Corect");
            empty_pw();
            delay_ms(2000);
         }
      }
      else if(check=='#')
      {
         lcd_putc('\f');
         lcd_gotoxy(1,1);
         printf(lcd_putc,"MatKhauCu"); 
         enter_pw();
         if(compare()==1)
         {  
            lcd_putc('\f');
            lcd_gotoxy(1,1);
            printf(lcd_putc,"MatKhauMoi");
            empty_pw();
            enter_pw();
            write_eeprom_pass();
            empty_pw();
            lcd_putc('\f');
            lcd_gotoxy(1,1);
            printf(lcd_putc,"MKMoiDaLuu");
            delay_ms(2000);
         }
         else if(compare()==0)
         {  
            lcd_putc('\f');
            lcd_gotoxy(1,1);
            printf(lcd_putc,"MatKhauCuSai");
            empty_pw();
            delay_ms(2000);
         }
      }
  
   }
   
}
#INT_RDA
void serial_isr() {
   char received_char = getc();
   printf("%c", received_char);
   received[v++] = received_char;
}

