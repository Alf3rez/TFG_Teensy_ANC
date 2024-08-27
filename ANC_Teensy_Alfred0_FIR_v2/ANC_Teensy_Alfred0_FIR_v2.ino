#include <_Teensy.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <arm_math.h>
#include <arm_const_structs.h>
#include <Encoder.h>

#define FFT_SIZE 511

float MU = 0.01; // Tasa de aprendizaje

// GUItool: begin automatically generated code
AudioInputI2SQuad        i2s_quad1;         //xy=360.2727355957031,303.8181953430176
AudioInputUSB            usb1;              //xy=368.72727966308594,224.9999942779541
AudioMixer4              mixer1;            //xy=595.2727127075195,201.54543685913086
AudioMixer4              mixer2;            //xy=595.3636436462402,326
AudioAmplifier           amp_left;          //xy=733,228
AudioAmplifier           amp_right;         //xy=733,291
AudioAnalyzeFFT1024      fft1024_1;         //xy=739.8182373046875,171.36367797851562
AudioAnalyzeFFT1024      fft1024_2;         //xy=740.1818237304688,346.45458984375
AudioFilterFIR           firFilter_Right;   //xy=878.0908966064453,291.6363639831543
AudioFilterFIR           firFilter_Left;    //xy=878.8181610107422,228.00000190734863
AudioAnalyzeFFT1024      fft1024_5;         //xy=886,172
AudioAnalyzeFFT1024      fft1024_6;         //xy=888,350
AudioOutputI2S           i2s1;              //xy=1035.5454711914062,253.8182029724121
AudioAnalyzeFFT1024      fft1024_4;         //xy=1041.363540649414,349.5454730987549
AudioAnalyzeFFT1024      fft1024_3;         //xy=1042.7273483276367,175.36364269256592
AudioConnection          patchCord1(i2s_quad1, 0, mixer1, 1);
AudioConnection          patchCord2(i2s_quad1, 1, mixer2, 1);
AudioConnection          patchCord3(usb1, 0, mixer1, 0);
AudioConnection          patchCord4(usb1, 1, mixer2, 0);
AudioConnection          patchCord5(mixer1, amp_left);
AudioConnection          patchCord6(mixer1, fft1024_1);
AudioConnection          patchCord7(mixer2, amp_right);
AudioConnection          patchCord8(mixer2, fft1024_2);
AudioConnection          patchCord9(amp_left, fft1024_5);
AudioConnection          patchCord10(amp_left, firFilter_Left);
AudioConnection          patchCord11(amp_right, firFilter_Right);
AudioConnection          patchCord12(amp_right, fft1024_6);
AudioConnection          patchCord13(firFilter_Right, 0, i2s1, 1);
AudioConnection          patchCord14(firFilter_Right, fft1024_4);
AudioConnection          patchCord15(firFilter_Left, 0, i2s1, 0);
AudioConnection          patchCord16(firFilter_Left, fft1024_3);
AudioControlSGTL5000     sgtl5000_1;     //xy=604.7272644042969,249.00001621246338
AudioControlSGTL5000     sgtl5000_2;     //xy=604.7272338867188,280.3636484146118
// GUItool: end automatically generated code



float vol = 0.5;
const int16_t numCoeffs = 513;

double FIR_Coef_Left[numCoeffs];          // Coeficientes del filtro FIR para el canal izquierdo en double
double FIR_Coef_Right[numCoeffs];         // Coeficientes del filtro FIR para el canal derecho en double
double fft_input_left[FFT_SIZE];          // Buffer para el canal izquierdo de entrada al sistema
double fft_input_right[FFT_SIZE];         // Buffer para el canal derecho de entrada al sistema
double fft_output_left[FFT_SIZE];         // Buffer para el canal izquierdo de salida al control
double fft_output_right[FFT_SIZE];        // Buffer para el canal derecho de salida al control
double fft_filter_left[FFT_SIZE];         // Buffer para el canal izquierdo de entrada al filtro FIR
double fft_filter_right[FFT_SIZE];        // Buffer para el canal derecho de entrada al filtro FIR
double error_signal_left[FFT_SIZE];       // Señal de error para el canal izquierdo
double error_signal_right[FFT_SIZE];      // Señal de error para el canal derecho

short int FIR_Coef_Left_int[numCoeffs];   // Coeficientes del filtro FIR para el canal izquierdo como int16_t
short int FIR_Coef_Right_int[numCoeffs];  // Coeficientes del filtro FIR para el canal derecho como int16_t

Encoder encoder(4, 5);                    // Inicio el encoder con sus pines correspondientes

void setup() {

  Serial.begin(115200);
  AudioMemory(12);
  
  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.volume(vol);

  sgtl5000_2.setAddress(HIGH);
  sgtl5000_2.enable();
  sgtl5000_2.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_2.volume(vol);
  
  // Se configuran los mixer para que sirvan de punto de unión de ambas entradas para cada canal
  mixer1.gain(0, 1.0);
  mixer1.gain(1, 1.0);
  mixer2.gain(0, 1.0);
  mixer2.gain(1, 1.0);

  // Se configuran los amplificadores para que inviertan la señal
  amp_left.gain(-1);
  amp_right.gain(-1);

  // Inicializo los pines del encoder
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  // Debugging
  //q15_t FIR_Coef_Deb[numCoeffs] = {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF};
  //firFilter_Left.begin(FIR_PASSTHRU, 0);
  //firFilter_Right.begin(FIR_PASSTHRU, 0);

  // Inicializar los coeficientes en punto flotante
  init_FIR_coeffs(FIR_Coef_Left);
  init_FIR_coeffs(FIR_Coef_Right);

}

void loop() {
  // Obtener la señal de error de las salidas de FFT

  // Llenar los buffers fft_input y calcular las señales de error
  for (int i = 0; i < FFT_SIZE; i++) {
    double mic_left = fft1024_1.output[i];
    double mic_right = fft1024_2.output[i];
    double filter_left = fft1024_5.output[i];
    double filter_right = fft1024_6.output[i];
    double output_left = fft1024_3.output[i];
    double output_right = fft1024_4.output[i];

    // Llenar los buffers fft_input
    fft_input_left[i] = mic_left;
    fft_input_right[i] = mic_right;

    // Llenar los buffers fft_filter
    fft_filter_left[i] = filter_left;
    fft_filter_right[i] = filter_right;

    // Llenar los buffers fft_output
    fft_output_left[i] = output_left;
    fft_output_right[i] = output_right;

    // Calcular las señales de error
    error_signal_left[i] = fft_input_left[i] - fft_output_left[i];
    error_signal_right[i] = fft_input_right[i] - fft_output_right[i];
  }

  // Actualizar los coeficientes del filtro FIR
  update_FIR_coeffs(FIR_Coef_Left, fft_filter_left, error_signal_left);
  update_FIR_coeffs(FIR_Coef_Right, fft_filter_right, error_signal_right);

  // Transformo los coeficientes para poder utilizarlos en el filtro FIR
  transformCoeffs(FIR_Coef_Left_int, FIR_Coef_Left, numCoeffs);
  transformCoeffs(FIR_Coef_Right_int, FIR_Coef_Right, numCoeffs);

  // Inicializar los filtros FIR con los coeficientes convertidos
  firFilter_Left.begin(FIR_Coef_Left_int, numCoeffs);
  firFilter_Right.begin(FIR_Coef_Right_int, numCoeffs);

  // Encoder MU
  encoderFN();

}

// Funciones auxiliares

void transformCoeffs( short int out[], double in[], int k )
{
  for ( int i = 0 ; i < k ; i++ )
    out[i] = 32768 * in[i];
}

void init_FIR_coeffs(double coeffs[]) {
    for (int i = 0; i < numCoeffs; i++) {
        coeffs[i] = 0.0; // Inicializa los coeficientes a 0
    }
}

void update_FIR_coeffs(double coeffs[], double input[], double error[]) {
    for (int i = 0; i < numCoeffs; i++) {
        coeffs[i] += (MU * error[i] * input[i]); // Actualiza los coeficientes del filtro según la función Lean Mean Square
    }
}

void encoderFN() {
  //if the button is pressed
  int i = 0;
  i = i + encoder.read();

  if (i <= -4){
    MU += 0.05;
    encoder.write(0);
  }else if (i >= 4){
    if (MU > 0.01) {
      MU -= 0.05;
    }
    encoder.write(0);
  }
}