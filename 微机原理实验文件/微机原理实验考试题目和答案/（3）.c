//将16位开关的输入当作一个有符号二进制数，用最左边的四个7段数码管显示该二进制数的补码对应的十六进制数。只有按下任意Button，才可以改变输出。
#include "stdio.h"
#include "xil_io.h"
#include "xgpio_l.h"
#include "xtmrctr_l.h"
#include "xintc_l.h"
#define RESET_VALUE 10000
void Seg_TimerCounterHandler();
void BtnHandler();
void SwitchHandler();
void My_ISR()__attribute__((interrupt_handler));//总中断服务程序
char segtable[16]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x98,0x88,0x83,0xc6,0xa1,0x86,0x8e};
char segcode[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
int i=0;
short pos=0xff7f;
unsigned short switch_status;
int main()
{
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_TRI_OFFSET,0xffff);//设定开关输入方式
	Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_TRI_OFFSET,0x0);//七段数码管位选输出方式
	Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_TRI2_OFFSET,0x0);//七段数码管段选输出方式
	Xil_Out32(XPAR_AXI_GPIO_2_BASEADDR+XGPIO_TRI_OFFSET,0x1f);//按钮输入方式
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_IER_OFFSET,XGPIO_IR_CH1_MASK);//GPIO通道一允许中断
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_GIE_OFFSET,XGPIO_GIE_GINTR_ENABLE_MASK);//允许GPIO中断输出
	Xil_Out32(XPAR_AXI_GPIO_2_BASEADDR+XGPIO_IER_OFFSET,XGPIO_IR_CH1_MASK);//GPIO2通道一允许中断
	Xil_Out32(XPAR_AXI_GPIO_2_BASEADDR+XGPIO_GIE_OFFSET,XGPIO_GIE_GINTR_ENABLE_MASK);//允许GPIO2中断输出
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
			Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_ENABLE_TMR_MASK);//写TCSR
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,RESET_VALUE);//写TLR，预置计数初值
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
			Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)|XTC_CSR_LOAD_MASK);//获取计数初值
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
			(Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_LOAD_MASK)\
			|XTC_CSR_ENABLE_TMR_MASK|XTC_CSR_AUTO_RELOAD_MASK|XTC_CSR_ENABLE_INT_MASK|XTC_CSR_DOWN_COUNT_MASK);
	//开始计时运行，自动计数，允许中断，减计数
	Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_IER_OFFSET,XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK|XPAR_AXI_GPIO_2_IP2INTC_IRPT_MASK|XPAR_AXI_TIMER_0_INTERRUPT_MASK);//中断使能
	Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_MER_OFFSET,XIN_INT_MASTER_ENABLE_MASK|XIN_INT_HARDWARE_ENABLE_MASK);
	microblaze_enable_interrupts();//微处理器开放中断
	return 0;
}
void My_ISR()
{
	int status;
		status=Xil_In32(XPAR_AXI_INTC_0_BASEADDR+XIN_ISR_OFFSET);//读取ISR
	 if((status&XPAR_AXI_GPIO_2_IP2INTC_IRPT_MASK)==XPAR_AXI_GPIO_2_IP2INTC_IRPT_MASK)
			BtnHandler();//调用按钮中断程序
	 else if((status&XPAR_AXI_TIMER_0_INTERRUPT_MASK)==XPAR_AXI_TIMER_0_INTERRUPT_MASK)
			Seg_TimerCounterHandler();//调用七段数码管中断程序
	Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_IAR_OFFSET,status);//写IAR
}
void SwitchHandler()//开关事务处理
{
	int index;
	switch_status=Xil_In32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_DATA_OFFSET);//读取开关状态
	switch_status=switch_status&0xffff;
    if(switch_status>=0x8000)
    {
    	switch_status=switch_status&0x7fff;
    	switch_status=~switch_status;
    	switch_status=switch_status+1;
    }
    for(index=0;index<4;index++)//低4位熄灭
    	    			{
    	    				segcode[7-index]=0xff;
    	    			}
    for(index=0;index<4;index++)
    	    			{
    	    				segcode[3-index]=segtable[(switch_status>>(index*4))&0xf];//得到4位16进制段码
    	    			}
	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_ISR_OFFSET,Xil_In32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_ISR_OFFSET));//清除中断
}
void BtnHandler()//按钮事务处理
{
	int status;
	status=Xil_In32(XPAR_AXI_INTC_0_BASEADDR+XIN_ISR_OFFSET);//读取ISR
	if((status&XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK)==XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK)
		 		SwitchHandler();//调用开关中断程序
	Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_IAR_OFFSET,status);//写IAR
	Xil_Out32(XPAR_AXI_GPIO_2_BASEADDR+XGPIO_ISR_OFFSET,Xil_In32(XPAR_AXI_GPIO_2_BASEADDR+XGPIO_ISR_OFFSET));//清除中断
}
void Seg_TimerCounterHandler()//七段数码管中断调用
{

		Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_DATA2_OFFSET,segcode[i]);
		Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_DATA_OFFSET,pos);
		pos=pos>>1;//位码动态扫描
		i++;
		if(i==8)
		{
			i=0;
			pos=0xff7f;
		}
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET));//清除中断
}

	
