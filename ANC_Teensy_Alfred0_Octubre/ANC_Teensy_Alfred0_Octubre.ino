#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>
#include <libmathq15.h>

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
const int size = 128;

double w[] = {-0.00469736414125448,-0.00845567080346478,-0.00955738130013776,-0.00764783948027278,-0.00333867849478365,0.00201000975925008,0.00671184541319939,0.00927549737037603,0.00889326066643777,0.00569007562874221,0.000675200712390193,-0.00456330567284496,-0.00837290847056867,-0.00955171852206775,-0.00772403064203489,-0.003469108738953,0.00186756234553347,0.00659643944039242,0.00922728598134767,0.00892588375898218,0.00578829934081868,0.000814135608056087,-0.00443345366112391,-0.00829246360585894,-0.009545411518768,-0.00779441249139678,-0.00359378471745823,0.00172587528905582,0.00648301627568806,0.00918088280495098,0.00895976561479393,0.00589544839566299,0.000953597544821564,-0.00429909108280126,-0.00820861555772293,-0.00953489276914879,-0.00786309046939154,-0.00371886912821663,0.00158801227752266,0.00637713690282617,0.00913810221279491,0.00899413274076447,0.00599556610317599,0.00109200468885495,-0.00416901297801348,-0.00812503506634997,-0.00952770387883349,-0.0079299114417642,-0.0038445946585028,0.0014468504442768,0.0062626906663711,0.00909142211536289,0.00902915105709584,0.00610360036847026,0.0012400675224402,-0.00402883592027468,-0.00803254281207319,-0.00950998620095959,-0.00799646195437838,-0.00397199395193817,0.00129488324778871,0.00613969728341646,0.00902700562901857,0.00904894347753947,0.00619750594735751,0.00137735788905689,-0.00389397077168632,-0.00794852117551086,-0.00950255085973001,-0.00806801716623452,-0.00409914631297043,0.00114910713621456,0.0060179135283747,0.00896916284602241,0.00907329310807608,0.00629391002644375,0.00150949699951598,-0.00376574319418764,-0.00786410110964959,-0.00948794958899934,-0.00813302148878795,-0.00422194634571721,0.00100444783967135,0.00589507655574524,0.00890771133295813,0.00908470400142576,0.0063747277173799,0.00163342079400946,-0.00363743261010945,-0.00777695925550455,-0.0094727895046942,-0.0081923087090046,-0.00434186456998076,0.000865906586963498,0.00578025222563565,0.00885574788752292,0.00911475071182304,0.00647627715991875,0.00177600975409286,-0.00350111913259297,-0.00768694054444114,-0.00945711780527312,-0.00825640387305933,-0.00446110838738236,0.000726703371409721,0.00566774696558606,0.00880142208240949,0.00913668327194334,0.00657290642544738,0.00191713677029691,-0.00335959000819229,-0.00758532434694106,-0.00943379007650661,-0.00831097944857597,-0.0045801787759045,0.000583323782798151,0.00554672032538055,0.00874030810604871,0.00916040089782836,0.00666764378076455,0.00205533399992578,-0.00322160149180785,-0.0074951699693251,-0.00940751818948448,-0.00837117022087177,-0.00469503059365825,0.000440104725267185,0.00542101502818898};

q15_t coeffs[size];

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

  // Transformo los coeficientes
  transformCoeffs(coeffs, w, size);

  // Inicio los filtros FIR
  fir1.begin(coeffs, size);
  fir2.begin(coeffs, size);

/*  fir1.begin(FIR_PASSTHRU, 0);
  fir2.begin(FIR_PASSTHRU, 0); */

}

void loop() {

  // Encoder
  encoderFN();

  // En caso de no emplear el generador de ondas, comentar esta sección
  //waveform1.begin(j, 4000, WAVEFORM_SINE);


}

/* FUNCIONES AUXILIARES */

// Función Encoder
void encoderFN() {

  int i = 0;
  i = i + encoder.read(); // Cada giro del encoder detecta los pasos, de enclavamiento a enclavamiento son cuatro pasos o bien positivos o bien negativos según el sentido, así que sumará o restará 1 cada vez.

  if (i <= -4){
    if (vol <= 1.0) {
      vol += 0.05;
    }
    encoder.write(0);     // En caso de ser menor a 4 se habrá realizado un giro en el sentido de auento, por tanto se realiza la suma a la variable j y se reinicia el encoder a 0. En el caso contrario se hará lo opuesto y también se reinicia a 0.
  }else if (i >= 4){
    if (vol > 0) {
      vol -= 0.05;
    }
    encoder.write(0);
  }

  sgtl5000_1.volume(vol);
  sgtl5000_2.volume(vol);

}

/* Función Transformación de coeficientes para el filtro FIR
void transformCoeffs(q15_t out[], double in[], int k)
{
  for (int i = 0 ; i < k ; i++)
    out[i] = static_cast<q15_t>(in[i] * 32767.0);
} */

// Función Transformación de coeficientes para el filtro FIR
void transformCoeffs(q15_t out[], double in[], int k)
{
  for (int i = 0 ; i < k ; i++)
    out[i] = q15_from_float(in[i]);
}