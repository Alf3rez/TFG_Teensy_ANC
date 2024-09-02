#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=332,263
AudioInputI2SQuad        i2s_quad1;      //xy=341,327
AudioSynthWaveform       waveform1;      //xy=344,402
AudioMixer4              mixer1;         //xy=554,292
AudioMixer4              mixer2;         //xy=554,355
AudioFilterFIR           fir1;           //xy=726,309
AudioFilterFIR           fir2;           //xy=726,342
AudioOutputI2S           i2s1;           //xy=866,325
AudioConnection          patchCord1(usb1, 0, mixer1, 0);
AudioConnection          patchCord2(usb1, 1, mixer2, 0);
AudioConnection          patchCord3(i2s_quad1, 2, mixer1, 1);
AudioConnection          patchCord4(i2s_quad1, 3, mixer2, 1);
AudioConnection          patchCord5(waveform1, 0, mixer1, 2);
AudioConnection          patchCord6(waveform1, 0, mixer2, 2);
AudioConnection          patchCord7(mixer1, fir1);
AudioConnection          patchCord8(mixer2, fir2);
AudioConnection          patchCord9(fir1, 0, i2s1, 0);
AudioConnection          patchCord10(fir2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=563,244
AudioControlSGTL5000     sgtl5000_2;     //xy=564,403
// GUItool: end automatically generated code

Encoder encoder(4, 5);                  // Inicio el encoder con sus pines correspondientes

float vol = 0.5;                        // Defino el nivel de volumen para ambos Audio Shield
float j = 0.0;                          // Defino la variable para activar el seno

void setup() {

  Serial.begin(115200);
  AudioMemory(12);
  
  // Inicializo el Audio Shield 1
  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.volume(vol);

  // Inicializo el Audio Shield 2
  sgtl5000_2.setAddress(HIGH);
  sgtl5000_2.enable();
  sgtl5000_2.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_2.volume(vol);
  
  // Se configuran los mixer para que pueda contar con las 3 entradas de audio
  mixer1.gain(0, 1.0);    // Canal izquierdo de la entrada USB
  mixer1.gain(1, 1.0);    // Canal izquierdo de la entrada de línea
  mixer1.gain(2, 1.0);    // Canal izquierdo de un seno generado

  mixer2.gain(0, 1.0);    // Canal derecho de la entrada USB
  mixer2.gain(1, 1.0);    // Canal derecho de la entrada de línea
  mixer2.gain(2, 1.0);    // Canal derecho de un seno generado

  // Inicio los filtros FIR
  fir1.begin(FIR_PASSTHRU, 0);
  fir2.begin(FIR_PASSTHRU, 0);

  
}

void loop() {

  // Encoder
  encoderFN();

  // En caso de no emplear el generador de ondas, comentar esta sección
  waveform1.begin(j, 4000, WAVEFORM_SINE);


}


/* FUNCIONES AUXILIARES */

// Función Encoder
void encoderFN() {

  int i = 0;
  i = i + encoder.read(); // Cada giro del encoder detecta los pasos, de enclavamiento a enclavamiento son cuatro pasos o bien positivos o bien negativos según el sentido, así que sumará o restará 1 cada vez.

  if (i <= -4){
    if (j <= 0.1) {
      j += 0.01;
    }
    encoder.write(0);     // En caso de ser menor a 4 se habrá realizado un giro en el sentido de auento, por tanto se realiza la suma a la variable j y se reinicia el encoder a 0. En el caso contrario se hará lo opuesto y también se reinicia a 0.
  }else if (i >= 4){
    if (j > 0) {
      j -= 0.01;
    }
    encoder.write(0);
  }
}