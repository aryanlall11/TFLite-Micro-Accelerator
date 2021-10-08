#include "main.h"
#include <stdio.h>
#include <string.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"    // Include the model file

TIM_HandleTypeDef htim14;   // Timer object

UART_HandleTypeDef huart1;  // UART object

// TFLite globals
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  TfLiteTensor* model_output = nullptr;

  // Create an area of memory to use for input, output, and other TensorFlow
  // arrays. You'll need to adjust this by compiling, running, and looking
  // for errors.
  constexpr int kTensorArenaSize = 2 * 1024;     // Tensor arena size in bytes
  __attribute__((aligned(16)))uint8_t tensor_arena[kTensorArenaSize];
} // namespace

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM10_Init(void);
static void MX_USART2_UART_Init(void);

/* main function begin */
int main(void)
{
  char buf[50];
  int buf_len = 0;
  TfLiteStatus tflite_status;
  uint32_t num_elements;
  uint32_t timestamp;

  float y_val;
  
  HAL_Init();   // Initialize HW platform

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM10_Init();
  MX_USART2_UART_Init();

  HAL_TIM_Base_Start(&htim10);  // Start the timer peripheral
  
  /*-------------- TFLite application code ---------------*/

  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  error_reporter->Report("STM32 TensorFlow Lite test");

  model = tflite::GetModel(my_model);   // my_model is the C byte array variable in the "model.h" file
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
  	error_reporter->Report("Model version does not match Schema");
  	while(1);
  }

  /*---------- Manually register the layer operations ----------*/

  static tflite::MicroMutableOpResolver<1> micro_op_resolver;
  tflite_status = micro_op_resolver.AddFullyConnected();   // Register fully connected layer
  if (tflite_status != kTfLiteOk)
  {
    error_reporter->Report("Could not add FULLY CONNECTED op");
    while(1);
  }

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  tflite_status = interpreter->AllocateTensors();   // Allocation of tensors
  if (tflite_status != kTfLiteOk)
  {
    error_reporter->Report("AllocateTensors() failed");
    while(1);
  }

  // Assign model input and output buffers (tensors) to pointers
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);

  // Get number of elements in input tensor (for debugging)
  num_elements = model_input->bytes / sizeof(float);
  buf_len = sprintf(buf, "Number of input elements: %lu\r\n", num_elements);
  HAL_UART_Transmit(&huart2, (uint8_t *)buf, buf_len, 100);   // Transmit via UART

  while (1)
  {
    for (uint32_t i = 0; i < num_elements; i++)
    {
      model_input->data.f[i] = 2.0f; // Assign the required input values
    }

    // Get current timestamp
    timestamp = htim10.Instance->CNT;

    // Run inference
    tflite_status = interpreter->Invoke();   // TFLite kernels are invoked
    if (tflite_status != kTfLiteOk)
    {
      error_reporter->Report("Invoke failed");
    }

    // Read the output of neural network
    y_val = model_output->data.f[0];

    timestamp = htim10.Instance->CNT - timestamp;  // Get total time consumed (in timer cycles)

    buf_len = sprintf(buf,
              "Output: %f | Duration: %lu\r\n",
              y_val, timestamp);
    HAL_UART_Transmit(&huart2, (uint8_t *)buf, buf_len, 100);  // Send the info via UART

    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 79;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/* USER CODE BEGIN 4 */
extern "C" void DebugLog(const char* s)  // Modified Debug function to enable UART access
{
	HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), 100);  // Simply print the error messages, if any
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
