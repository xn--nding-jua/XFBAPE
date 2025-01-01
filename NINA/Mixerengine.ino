void initAudiomixer() {
  // set main-volume for left, right and sub to 100%
  audiomixer.volumeMain = 0.0; // set to -20dBfs
  audiomixer.balanceMain = 50; // set to center
  audiomixer.volumeSub = 0.0; // set to -20dBfs
  audiomixer.volumeCard = 0.0; // set to 0dBfs
  audiomixer.volumeBt = 0.0; // set to 0dBfs
  sendVolumeToFPGA(0); // send main to FPGA

  // set individual channels to 100% and left/right
  uint8_t i;
  for (i=0; i<MAX_AUDIO_CHANNELS; i++) {
    audiomixer.volumeCh[i] = -48; // set channel to 0% = -48dBfs
	
    // pan channels 1..32 to center
    audiomixer.balanceCh[i] = 50; // set to center
	
    sendVolumeToFPGA(i+1); // send values to FPGA
  }
  
  sendStereoVolumeToFPGA(0, 0); // set SD-Card to 0dBfs
  sendStereoVolumeToFPGA(1, -48); // set Bluetooth to -48dBfs

  for (int i=0; i<MAX_ADCS; i++) {
    setADCgain(i, 0);
  }

  // set equalizer-coefficients to standard-values
  resetAudioFilters();
  sendFiltersToFPGA();

  // reset dynamics (noisegates and compressors) to standard-values
  resetDynamics();
  sendDynamicsToFPGA();

  // reset all integrators
  setResetFlags(1, 1, 1, 1); // [eqs, compressor, crossover, upsampler]
  delay(20);
  setResetFlags(0, 0, 0, 0); // [eqs, compressor, crossover, upsampler]
}

void sendStereoVolumeToFPGA(uint8_t channel, float volume) {
  // channel = 0 -> SD-Card
  // channel = 1 -> Bluetooth
  float value = (pow(10, volume/20.0f) * 128.0f); // convert dB to uint8_t

  data_64b fpga_data;
  fpga_data.u32[0] = trunc(value);
  sendDataToFPGA(FPGA_IDX_MAIN_VOL+67+channel, &fpga_data);
}

void sendVolumeToFPGA(int8_t channel) {
  // negative values means we mute this specific channel. With int8_t we can still address 127 channels. If you need more, you have to change to int16_t

  // send volume for left and right for desired channel

  data_64b fpga_data;
  float volume_left;
  float volume_right;
  float volume_sub;

  // value is between 0...100. We will change this value to meet 8bit = 0..256 to make calculation in FPGA a bit easier
  // within FPGA we will do an integer-calculation like: ((AudioSampleData * ReceivedValueFromMicrocontroller) >> 8) = DataForDAC

  if (channel == 0) {
    // main-channels have only left, right and sub, no balance

    // convert dBfs-values into byte-value. -140dBfs = 0, -72dBfs = 0.5x, 0dBfs = 1x, 6dBfs = 2x
    volume_left = (pow(10, audiomixer.volumeMain/20.0f) * 128.0f) * limitMax((100 - audiomixer.balanceMain) * 2, 100) / 100.0f;
    volume_right = (pow(10, audiomixer.volumeMain/20.0f) * 128.0f) * limitMax(audiomixer.balanceMain * 2, 100) / 100.0f;
    volume_sub = (pow(10, audiomixer.volumeSub/20.0f) * 128.0f);

    // send data to FPGA
    fpga_data.u32[0] = trunc(volume_left);
    sendDataToFPGA(FPGA_IDX_MAIN_VOL, &fpga_data);
    fpga_data.u32[0] = trunc(volume_right);
    sendDataToFPGA(FPGA_IDX_MAIN_VOL+1, &fpga_data);
    fpga_data.u32[0] = trunc(volume_sub);
    sendDataToFPGA(FPGA_IDX_MAIN_VOL+2, &fpga_data);
  }else if (channel > 0) {
    // send regular volume to FPGA

    // each channel has two audio-volumes: left and right, so we have to send 2x the values that we have channels
    // convert dBfs-values into byte-value
    volume_left = (pow(10, audiomixer.volumeCh[channel - 1]/20.0f) * 128.0f) * limitMax((100 - audiomixer.balanceCh[channel - 1]) * 2, 100) / 100.0f;
    volume_right = (pow(10, audiomixer.volumeCh[channel - 1]/20.0f) * 128.0f) * limitMax(audiomixer.balanceCh[channel - 1] * 2, 100) / 100.0f;

    fpga_data.u32[0] = trunc(volume_left);
    sendDataToFPGA(FPGA_IDX_CH_VOL + (channel - 1) * 2, &fpga_data); // send data for this channel to main left
    fpga_data.u32[0] = trunc(volume_right);
    sendDataToFPGA(FPGA_IDX_CH_VOL + (channel - 1) * 2 + 1, &fpga_data); // send data for this channel to main right
  }else{
    // mute this specific channel without changing the volume-information in the fpga
    fpga_data.u32[0] = 0;
    sendDataToFPGA(FPGA_IDX_CH_VOL + (channel - 1) * 2, &fpga_data); // send data for this channel to main left
    fpga_data.u32[0] = 0;
    sendDataToFPGA(FPGA_IDX_CH_VOL + (channel - 1) * 2 + 1, &fpga_data); // send data for this channel to main right
  }
}

float logValue(float value, float minIn, float maxIn, float minOut, float maxOut) {
  // position will be between 0 and 100
  float minp = minIn;
  float maxp = maxIn;

  // The result should be between 1 an 100
  float minv = log(minOut + 1); // log() = ln()
  float maxv = log(maxOut + 1); // log() = ln()

  // calculate adjustment factor
  float scale = (maxv-minv) / (maxp-minp);

  return exp(minv + scale*(value - minp)) - 1; // e^()
}

// online-converter for floating-point to fixed-point: https://www.rfwireless-world.com/calculators/floating-vs-fixed-point-converter.html
void recalcFilterCoefficients_PEQ(struct sPEQ *peq) {
  // Online-Calculator: https://www.earlevel.com/main/2021/09/02/biquad-calculator-v3
  // Source: https://www.earlevel.com/main/2012/11/26/biquad-c-source-code

  double V = pow(10.0, fabs(peq->gain)/20.0);
  double K = tan(PI * peq->fc / AUDIO_SAMPLERATE);
  double norm;
  double coeff_a[3];
  double coeff_b[3];

  switch (peq->type) {
    case 0: // allpass
      norm = 1.0 / (1.0 + K * 1.0/peq->Q + K * K);
      coeff_a[0] = (1.0 - K * 1.0/peq->Q + K * K) * norm;
      coeff_a[1] = 2.0 * (K * K - 1.0) * norm;
      coeff_a[2] = 1.0;
      coeff_b[1] = coeff_a[1];
      coeff_b[2] = coeff_a[0];
      break;
    case 1: // peak-filter
      if (peq->gain >= 0) {
        norm = 1.0 / (1.0 + 1.0/peq->Q * K + K * K);
        coeff_a[0] = (1.0 + V/peq->Q * K + K * K) * norm;
        coeff_a[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_a[2] = (1.0 - V/peq->Q * K + K * K) * norm;
        coeff_b[1] = coeff_a[1];
        coeff_b[2] = (1.0 - 1.0/peq->Q * K + K * K) * norm;
      }else{
        norm = 1.0 / (1.0 + V/peq->Q * K + K * K);
        coeff_a[0] = (1.0 + 1.0/peq->Q * K + K * K) * norm;
        coeff_a[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_a[2] = (1.0 - 1.0/peq->Q * K + K * K) * norm;
        coeff_b[1] = coeff_a[1];
        coeff_b[2] = (1.0 - V/peq->Q * K + K * K) * norm;
      }
      break;
    case 2: // low-shelf
      if (peq->gain >= 0) {    // boost
        norm = 1.0 / (1.0 + sqrt(2.0) * K + K * K);
        coeff_a[0] = (1.0 + sqrt(2.0*V) * K + V * K * K) * norm;
        coeff_a[1] = 2.0 * (V * K * K - 1.0) * norm;
        coeff_a[2] = (1.0 - sqrt(2.0*V) * K + V * K * K) * norm;
        coeff_b[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_b[2] = (1.0 - sqrt(2.0) * K + K * K) * norm;
      }
      else {    // cut
        norm = 1.0 / (1.0 + sqrt(2.0*V) * K + V * K * K);
        coeff_a[0] = (1.0 + sqrt(2.0) * K + K * K) * norm;
        coeff_a[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_a[2] = (1.0 - sqrt(2) * K + K * K) * norm;
        coeff_b[1] = 2.0 * (V * K * K - 1.0) * norm;
        coeff_b[2] = (1.0 - sqrt(2.0*V) * K + V * K * K) * norm;
      }
      break;
    case 3: // high-shelf
        if (peq->gain >= 0) {    // boost
          norm = 1.0 / (1.0 + sqrt(2.0) * K + K * K);
          coeff_a[0] = (V + sqrt(2.0*V) * K + K * K) * norm;
          coeff_a[1] = 2.0 * (K * K - V) * norm;
          coeff_a[2] = (V - sqrt(2.0*V) * K + K * K) * norm;
          coeff_b[1] = 2.0 * (K * K - 1.0) * norm;
          coeff_b[2] = (1.0 - sqrt(2.0) * K + K * K) * norm;
        }
        else {    // cut
          norm = 1.0 / (V + sqrt(2.0*V) * K + K * K);
          coeff_a[0] = (1.0 + sqrt(2.0) * K + K * K) * norm;
          coeff_a[1] = 2.0 * (K * K - 1.0) * norm;
          coeff_a[2] = (1.0 - sqrt(2.0) * K + K * K) * norm;
          coeff_b[1] = 2.0 * (K * K - V) * norm;
          coeff_b[2] = (V - sqrt(2.0*V) * K + K * K) * norm;
        }
      break;
    case 4: // bandpass
        norm = 1.0 / (1.0 + K / peq->Q + K * K);
        coeff_a[0] = (K / peq->Q) * norm;
        coeff_a[1] = 0;
        coeff_a[2] = -coeff_a[0];
        coeff_b[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_b[2] = (1.0 - K / peq->Q + K * K) * norm;
      break;
    case 5: // notch
        norm = 1.0 / (1.0 + K / peq->Q + K * K);
        coeff_a[0] = (1.0 + K * K) * norm;
        coeff_a[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_a[2] = coeff_a[0];
        coeff_b[1] = coeff_a[1];
        coeff_b[2] = (1.0 - K / peq->Q + K * K) * norm;
      break;
    case 6: // lowpass
        norm = 1.0 / (1.0 + K / peq->Q + K * K);
        coeff_a[0] = K * K * norm;
        coeff_a[1] = 2.0 * coeff_a[0];
        coeff_a[2] = coeff_a[0];
        coeff_b[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_b[2] = (1.0 - K / peq->Q + K * K) * norm;
      break;
    case 7: // highpass
        norm = 1.0 / (1.0 + K / peq->Q + K * K);
        coeff_a[0] = 1.0 * norm;
        coeff_a[1] = -2.0 * coeff_a[0];
        coeff_a[2] = coeff_a[0];
        coeff_b[1] = 2.0 * (K * K - 1.0) * norm;
        coeff_b[2] = (1.0 - K / peq->Q + K * K) * norm;
      break;
  }

  // convert to Q30-format
  for (int i=0; i<3; i++) {
    peq->a[i].s32 = coeff_a[i] * 1073741823; // convert to Q30
    peq->b[i].s32 = coeff_b[i] * 1073741823; // convert to Q30
  }
}

void recalcFilterCoefficients_LR12(struct sLR12 *LR12) {
  double wc = 2.0 * PI * LR12->fc;
  double wc2 = wc * wc;
  double wc22 = 2 * wc2;
  double k = wc / tan(PI * (LR12->fc / AUDIO_SAMPLERATE));
  double k2 = k * k;
  double k22 = 2 * k2;
  double wck2 = 2 * wc * k;
  double norm = (k2 + wc2 + wck2);

  if (LR12->isHighpass) {
    // coefficients for HighPass-Filter
    LR12->a[0].d = k2 / norm;
    LR12->a[1].d = -k22 / norm;
    LR12->a[2].d = k2 / norm;
  }else{
    // coefficients for LowPass-Filter
    LR12->a[0].d = wc2 / norm;
    LR12->a[1].d = wc22 / norm;
    LR12->a[2].d = wc2 / norm;
  }

  LR12->b[0].d = 0; // we are not using this coefficient but keep it to not confuse with indices
  LR12->b[1].d = (-k22 + wc22) / norm;
  LR12->b[2].d = (-wck2 + k2 + wc2) / norm;

  // convert to Q36-format
  for (int i=0; i<3; i++) {
    LR12->a[i].s64 = LR12->a[i].d * 68719476735; // convert to Q36
    LR12->b[i].s64 = LR12->b[i].d * 68719476735; // convert to Q36
  }
}

void recalcFilterCoefficients_LR24(struct sLR24 *LR24) {
  double wc = 2.0 * PI * LR24->fc;
  double wc2 = wc * wc;
  double wc3 = wc2 * wc;
  double wc4 = wc2 * wc2;
  double k = wc / tan(PI * (LR24->fc / AUDIO_SAMPLERATE));
  double k2 = k * k;
  double k3 = k2 * k;
  double k4 = k2 * k2;
  double sq_tmp1 = sqrt(2.0) * wc3 * k;
  double sq_tmp2 = sqrt(2.0) * wc * k3;
  double norm = 4.0 * wc2 * k2 + 2.0 * sq_tmp1 + k4 + 2.0 * sq_tmp2 + wc4;

  if (LR24->isHighpass) {
    // coefficients for HighPass-Filter
    LR24->a[0].d = k4 / norm;
    LR24->a[1].d = -4.0 * k4 / norm;
    LR24->a[2].d = 6.0 * k4 / norm;
    LR24->a[3].d = LR24->a[1].d;
    LR24->a[4].d = LR24->a[0].d;
  }else{
    // coefficients for LowPass-Filter
    LR24->a[0].d = wc4 / norm;
    LR24->a[1].d = 4.0 * wc4 / norm;
    LR24->a[2].d = 6.0 * wc4 / norm;
    LR24->a[3].d = LR24->a[1].d;
    LR24->a[4].d = LR24->a[0].d;
  }

  LR24->b[0].d = 0; // we are not using this coefficient but keep it to not confuse with indices
  LR24->b[1].d = (4.0 * (wc4 + sq_tmp1 - k4 - sq_tmp2)) / norm;
  LR24->b[2].d = (6.0 * wc4 - 8.0 * wc2 * k2 + 6.0 * k4) / norm;
  LR24->b[3].d = (4.0 * (wc4 - sq_tmp1 + sq_tmp2 - k4)) / norm;
  LR24->b[4].d = (k4 - 2.0 * sq_tmp1 + wc4 - 2.0 * sq_tmp2 + 4.0 * wc2 * k2) / norm;

  // convert to Q44-format
  for (int i=0; i<5; i++) {
    LR24->a[i].s64 = LR24->a[i].d * 17592186044415; // convert to Q44
    LR24->b[i].s64 = LR24->b[i].d * 17592186044415; // convert to Q44
  }
}

void recalcNoiseGate(struct sNoisegate *Noisegate) {
  Noisegate->value_threshold.u32 = pow(2, (Noisegate->audio_bitwidth-1) + (Noisegate->threshold/6.0f)) - 1; // only bitwidth-1 can be used for samples, as MSB is for sign

  // range of 60dB means that we will reduce the signal on active gate by 60dB. We have to convert logarithmic dB-value into linear value for gain
  float value_gainmin_fs = pow(2, Noisegate->gainmin_bitwidth)-1; // maximum allowed value within FPGA
  Noisegate->value_gainmin.u16 = saturate_f(value_gainmin_fs/(pow(10, Noisegate->range/20)), 0, value_gainmin_fs);

  Noisegate->value_coeff_attack.s16 = round(exp(-2197.22457734f/(AUDIO_SAMPLERATE * Noisegate->attackTime_ms)) * 32767); // convert to Q15

  Noisegate->value_hold_ticks.u16 = Noisegate->holdTime_ms * (AUDIO_SAMPLERATE / 1000.0f);

  Noisegate->value_coeff_release.s16 = round(exp(-2197.22457734f/(AUDIO_SAMPLERATE * Noisegate->releaseTime_ms)) * 32767); // convert to Q15
}

void recalcCompressor(struct sCompressor *Compressor) {
  Compressor->value_threshold.u32 = pow(2, (Compressor->audio_bitwidth-1) + (Compressor->threshold/6.0f)) - 1; // only bitwidth-1 can be used for samples, as MSB is for sign

  // convert ratio (0=oo:1, 1=1:1, 2=2:1, 4=4:1, 8=8:1, 16=16:1, 32=32:1, 64=64:1) to bitshift 1:1=0bit, 2:1=1bit, 4:1=2bit, 8:1=3bit, 16:1=4bit, 32:1=5bit, 64:1=6bit, oo:1=24bit
  if (Compressor->ratio > 0) {
    Compressor->value_ratio.u16 = log(Compressor->ratio) / log(2); // convert real ratio-values into bit-shift-values
  }else{
    // ratio == 0 -> limiter = oo:1
    Compressor->value_ratio.u16 = Compressor->audio_bitwidth; // move 24 bits to the right ^= 1/oo ^= 0
  }

  Compressor->value_makeup.u16 = round(Compressor->makeup/6.0f); // we are allowing only 6dB-steps, so we have to round to optimize the user-input

  Compressor->value_coeff_attack.s16 = round(exp(-2197.22457734f/(AUDIO_SAMPLERATE * Compressor->attackTime_ms)) * 32767); // convert to Q15

  Compressor->value_hold_ticks.u16 = Compressor->holdTime_ms * (AUDIO_SAMPLERATE / 1000.0f);

  Compressor->value_coeff_release.s16 = round(exp(-2197.22457734f/(AUDIO_SAMPLERATE * Compressor->releaseTime_ms)) * 32767); // convert to Q15
}

void setADCgain(uint8_t channel, uint8_t gain) {
  audiomixer.adcGain[channel] = gain;

  data_16b fpga_data;
  fpga_data.u16 = round(audiomixer.adcGain[channel]/6.0f); // we are allowing only 6dB-steps, so we have to round to optimize the user-input
  sendDataToFPGA(FPGA_IDX_ADC_GAIN+channel, &fpga_data);
}

void resetAudioFilters() {
  for (int i=0; i<MAX_EQUALIZERS; i++) {
    audiomixer.peq[i].type = 1; // set all EQs to peak
    audiomixer.peq[i].fc = ( ((10000-125)/(MAX_EQUALIZERS-1)) * i) + 125; // set frequencies between 125Hz and 10kHz automatically
    audiomixer.peq[i].Q = 2;
    audiomixer.peq[i].gain = 0; // +/- 0dB
  }

  audiomixer.LR24_LP_Sub.fc = 125;
  audiomixer.LR24_LP_Sub.isHighpass = false;
  audiomixer.LR24_HP_LR.fc = 15;
  audiomixer.LR24_HP_LR.isHighpass = true;
}

void sendFiltersToFPGA() {
  // recalculate all filter coefficients
  for (int i=0; i<MAX_EQUALIZERS; i++) {
    recalcFilterCoefficients_PEQ(&audiomixer.peq[i]);
  }

  recalcFilterCoefficients_LR24(&audiomixer.LR24_LP_Sub);
  recalcFilterCoefficients_LR24(&audiomixer.LR24_HP_LR);

  // transmit the calculated coefficients to FPGA
  setResetFlags(1, -1, 1, -1); // [eqs, compressor, crossover, upsampler]
  setBypassFlags(audiomixer.peq[0].gain==0, audiomixer.peq[1].gain==0, audiomixer.peq[2].gain==0, audiomixer.peq[3].gain==0, audiomixer.peq[4].gain==0, -1, audiomixer.LR24_LP_Sub.fc>=19000, audiomixer.LR24_HP_LR.fc<=19); // [eq1, eq2, eq3, eq4, eq5, flag6, crossoverSub, crossoverLR]

  // Linkwitz-Riley HighPass
  for (int i=0; i<5; i++) { sendDataToFPGA(FPGA_IDX_XOVER+i, &audiomixer.LR24_HP_LR.a[i]); }
  for (int i=0; i<4; i++) { sendDataToFPGA(FPGA_IDX_XOVER+5+i, &audiomixer.LR24_HP_LR.b[i+1]); }; // we are not using b0
  // Linkwitz-Riley LowPass
  for (int i=0; i<5; i++) { sendDataToFPGA(FPGA_IDX_XOVER+9+i, &audiomixer.LR24_LP_Sub.a[i]); }
  for (int i=0; i<4; i++) { sendDataToFPGA(FPGA_IDX_XOVER+14+i, &audiomixer.LR24_LP_Sub.b[i+1]); } // we are not using b0

  // PEQ
  for (int peq=0; peq<MAX_EQUALIZERS; peq++) {
    for (int i=0; i<3; i++) { sendDataToFPGA(FPGA_IDX_PEQ+(peq*5)+i, &audiomixer.peq[peq].a[i]); } // 3 a-coefficients
    for (int i=0; i<2; i++) { sendDataToFPGA(FPGA_IDX_PEQ+3+(peq*5)+i, &audiomixer.peq[peq].b[i+1]); }// only 2 b-coefficients
  }
  delay(10);
  setResetFlags(0, -1, 0, -1); // [eqs, compressor, crossover, upsampler]
}

void setResetFlags(int8_t eqs, int8_t compressor, int8_t crossover, int8_t upsampler) {
  if (eqs == 1) {
    bitSet(audiomixer.resetFlags, 0);
  }else if (eqs == 0) {
    bitClear(audiomixer.resetFlags, 0);
  }
  
  if (compressor == 1) {
    bitSet(audiomixer.resetFlags, 1);
  }else if (compressor == 0) {
    bitClear(audiomixer.resetFlags, 1);
  }
  
  if (crossover == 1) {
    bitSet(audiomixer.resetFlags, 2);
  }else if (crossover == 0) {
    bitClear(audiomixer.resetFlags, 2);
  }
  
  if (upsampler == 1) {
    bitSet(audiomixer.resetFlags, 3);
  }else if (upsampler == 0) {
    bitClear(audiomixer.resetFlags, 3);
  }

  data_16b fpga_data;
  fpga_data.u16 = audiomixer.resetFlags;
  sendDataToFPGA(FPGA_IDX_AUX_CMD, &fpga_data);
}

void setBypassFlags(int8_t eq1, int8_t eq2, int8_t eq3, int8_t eq4, int8_t eq5, int8_t flag6, int8_t crossoverSub, int8_t crossoverLR) {
  if (eq1 == 1) {
    bitSet(audiomixer.bypassFlags, 0);
  }else if (eq1 == 0) {
    bitClear(audiomixer.bypassFlags, 0);
  }
  
  if (eq2 == 1) {
    bitSet(audiomixer.bypassFlags, 1);
  }else if (eq2 == 0) {
    bitClear(audiomixer.bypassFlags, 1);
  }
  
  if (eq3 == 1) {
    bitSet(audiomixer.bypassFlags, 2);
  }else if (eq3 == 0) {
    bitClear(audiomixer.bypassFlags, 2);
  }
  
  if (eq4 == 1) {
    bitSet(audiomixer.bypassFlags, 3);
  }else if (eq4 == 0) {
    bitClear(audiomixer.bypassFlags, 3);
  }
  
  if (eq5 == 1) {
    bitSet(audiomixer.bypassFlags, 4);
  }else if (eq5 == 0) {
    bitClear(audiomixer.bypassFlags, 4);
  }

  if (flag6 == 1) {
    bitSet(audiomixer.bypassFlags, 5);
  }else if (flag6 == 0) {
    bitClear(audiomixer.bypassFlags, 5);
  }

  if (crossoverSub == 1) {
    bitSet(audiomixer.bypassFlags, 6);
  }else if (crossoverSub == 0) {
    bitClear(audiomixer.bypassFlags, 6);
  }

  if (crossoverLR == 1) {
    bitSet(audiomixer.bypassFlags, 7);
  }else if (crossoverLR == 0) {
    bitClear(audiomixer.bypassFlags, 7);
  }

  data_16b fpga_data;
  fpga_data.u16 = audiomixer.bypassFlags;
  sendDataToFPGA(FPGA_IDX_AUX_CMD+1, &fpga_data);
}

void resetDynamics() {
  for (int gate=0; gate<MAX_NOISEGATES; gate++) {
    audiomixer.gates[gate].threshold = -80;
    audiomixer.gates[gate].range = 48;
    audiomixer.gates[gate].attackTime_ms = 10;
    audiomixer.gates[gate].holdTime_ms = 50;
    audiomixer.gates[gate].releaseTime_ms = 258;
  }

  for (int comp=0; comp<MAX_COMPRESSORS; comp++) {
    audiomixer.compressors[comp].threshold = 0;
    audiomixer.compressors[comp].ratio = 1;
    audiomixer.compressors[comp].makeup = 0;
    audiomixer.compressors[comp].attackTime_ms = 10;
    audiomixer.compressors[comp].holdTime_ms = 10;
    audiomixer.compressors[comp].releaseTime_ms = 151;
  }
}

void sendDynamicsToFPGA() {
  for (int gate=0; gate<MAX_NOISEGATES; gate++) {
    // recalculate values for specific noise-gate
    recalcNoiseGate(&audiomixer.gates[gate]);

    // transmit the calculated data to FPGA
    sendDataToFPGA(FPGA_IDX_GATE+0+(gate*5), &audiomixer.gates[gate].value_threshold);
    sendDataToFPGA(FPGA_IDX_GATE+1+(gate*5), &audiomixer.gates[gate].value_gainmin);
    sendDataToFPGA(FPGA_IDX_GATE+2+(gate*5), &audiomixer.gates[gate].value_coeff_attack);
    sendDataToFPGA(FPGA_IDX_GATE+3+(gate*5), &audiomixer.gates[gate].value_hold_ticks);
    sendDataToFPGA(FPGA_IDX_GATE+4+(gate*5), &audiomixer.gates[gate].value_coeff_release);
  }

  for (int comp=0; comp<MAX_COMPRESSORS; comp++) {
    // recalculate values for specific compressor
    recalcCompressor(&audiomixer.compressors[comp]);

    // transmit the calculated data to FPGA
    sendDataToFPGA(FPGA_IDX_COMP+0+(comp*6), &audiomixer.compressors[comp].value_threshold);
    sendDataToFPGA(FPGA_IDX_COMP+1+(comp*6), &audiomixer.compressors[comp].value_ratio);
    sendDataToFPGA(FPGA_IDX_COMP+2+(comp*6), &audiomixer.compressors[comp].value_makeup);
    sendDataToFPGA(FPGA_IDX_COMP+3+(comp*6), &audiomixer.compressors[comp].value_coeff_attack);
    sendDataToFPGA(FPGA_IDX_COMP+4+(comp*6), &audiomixer.compressors[comp].value_hold_ticks);
    sendDataToFPGA(FPGA_IDX_COMP+5+(comp*6), &audiomixer.compressors[comp].value_coeff_release);
  }
}
