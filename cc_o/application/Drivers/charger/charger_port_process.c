#include "charger_port_process.h"

const char *stop_charge_reason[]={
    "unkown",
    "set port",
    "start_port",
    "recv stop cmd",
    "over load",
    "low current",
    "timeout",
    "consume out",
};

int port_vnic_init(void)
{
    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    return 0;
}

int port_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = CHARGER_RELAY_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, CHARGER_RELAY_GPIO, GPIO_PIN_RESET);


    GPIO_InitStruct.Pin = CHARGER_DETECT_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    return 0;
}

int port_process_init(void)
{
    port_gpio_init();
    port_vnic_init();
    port1_process_init();
    port2_process_init();
    port3_process_init();
    port4_process_init();
    port5_process_init();
    port6_process_init();
    port7_process_init();
    port8_process_init();
    port9_process_init();
    port10_process_init();
    return 0;
}

