/*********************** Disclaimer **********************************
  These modules carry no warranty or guarantee of any kind! They are used at
  your own risk, and I make no claims as to their suitability for a particular
  function. Prospective users must evaluate the system before using it, and no
  liability will be entertained by myself in any shape or form whatsoever.
  The modules and software have been produced at low cost for the benefit of
  the K-Swapped Insight community. The software is available free via the internet.
  Users may modify or adapt the system as they see fit. Be aware that vehicle
  modifications can lead to invalidated insurance and warranty issues. You the
  end user remain fully liable for any modifications made to your vehicle.


**************** Start User Configurable Settings **************************
The following variables can be changed to alter the behaviour of the IGuF device.
The variables will be separated into groups based on the gauge which they support.*/

//*****Speedometer Variables*****\\
  // Speedometer Mode (sm)
    boolean sm = 1;      // 0 = low frequency vsss, 1 = high frequency vss
  // Speedometer Multiplier (sx)
    float sx = 1.76;     // This variable is for drivetrain multiplier, FD, wheel/tire size, VSS type will all affect wheel speed calibration. This variable is in place to allow for easy fine tuning.

//*****Tachometer Variables*****\\
  // Tachometer Mode (tm)
    boolean tm = 1;          // 0 = traditional/accurate RPM, 1 = Redline Centric Mode
  // Redline RPM (rl)
    int rl = 7100;       // This variable is referenced when the tachometer mode is set to Redline Centric Mode. The IGuF will adjust the output of the tach so that the file bars will illuminate at this RPM

//*****ECT Variables*****\\
  // Bar Temps (bt3-13)
    int bt3 = 359;
    int bt5 = 259;
    int bt7 = 199;
    int bt9 = 129;
    int bt11 = 109;
    int bt13 = 49;
    
/**************** End User Configurable Settings **************************/
















// Define Pins
  // Input Pins
    const int eip =  A15;             // ECT input pin
    const int sip =  26;              // Speedo input pin
    const int tip =  28;              // Tach input pin
    const int i1p =  34;              // IMA input 1 digital pin
    //const int i1p =  A14;           // IMA input 1 analog pin
    const int i2p =  36;              // IMA input 2 digital pin
    //const int i2p =  A13;           // IMA input 2 analog pin
  // Output Pins
    const int eop =  22;              // the number of the Speedometer pin
    const int sop =  24;              // the number of the Speedometer pin
    const int top =  30;              // the number of the Speedometer pin
    const int met =  18;              // the serial pin driving comms to the LTC1487
    const int kap =  38;              // the number of the IMA keep alive signal output pin

// Define Variables
    // ECT Variables
      unsigned long eopT = 4425;      // the duration of the speedometer off pulse (microseconds)
      unsigned long eipV = 0;         // the duration of the high side of the input square wave
    // Speedometer Variables
      unsigned long sipTime = 0;      // the duration of the high side of the input square wave
      float dt = sx / 4.42857;
      float vx = sx / 0.125;
      int long sto = 30000;           // Timeout in ms for the speedomiter pulsein reading
    // Tachometer Variables
      unsigned long tipTime = 0;      // the duration of the high side of the input square wave
      float rx = 0.08680556;          // multiplier for accurate RPM
      float qx2 = ((1000000 / (rl / 30)) / 2);
      float qx = 180 / qx2;
    //storage variables
      boolean toggle0 = 0;            // toggle variable for ECT
      boolean toggle1 = 0;            // toggle variable for Speedometer
      boolean toggle2 = 0;            // toggle variable for Tach

void setup(){
  // Set pin types
    pinMode(eip, INPUT);
    pinMode(eop, OUTPUT);
    pinMode(sip, INPUT);
    pinMode(sop, OUTPUT);
    pinMode(tip, INPUT);
    pinMode(top, OUTPUT);

  // Set tach to appropriate mode
    if (tm) {
      rx = qx;
    } else {
      rx = rx;
    }
  // Set speedo to appropriate mode
    if (sm) {
      dt = dt;
    } else {
      dt = vx;
      sto = 100000;
    }
    
cli(); // stop interrupts
  
  // configure timer5 for ECT
    TCCR5A = 0;                           // set entire TCCR3A register to 0
    TCCR5B = 0;                           // same for TCCR3B
    TCNT5  = 0;                           //initialize counter value to 0
    OCR5A = 210;                          // set compare match register for 1hz increments
    TCCR5B |= (1 << WGM52);               // turn on CTC mode
    TCCR5B |= (1 << CS52) | (1 << CS50);  // Set CS12 bit for 1024 prescaler
    TIMSK5 |= (1 << OCIE5A);              // enable timer compare interrupt
    
  // configure timer4 for Speedometer
    TCCR3A = 0;                           // set entire TCCR4A register to 0
    TCCR3B = 0;                           // same for TCCR4B
    TCNT3  = 0;                           //initialize counter value to 0
    OCR3A = 22500;                        // set compare match register for 1hz increments
    TCCR3B |= (1 << WGM32);               // turn on CTC mode
    TCCR3B |= (1 << CS32);                // Set CS12 bit for 256 prescaler
    TIMSK3 |= (1 << OCIE3A);              // enable timer compare interrupt

  //  configure timer3 for Tachometer
    TCCR4A = 0;                           // set entire TCCR5A register to 0
    TCCR4B = 0;                           // same for TCCR5B
    TCNT4  = 0;                           //initialize counter value to 0
    OCR4A = 22500;                          // set compare match register for 1hz increments
    TCCR4B |= (1 << WGM42);               // turn on CTC mode
    TCCR4B |= (1 << CS42);                // Set CS12 bit for 256 prescaler
    TIMSK4 |= (1 << OCIE4A);              // enable timer compare interrupt
  
sei();//allow interrupts

}//end setup

// Timer 5 interrupt sequence for ECT
  ISR(TIMER5_COMPA_vect){
   //Toggle the eop pin at the desired frequency. 
    if (toggle0){
      digitalWrite(eop,HIGH);
      toggle0 = 0;
      OCR5A = 469; //30ms high time
    }
    else{
      digitalWrite(eop,LOW);
      toggle0 = 1;
      // Transform the input length to output length
      OCR5A = eopT;
    }
  }

// Timer 3 interrupt sequence for Speedometer
  ISR(TIMER3_COMPA_vect){
   // Transform the input length to output length
   if (sipTime > 0){
   OCR3A = sipTime / dt;
   }
   //Toggle the spd pin at the desired frequency. 
    if (toggle1){
      digitalWrite(sop,HIGH);
      toggle1 = 0;
    }
    else{
      digitalWrite(sop,LOW);
      toggle1 = 1;
    }
  }

// Timer 4 interrupt sequence for Tachometer
  ISR(TIMER4_COMPA_vect){
   // Transform the input length to output length
   if (tipTime > 0){
   OCR4A = tipTime * rx;
   }
   //Toggle the spd pin at the desired frequency. 
    if (toggle2){
      digitalWrite(top,HIGH);
      toggle2 = 0;
    }
    else{
      digitalWrite(top,LOW);
      toggle2 = 1;
    }
  }
  
void loop(){
  // Define variable for current micros
    unsigned long currentMicros = micros();
  // Capture input values
    eipV = analogRead(eip);                // ECT input pin voltage (0-1023)
    sipTime = pulseIn(sip, LOW, sto);    // Speedometer pulse time
    tipTime = pulseIn(tip, LOW, 40000);    // Tachometer pulse time

  // Logic to figure out appropriate OCR5A length.
    if (eipV <= 49){
      eopT = 2460;  //Value will display 13-14 bars
    } else if ((eipV <= 109) && (eipV > 49)){
        eopT = 3477; //Value to display 11-12 bars
      } else if ((eipV <=129) && (eipV > 109)){
        eopT = 4966; //Value to display 9-10 bars
      } else if ((eipV <= 199) && (eipV > 129)){
        eopT = 6954; //Value to display 7-8 bars
      } else if ((eipV <= 259) && (eipV > 199)){
        eopT = 11324; //Value to display 5-6 bars
      } else if ((eipV <= 359) && (eipV > 259)){
        eopT = 17388; //Value to display 3-4 bars
      } else { eopT = 38111;}
}
