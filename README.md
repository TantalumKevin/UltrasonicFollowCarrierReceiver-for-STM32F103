# Ultrasonic Follow Carrier Receiver for STM32F103

#### 1 介绍
本项目为2021-2022学年第二学期电气工程学院2020级卓越班嵌入式系统与智能设计课程设计：超声波跟随载物平台之超声接收端。<br>
根据设计，STM32F103C6T6A为压电陶瓷驱动电路控制核心MCU，运行C语言代码，驱动传感器与外围电路。
相关PCB设计文件，请移步仓库[Github](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-PCB) & [Gitee](https://gitee.com/kevin_ud/ultrasonic-follow-carrier-receiver-pcb)
#### 2 软件架构
软件架构采用STM32CubeMX选配导出，相关配置如图所示，或直接打开```./Receiver.ioc```文件即可。<br><br>
![Full](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/Full.jpg?raw=true)
![DMA](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/DMA.jpg?raw=true)
![GPIO](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/GPIO.jpg?raw=true)
![ADC](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/ADC.jpg?raw=true)
![TIM1](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/TIM1.jpg?raw=true)
![TIM2](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/TIM2.jpg?raw=true)
![TIM3](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/TIM3.jpg?raw=true)
![UART1](https://github.com/TantalumKevin/UltrasonicFollowCarrierReceiver-for-STM32F103/blob/master/Img/UART1.jpg?raw=true)
<br>
请注意：一切参数配置以```./Receiver.ioc```文件为准，图片更新滞后，并不能保证与最新代码适配。

#### 3 运行流程
```C
  MCU各外设初始化->
  等待串口数据->
  发送串口数据->   
  等待串口数据->   
  //这里的三步串口是为了保证数据通畅的初始化通信
  传感器数据初始化->
┌>循环读取传感器数据->
│ [根据传感器原始数据计算目标相对位置->]
│ 发送串口数据-> ─┐
└────────────────┘
```