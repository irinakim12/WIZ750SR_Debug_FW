;/******************************************************************************************************************************************************
;* Copyright ¡§I 2016 <WIZnet Co.,Ltd.> 
;* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ¢®¡ÆSoftware¢®¡¾), 
;* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
;* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;*
;* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;
;* THE SOFTWARE IS PROVIDED ¢®¡ÆAS IS¢®¡¾, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
;* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
;* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
;* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;*********************************************************************************************************************************************************/
;/**************************************************************************/
;/**
; * @file    startup_W7500x.s 
; * @author  IOP Team
; * @version V1.0.0
; * @date    01-May-2015
; * @brief   CMSIS Cortex-M0 Core Device Startup File for Device W7500x
; ******************************************************************************
; *
; * @attention
; * @par Revision history
; *    <2015/05/01> 1st Release
; *
; * <h2><center>&copy; COPYRIGHT 2015 WIZnet Co.,Ltd.</center></h2>
; ******************************************************************************
; */


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000800

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000400

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     Remap_NMI_Handler               ; NMI Handler
                DCD     Remap_HardFault_Handler         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     Remap_SVC_Handler               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     Remap_PendSV_Handler            ; PendSV Handler
                DCD     Remap_SysTick_Handler           ; SysTick Handler
                DCD     Remap_SSP0_Handler              ; 16+ 0: SSP 0 Handler                   
                DCD     Remap_SSP1_Handler              ; 16+ 1: SSP 1 Handler                   
                DCD     Remap_UART0_Handler             ; 16+ 2: UART 0 Handler                  
                DCD     Remap_UART1_Handler             ; 16+ 3: UART 1 Handler                  
                DCD     Remap_UART2_Handler             ; 16+ 4: UART 2 Handler                  
                DCD     Remap_I2C0_Handler              ; 16+ 5: I2C 0 Handler                   
                DCD     Remap_I2C1_Handler              ; 16+ 6: I2C 1 Handler                   
                DCD     Remap_PORT0_Handler             ; 16+ 7: GPIO Port 0 Combined Handler    
                DCD     Remap_PORT1_Handler             ; 16+ 8: GPIO Port 1 Combined Handler    
                DCD     Remap_PORT2_Handler             ; 16+ 9: GPIO Port 2 Combined Handler    
                DCD     Remap_PORT3_Handler             ; 16+10: GPIO Port 3 Combined Handler    
                DCD     Remap_DMA_Handler               ; 16+11: DMA Combined Handler            
	            DCD     Remap_DUALTIMER0_Handler        ; 16+12: Dual timer 0 handler             
	            DCD     Remap_DUALTIMER1_Handler        ; 16+13: Dual timer 1 handler            
                DCD     Remap_PWM0_Handler              ; 16+14: PWM0 Handler                    
                DCD     Remap_PWM1_Handler              ; 16+15: PWM1 Handler                    
                DCD     Remap_PWM2_Handler              ; 16+16: PWM2 Handler                    
                DCD     Remap_PWM3_Handler              ; 16+17: PWM3 Handler                    
                DCD     Remap_PWM4_Handler              ; 16+18: PWM4 Handler                    
                DCD     Remap_PWM5_Handler              ; 16+19: PWM5 Handler                    
                DCD     Remap_PWM6_Handler              ; 16+20: PWM6 Handler                    
                DCD     Remap_PWM7_Handler              ; 16+21: PWM7 Handler                    
                DCD     Remap_RTC_Handler               ; 16+22: RTC Handler                     
                DCD     Remap_ADC_Handler               ; 16+23: ADC Handler                     
                DCD     Remap_WZTOE_Handler             ; 16+24: WZTOE_Handler                   
                DCD     Remap_EXTI_Handler             ; 16+25: EXTI_Handler      					
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

Remap_NMI_Handler     PROC
                EXPORT  Remap_NMI_Handler               [WEAK]
                B       .
                ENDP
Remap_HardFault_Handler PROC
                EXPORT  Remap_HardFault_Handler         [WEAK]
                B       .
                ENDP
Remap_SVC_Handler     PROC
                EXPORT  Remap_SVC_Handler               [WEAK]
                B       .
                ENDP
Remap_PendSV_Handler  PROC
                EXPORT  Remap_PendSV_Handler            [WEAK]
                B       .
                ENDP
Remap_SysTick_Handler PROC
               EXPORT  Remap_SysTick_Handler            [WEAK]
               B       .
               ENDP
Remap_Default_Handler PROC
                EXPORT Remap_SSP0_Handler               [WEAK]
                EXPORT Remap_SSP1_Handler               [WEAK]
                EXPORT Remap_UART0_Handler              [WEAK]
                EXPORT Remap_UART1_Handler              [WEAK]
                EXPORT Remap_UART2_Handler              [WEAK]
                EXPORT Remap_I2C0_Handler               [WEAK]
                EXPORT Remap_I2C1_Handler               [WEAK]
                EXPORT Remap_PORT0_Handler              [WEAK]
                EXPORT Remap_PORT1_Handler              [WEAK]
                EXPORT Remap_PORT2_Handler              [WEAK]
                EXPORT Remap_PORT3_Handler              [WEAK]
                EXPORT Remap_DMA_Handler                [WEAK]
                EXPORT Remap_DUALTIMER0_Handler         [WEAK]
                EXPORT Remap_DUALTIMER1_Handler         [WEAK]
                EXPORT Remap_PWM0_Handler               [WEAK]
                EXPORT Remap_PWM1_Handler               [WEAK]
                EXPORT Remap_PWM2_Handler               [WEAK]
                EXPORT Remap_PWM3_Handler               [WEAK]
                EXPORT Remap_PWM4_Handler               [WEAK]
                EXPORT Remap_PWM5_Handler               [WEAK]
                EXPORT Remap_PWM6_Handler               [WEAK]
                EXPORT Remap_PWM7_Handler               [WEAK]
                EXPORT Remap_RTC_Handler                [WEAK]
                EXPORT Remap_ADC_Handler                [WEAK]
                EXPORT Remap_WZTOE_Handler              [WEAK]
                EXPORT Remap_EXTI_Handler              [WEAK]						
Remap_SSP0_Handler                      
Remap_SSP1_Handler                      
Remap_UART0_Handler                     
Remap_UART1_Handler                     
Remap_UART2_Handler                     
Remap_I2C0_Handler                      
Remap_I2C1_Handler                      
Remap_PORT0_Handler                     
Remap_PORT1_Handler                     
Remap_PORT2_Handler                     
Remap_PORT3_Handler                     
Remap_DMA_Handler                       
Remap_DUALTIMER0_Handler                
Remap_DUALTIMER1_Handler                
Remap_PWM0_Handler                      
Remap_PWM1_Handler                      
Remap_PWM2_Handler                      
Remap_PWM3_Handler                      
Remap_PWM4_Handler                      
Remap_PWM5_Handler                      
Remap_PWM6_Handler                      
Remap_PWM7_Handler                      
Remap_RTC_Handler                       
Remap_ADC_Handler                       
Remap_WZTOE_Handler                
Remap_EXTI_Handler
                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
