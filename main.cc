#include "m3_for_arty.h"  // Platform specific
#include "xparameters.h"  // Identifying peripherals
#include "xuartlite.h"    // UART IP header file
#include "xil_printf.h"   // Using print statements
#include "xtmrctr.h"      // Timer IP header file

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Include Model file
#include "tensorflow/lite/micro/examples/hello_world/model.h"
/*----------------------------------------------------------------*/

// Declare UART IP instance
static XUartLite UART_instance;

typedef struct {
	XTmrCtr m_AxiTimer;
	unsigned int m_tickCounter1;
	unsigned int m_tickCounter2;
	double m_clockPerioduSeconds;
	double m_timerClockFreq;
}timer;
timer tm;

unsigned int startTimer() {
	// Start timer 0
	XTmrCtr_Reset(&tm.m_AxiTimer,0);
	tm.m_tickCounter1 =  XTmrCtr_GetValue(&tm.m_AxiTimer, 0);
	XTmrCtr_Start(&tm.m_AxiTimer, 0);
	return tm.m_tickCounter1;
}

unsigned int stopTimer() {
	XTmrCtr_Stop(&tm.m_AxiTimer, 0);
	tm.m_tickCounter2 =  XTmrCtr_GetValue(&tm.m_AxiTimer, 0);
	return tm.m_tickCounter2 - tm.m_tickCounter1;
}

float getElapsedTimerInuSeconds() {
	float elapsedTimeInuSeconds = (float)(tm.m_tickCounter2 - tm.m_tickCounter1) * tm.m_clockPerioduSeconds;
	return elapsedTimeInuSeconds;
}

namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  TfLiteTensor* model_output = nullptr;

  // Create an area of memory to use for input, output, and other TensorFlow
  // arrays. You'll need to adjust this by compiling, running, and looking
  // for errors.
  constexpr int kTensorArenaSize = 2 * 1024;
  __attribute__((aligned(16)))uint8_t tensor_arena[kTensorArenaSize];
}

int main(void)
{
  /* USER CODE BEGIN 1 */
	XTmrCtr_Initialize(&tm.m_AxiTimer, XPAR_TMRCTR_0_DEVICE_ID);  //Initialize Timer
	tm.m_timerClockFreq = (double) XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;
	tm.m_clockPerioduSeconds = (double)1000000/tm.m_timerClockFreq;
	
  // Initialize the UART peripheral
	XUartLite_Initialize(&UART_instance, XPAR_AXI_UARTLITE_0_DEVICE_ID);
	NVIC_ClearAllPendingIRQ();

	print("Hello World\r\n");
	
  char buf[50];
  int buf_len = 0;
  TfLiteStatus tflite_status;
  uint32_t num_elements;
  uint32_t timestamp;

  float y_val;

  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  error_reporter->Report("ARM Soft-IP TensorFlow Lite test");

  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
	 error_reporter->Report("Model version does not match Schema");
	 while(1);
  }

  static tflite::MicroMutableOpResolver<1> micro_op_resolver;

  tflite_status = micro_op_resolver.AddFullyConnected();
  if (tflite_status != kTfLiteOk)
  {
  	error_reporter->Report("Could not add FULLY CONNECTED op");
  	while(1);
  }

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  tflite_status = interpreter->AllocateTensors();
  if (tflite_status != kTfLiteOk)
  {
    error_reporter->Report("AllocateTensors() failed");
    while(1);
  }

  // Assign model input and output buffers (tensors) to pointers
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);
	
	num_elements = model_input->bytes / sizeof(float);
  buf_len = sprintf(buf, "Number of input elements: %u\r\n", num_elements);
	print(buf);

  while (1)
  {
		model_input->data.f[0] = 2.0f;

		startTimer();
		
		tflite_status = interpreter->Invoke();
		
		stopTimer();
		
		if (tflite_status != kTfLiteOk)
		{
			error_reporter->Report("Invoke failed");
		}
		
		y_val = model_output->data.f[0];
		float tot_time = getElapsedTimerInuSeconds();
		
		buf_len = sprintf(buf,
		"Predicted: %f | Duration(us): %f\r\n",
							y_val, tot_time);
		print(buf);
		for(int i=0;i<1000000;i++);
  }
}

extern "C" void DebugLog(const char* s)
{
	print(s);
}
