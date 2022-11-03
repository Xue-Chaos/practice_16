/* ����ͷ�ļ� */
#include<iocc2530.h>
#include<string.h>

/*�궨��*/
#define LED1 P1_0
#define LED2 P1_1
#define uint16 unsigned short
/*�������*/
int count=0;//ͳ�ƶ�ʱ���������
char output[8];//���ת�����ַ���ʽ�Ĵ���������
uint16 light_val=0;//ADC�ɼ����
uint16 light_flag=0;//����״̬��־λ   1���й��գ�0��û����
uint16 light_flag_last=0;//��һ�εĹ���״̬

/*��������*/
void InitCLK(void);//ϵͳʱ�ӳ�ʼ��������Ϊ32MHz
void InitUART0( );//����0��ʼ��
void InitT1( );//��ʱ��1��ʼ��
void Delay(int delaytime);//��ʱ����
unsigned short Get_adc( );//ADC�ɼ�
void Uart_tx_string(char *data_tx,int len);//�����ڷ���ָ�����ȵ�����
void InitLED(void);//�Ƶĳ�ʼ��

/*���庯��*/
void InitCLK(void)
{
  CLKCONCMD &= 0x80;
  while(CLKCONSTA & 0x40);
}

void InitLED()
{
  P1SEL &= ~0x03;//����P1_0��P1_1ΪGPIO��
  P1DIR |= 0x03;//����P1_0��P1_1Ϊ���
}

void Delay(int delaytime)
{
    int i=0,j=0;
    for(i=0; i<240; i++)
        for(j=0; j<delaytime; j++);
}

void InitT1( )
{
    /*.......������2��ʼ����ʱ��1����....................*/
    T1CTL = 0X0D;//128Ƶ����������ģʽ ��1101��
    TIMIF|=0X40;//ʹ�ܶ�ʱ������ж�
    IEN1 |=0X02;//��ʱ��1�ж�ʹ�� �� T1IE=1
   
    /*.......������2����.......................................*/
     EA = 1; 
}

uint16 Get_adc( )
{
    /*.......������3��ʼ�����������ų�ʼ����ADC��������....................*/
    APCFG |= 1;//����P0_0Ϊģ��˿�
    P0SEL |= 0x01;//����P0_0Ϊ����
    P0DIR &= ~0x01;//����P0_0Ϊ���뷽��
    ADCCON3 = 0x80|0x20|0x00;//��0xA0;���òο���ѹ3.3V 256��Ƶ ʹ��AIN0ͨ��
  /*.......������3����.......................................*/
     
    while(!ADCIF);
    ADCIF=0;
    unsigned long value;
    value = ADCH;
    value = value<<8;
    value |=ADCL;
    value = value*330;//VALUE * 3.3v / 32768
    value = value>>15;
    return (uint16)value;
}
void InitUART0( )
{
   U0CSR |=0X80;//����ģʽ
    /*.......������4��ʼ����������....................*/
   
    PERCFG|= 0x00;//USART0ʹ�ñ���λ��1 P0_2 P0_3
    P0SEL |=0X0C;//����P0_2 P0_3Ϊ����
    U0UCR |= 0X80;//������ 8λ����λ ����żУ�� 1λֹͣλ

    U0GCR = 9; //���ò�����Ϊ19200 �������϶�Ӧ��
    U0BAUD = 59;
    /*.......������4����.......................................*/
  
    UTX0IF = 0;    
    EA = 1;   
}



void Uart_tx_string(char *data_tx,int len)  
{   unsigned int j;  
    for(j=0;j<len;j++)  
    {   U0DBUF = *data_tx++;  
        while(UTX0IF == 0);  
        UTX0IF = 0;   
    }
} 



#pragma vector = T1_VECTOR
__interrupt void t1( )
{
    T1IF = 0;//�����ʱ��1�жϱ�־
    count++; //�ۼ��жϴ���
    
     /*.......������5��ʼ�� ��ʱ1��󣬲ɼ�һ�δ�������ֵ��Ҫ��
    ���ô������е�"100"�ĳ���ȷ��ֵ���Ҳɼ���������ֵ*/
    if(count==4)// ��ʱ1�뵽
    {
        light_val = Get_adc();//��ȡ����ֵ
    
    /*.......������5����.......................................*/
        if(light_val > 50)
          light_flag = 1;//����ǿ
        else
          light_flag = 0;//�����޹���
        
        if(light_flag_last != light_flag)//������״̬�����ı�ʱ
        {
          
           /*.......������6��ʼ�����ݹ���״̬����������LED�������й���������LED������û������*/
            if(light_flag == 1)
            {
               LED1 = 1;
               LED2 = 1;
            }
            else
            {
               LED1 = 0;
               LED2 = 0;
            }
           /*.......������6����.......................................*/
            
           /*.......������7��ʼ�������������Ҫ����֡���Ϸ���������ʾ*/
            output[0] = light_val/100+'0';
            output[1] = '.';
            output[2] = light_val/10%10+'0';
            output[3] = light_val%10+'0';
            output[4] = 'V';
            output[5] = '\r';
            output[6] = '\n';
            output[7] = '\0';
            
            Uart_tx_string("����ֵ��",sizeof("����ֵ��"));
            Uart_tx_string(output,8);//���ʹ������ݵ�����
            /*.......������7����.......................................*/
            
            light_flag_last = light_flag;//������һ�εĹ���״̬
        }
        
        count = 0; //����ֵ����
    }
}
void main( )
{
    InitCLK();//ϵͳʱ��32M
    InitLED();//�Ƶĳ�ʼ��   
    InitUART0();//���ڳ�ʼ��
    InitT1();//��ʱ����ʼ��
    /*.......������1��ʼ������ʼ״̬....................*/
    LED1 = 0;
    LED2 = 0;  
    /*.......������1����.......................................*/
    while(1)
    {
    }
  }