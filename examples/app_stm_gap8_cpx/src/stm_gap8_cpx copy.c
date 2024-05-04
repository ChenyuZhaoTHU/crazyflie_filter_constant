/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2023 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * App layer application that communicates with the GAP8 on an AI deck.
 */


#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#include "cpx.h"
#include "cpx_internal_router.h"

#include "FreeRTOS.h"
#include "task.h"

#define DEBUG_MODULE "APP"
#include "debug.h"



int accZ = 0;
int motor1 = 0;
int motor2 = 0;
int motor3 = 0;
int motor4 = 0;
int r_square = 0;
float thrust = 0;
float Fa = 0.0;

#define mass 0.036 // mass
#define g 9.81 // gravity
#define rho 1.225 // air density
#define D 0.05  // diameter of the rotor
#define rpm2rs 3600
#define C_t_fixed 0.09937873049125099
#define DATA_DIMENSION 1

// #include <stdio.h>
#include <math.h>
#include <stdlib.h>
// #include <string.h>

#define PI 3.14159265358979323846
#define QUEUE_SIZE 150
// 复数结构体
typedef struct {
    int real;
    int imag;
} Complex;



typedef struct {
    double data[QUEUE_SIZE];  // 二维数组
    int front;
    int rear;
    int size; 
} CircularQueue;

typedef struct {
    int32_t data[61];  // 二维数组
    int front;
    int rear;
    int size; 
} CircularQueueCFAR;


typedef struct {
    int16_t data[10];  // 二维数组
    int front;
    int rear;
    int size; 
} CircularQueue10;

void cfar_so(CircularQueueCFAR *queue, int N, int pro_N, double PAD, double *XT, int *target_s2);
double average(double *arr, int len);


void initQueue(CircularQueue *q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}
void initQueueCFAR(CircularQueueCFAR *q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}
void initQueue10(CircularQueue10 *q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}


double average(double *arr, int len) {
    double sum = 0.0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum / len;
}

int isFull(CircularQueue *q) {
    return q->size == QUEUE_SIZE;
}

int isEmpty(CircularQueue *q) {
    return q->size == 0;
}

int isFullCFAR(CircularQueueCFAR *q) {
    return q->size == 61;
}

int isEmptyCFAR(CircularQueueCFAR *q) {
    return q->size == 0;
}

int isFull10(CircularQueue10 *q) {
    return q->size == 10;
}

int isEmpty10(CircularQueue10 *q) {
    return q->size == 0;
}


void enqueue(CircularQueue *queue, int16_t element) {
    if (queue->size >= QUEUE_SIZE) {
        for (int i = 0; i < QUEUE_SIZE - 1; i++) {
            for (int j = 0; j < DATA_DIMENSION; j++) {
                queue->data[i * DATA_DIMENSION + j] = queue->data[(i + 1) * DATA_DIMENSION + j];
            }
        }
        queue->rear = (QUEUE_SIZE - 1) * DATA_DIMENSION; // Update rear index

    } else {
        queue->size++;
    }
    // Copy new element to the rear position
    for (int i = 0; i < DATA_DIMENSION; i++) {
        queue->data[queue->rear + i] = element;
    }
    // cpxPrintToConsole(LOG_TO_CRTP,"dataQueue->size:%hd\n",queue->size);
    queue->rear = (queue->rear + DATA_DIMENSION) % (QUEUE_SIZE * DATA_DIMENSION);
    // cpxPrintToConsole(LOG_TO_CRTP,"queue->rear:%hd\n",queue->rear);

}



void enqueueCFAR(CircularQueueCFAR *q, int16_t data) {
    if (isFullCFAR(q)) {
        // 如果队列已满，弹出队列头部的数据
        q->front = (q->front + 1) % 61;
        q->size--;
    }
  
    q->data[q->rear] = data;
    
    q->rear = (q->rear + 1) % 61;
    q->size++;
}

void enqueue10(CircularQueue10 *q, int16_t data) {
    if (isFull10(q)) {
        // 如果队列已满，弹出队列头部的数据
        q->front = (q->front + 1) % 10;
        q->size--;
    }
  
    q->data[q->rear] = data;
    
    q->rear = (q->rear + 1) % 10;
    q->size++;
}


// 计算短时傅里叶变换（STFT）
int STFTandextraction(CircularQueue* queue, int sampleRate, int segmentLength, int overlapPoints) {
    int* signal = (int*)malloc(segmentLength*sizeof(int));
    int index = queue->front;
    int i=0;
    while (i<segmentLength) {
        // printf("%d ", queue->rear);
        index = (index + 1) % segmentLength;
        signal[i] = queue->data[index];
        i++;
    }
    // 初始化STFT结果数组
    // 对每个片段进行傅里叶变换
    Complex* spectrum = (Complex*)malloc(segmentLength * sizeof(Complex));
    double* window = (double*)malloc(segmentLength * sizeof(double));
    for (int i = 0; i < segmentLength; ++i) {
        window[i] = (float32_t)0.54 - (float32_t)0.46 * arm_cos_f32(2 * PI * i / (segmentLength - 1));
    }
    for (int j = 0; j < segmentLength; ++j) {
            spectrum[j].real = signal[j] * window[j];
            spectrum[j].imag = 0;
        }
    free(signal);
    int j;
    int n = 1;  // 每次蝶形操作处理的数据点数量，从1开始
    int m;      // 蝶形操作的组号
    // int step;   // 步长

    // 计算蝶形操作的总阶数
    while (n < segmentLength) {
        n *= 2;
    }
    m = segmentLength / 2;

    // 重新排列输入序列
    for (int i = 0; i < segmentLength; ++i) {
        if (i < m) {
            j = i * 2;
        } else {
            j = (i - m) * 2 + 1;
        }
        if (j < i) {
            Complex temp = spectrum[i];
            spectrum[i] = spectrum[j];
            spectrum[j] = temp;
        }
    }
    // 执行蝶形操作
    for (int step = 2; step <= segmentLength; step *= 2) {
        double angle = -2 * PI / step;
        Complex w = {arm_cos_f32(angle), arm_sin_f32(angle)};
        Complex u = {1.0, 0.0};

        for (int m = 0; m < step / 2; ++m) {
            for (int k = m; k < segmentLength; k += step) {
                int j = k + step / 2;
                Complex t = {
                    u.real * spectrum[j].real - u.imag * spectrum[j].imag,
                    u.imag * spectrum[j].real + u.real * spectrum[j].imag
                };
                spectrum[j].real = spectrum[k].real - t.real;
                spectrum[j].imag = spectrum[k].imag - t.imag;
                spectrum[k].real += t.real;
                spectrum[k].imag += t.imag;
            }
            Complex temp = {
                u.real * w.real - u.imag * w.imag,
                u.imag * w.real + u.real * w.imag
            };
            u = temp;
        }
    }

    free(window);
    int h = 22222;
    free(spectrum);
    return h;
    
}



void cfar_so(CircularQueueCFAR *queue, int N, int pro_N, double PAD, double *XT, int *target_s2) {
    
    double alpha = N * (pow(PAD, -1.0 / N) - 1);
    double left_N[N/2];

    int index = queue->front-1;
    int i=0;
    while (i<50) {
        index = (index + 1) % 61;
        left_N[i] = queue->data[index];
        i++;
    }



    double Z = average(left_N, 50);
    *XT = Z * alpha;
    if (queue->data[queue->rear-1] > Z * alpha) {
        
        *target_s2 = 1;
       
    }
}






int16_t s1_calFa(double accZ, double motor1, double motor2,double motor3,double motor4){ // measurement: mili Newtow (mN)
  
//   int m1_rpm = -0.0008596853063780414 * accZ * accZ * accZ * accZ + 0.19747266568055943 * accZ * accZ * accZ -16.0526867466245 * accZ * accZ + 727.3007783602438 * accZ + 244;
    r_square= (motor1*motor1 + motor2*motor2 + motor3*motor3 + motor4*motor4)*0.18*0.24;
    thrust = C_t_fixed / rpm2rs * rho * D*D*D*D * 4 * r_square;
    Fa = (int16_t)(mass * accZ) - thrust*1000; // asume R=[0,0,1]
    return (int16_t)Fa; // measurement: mili Newtow (mN)
}

#define FFT_SIZE 1024
// 定义输入信号数组
float32_t inputSignal[2 * FFT_SIZE];
int DSP_Test() {
    // 初始化CMSIS-DSP库
    arm_cfft_radix2_instance_f32 fftInstance;
    arm_cfft_radix2_init_f32(&fftInstance, FFT_SIZE, 0, 1);

    // 填充输入信号数组（示例中使用随机数据填充）
    for (int i = 0; i < 2 * FFT_SIZE; i++) {
        inputSignal[i] = (float32_t)rand() / 100; // 替换为您的实际数据源
    }

    // 执行FFT变换
    arm_cfft_radix2_f32(&fftInstance, inputSignal);

    // 分析结果（在此示例中，仅打印前10个频谱数据）
    // for (int i = 0; i < 10; i++) {
    //     printf("%.2f + %.2fi\n", inputSignal[2*i], inputSignal[2*i+1]);
    // }
    DEBUG_PRINT("Hello! I am the stm_gap8_cpx app\n");
    return 0;
}

void appMain() {
    DEBUG_PRINT("Hello! I am the stm_gap8_cpx app\n");


    // print through cpx
    DEBUG_PRINT("Hello! I am the stm_gap8_cpx app\n");
    // Register a callback for CPX packets.
    // Packets sent to destination=CPX_T_STM32 and function=CPX_F_APP will arrive here
    // cpxRegisterAppMessageHandler(cpxPacketCallback);
    
    // 输出fusion_results
    CircularQueueCFAR* fusion_dataQueue=(CircularQueueCFAR*)malloc(sizeof(CircularQueueCFAR));
    initQueueCFAR(fusion_dataQueue);
    CircularQueue10* Fa_dataQueue = (CircularQueue10*)malloc(sizeof(CircularQueue10));
    initQueue10(Fa_dataQueue);

    // 输入信号
    CircularQueue* accZ_dataQueue = (CircularQueue*)malloc(sizeof(CircularQueue));
    CircularQueue* gyroY_dataQueue = (CircularQueue*)malloc(sizeof(CircularQueue));
    CircularQueue* motor2_1_dataQueue = (CircularQueue*)malloc((sizeof(CircularQueue)));
    CircularQueue* accX_dataQueue = (CircularQueue*)malloc(sizeof(CircularQueue));
    CircularQueue* motor3_1_dataQueue = (CircularQueue*)malloc(sizeof(CircularQueue));
    initQueue(accZ_dataQueue);
    initQueue(gyroY_dataQueue);
    initQueue(motor2_1_dataQueue);
    initQueue(accX_dataQueue);
    initQueue(motor3_1_dataQueue);
    DEBUG_PRINT("initial sucess\n");


    while(1){
    //     // !!! Replace following rows with LogGet() function
        vTaskDelay(M2T(200));
        enqueue(accZ_dataQueue, (int16_t)12);
        enqueue(gyroY_dataQueue, (int16_t)12);
        enqueue(motor2_1_dataQueue, (int16_t)12);
        enqueue(accX_dataQueue, 12);
        enqueue(motor3_1_dataQueue, 12);

        // int signalLength = sizeof(signal_accZ) / sizeof(double);
        // int sampleRate = 100;  // 采样率
        // int segmentLength = 100; // 每个片段的长度
        // int overlapPoints = 99; // 片段之间重叠的点数
        DSP_Test();
        // 执行STFT
        // double* f1 = STFT(accZ_dataQueue, sampleRate, segmentLength, overlapPoints);
        // double* f2 = (double*)malloc(sizeof(double));
        // uint64_t* sumpower1 = (uint64_t*)(malloc(sizeof(uint64_t)));
        // STFTandextraction(gyroY_dataQueue, sampleRate, segmentLength, overlapPoints);
        // enqueue10(Fa_dataQueue,*sumpower1);
        // free(sumpower1);
        // double* f3 = STFT(&motor2_1_dataQueue, sampleRate, segmentLength, overlapPoints);
        // double* f4 = STFT(accX_dataQueue, sampleRate, segmentLength, overlapPoints);
        // double* f5 = STFT(&motor3_1_dataQueue, sampleRate, segmentLength, overlapPoints);

        // // Select 6-8Hz
        // int sumpower1 = (f1[12]*f1[12]+f1[13]*f1[13]+f1[14]*f1[14]+f1[15]*f1[15]+f1[16]*f1[16])/1e6;
        // int sumpower2 = (int)malloc(sizeof(int));
        // sumpower2 = (f2[12]*f2[12]+f2[13]*f2[13]+f2[14]*f2[14]+f2[15]*f2[15]+f2[16]*f2[16])/100000;
        // // int sumpower3 = (f3[12]*f3[12]+f3[13]*f3[13]+f3[14]*f3[14]+f3[15]*f3[15]+f3[16]*f3[16])/1000000;
        // int sumpower4 = (int)malloc(sizeof(int));
        // sumpower4 = (f4[12]*f4[12]+f4[13]*f4[13]+f4[14]*f4[14]+f4[15]*f4[15]+f4[16]*f4[16])/100000;
        // // int sumpower5 = (f5[12]*f5[12]+f5[13]*f5[13]+f5[14]*f5[14]+f5[15]*f5[15]+f5[16]*f5[16])/1000000;
        

        // int fusion_result = (int)malloc(sizeof(int));
        // fusion_result = sumpower1;
        

        // DEBUG_PRINT("is entry (%u)\n", (uint16_t)*sumpower1);

        // // CFAR detection  Part
        // enqueueCFAR(fusion_dataQueue, fusion_result);
        // int N = 100;
        // int pro_N = 20;
        // double PAD = 1e-4;
        // double XT = 0.0;
        int target_s1 = 0;
        // int target_s2 = 0;
        
        // cfar_so(fusion_dataQueue, N, pro_N, PAD, &XT, &target_s2);

       
        
        int Fa_raw = s1_calFa(34, 53,89, 13, 43);
        
        enqueue10(Fa_dataQueue,Fa_raw);
        int Fa_filtered = average((double*)Fa_dataQueue->data, 10);
        
        if(Fa_filtered>30){
            target_s1 = 1;
        }

        // uint16_t is_entry = target_s1 * target_s2;
        
        
        DEBUG_PRINT("is entry (%u)\n", (uint16_t)target_s1);

    }

    


}



// void start_example(void) {
  // pi_bsp_init();
  // cpxInit();
  // cpxEnableFunction(CPX_F_APP);

  // cpxPrintToConsole(LOG_TO_CRTP, "Starting counter bouncer\n");

  // while (1) {
  //   cpxReceivePacketBlocking(CPX_F_APP, &rxPacket);
  //   uint8_t counterInStm = rxPacket.data[0];

  //   // cpxPrintToConsole(LOG_TO_CRTP, "Got packet from the STM (%u)\n", counterInStm);

  //   // Bounce the same value back to the STM
  //   cpxInitRoute(CPX_T_GAP8, CPX_T_STM32, CPX_F_APP, &txPacket.route);
  //   txPacket.data[0] = counterInStm;
  //   txPacket.dataLength = 1;

  //   cpxSendPacketBlocking(&txPacket);
  // }

  // step12();

// }


// void appMain() {
//   DEBUG_PRINT("Hello! I am the stm_gap8_cpx app\n");

  // Register a callback for CPX packets.
  // Packets sent to destination=CPX_T_STM32 and function=CPX_F_APP will arrive here



  // while(1) {
  //   vTaskDelay(M2T(2000));

  //   cpxInitRoute(CPX_T_STM32, CPX_T_GAP8, CPX_F_APP, &txPacket.route);
  //   txPacket.data[0] = counter;
  //   txPacket.dataLength = 1;

  //   cpxSendPacketBlocking(&txPacket);
  //   DEBUG_PRINT("Sent packet to GAP8 (%u)\n", counter);
  //   counter++;
  // }
  // start_example();
//   step12();
// }
