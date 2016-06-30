/********************
Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
creative commons license 3.0: Attribution-ShareAlike CC BY-SA
This software is furnished "as is", without technical support, and with no 
warranty, express or implied, as to its usefulness for any purpose.

Thread Safe: No
Extendable: Yes

modified 01.07.16 by TomaTLAB based on: the Dr. John Liu "phi_interfaces"

quadrature encoder driver (w/o PCINT)
quadrature encoder stream (fake, not using buffers)
*/

class quadEncoder {
public:
  volatile int stat_seq_ptr=4; // Center the status of the encoder
  volatile int pos;
  int pinA,pinB;
  quadEncoder(int a,int b):pinA(a),pinB(b) {}
  void begin() {
    pinMode(pinA, INPUT);
    digitalWrite(pinA,1);
    pinMode(pinB, INPUT);
    digitalWrite(pinB,1);
  }
};

//emulate a stream based on quadEncoder movement returning +/- for every 'sensivity' steps
//buffer not needer because we have an accumulator
class quadEncoderStream:public Stream {
public:
  quadEncoder &enc;//associated hardware quadEncoder
  int sensivity;
  int oldPos;
  quadEncoderStream(quadEncoder &enc,int sensivity):enc(enc), sensivity(sensivity) {}
  inline void setSensivity(int s) {sensivity=s;}

  int available(void) {
	  static const byte stat_seq[]={3,2,0,1,3,2,0,1,3}; // For always on switches use {0,1,3,2,0,1,3,2,0};
//	  static const byte stat_seq[]={0,1,3,2,0,1,3,2,0}; //For the sake of simple coding, please don't mix always-on encoders with always-off encoders.
	  byte stat_int=(digitalRead(enc.pinB)<<1) | digitalRead(enc.pinA);
	  if (stat_int==stat_seq[enc.stat_seq_ptr+1])
	  {
	    enc.stat_seq_ptr++;
	      enc.pos++;
	    if (enc.stat_seq_ptr==8)
	    {
	      enc.stat_seq_ptr=4;
//	      enc.pos++;
	    }
	  }
	  else if (stat_int==stat_seq[enc.stat_seq_ptr-1])
	  {
	    enc.stat_seq_ptr--;
	      enc.pos--;
	    if (enc.stat_seq_ptr==0)
	    {
	      enc.stat_seq_ptr=4;
//	      enc.pos--;
	    }
	  }

	return abs(enc.pos-oldPos)/sensivity;
	
	}

  int peek(void) {
    int d=enc.pos-oldPos;
    if (d<=-sensivity)return '-';
    if (d>=sensivity) return '+';
    return -1;
  }
  int read() {
    int d=enc.pos-oldPos;
    if (d<=-sensivity) {
      oldPos-=sensivity;
      return '-';
    }
    if (d>=sensivity) {
      oldPos+=sensivity;
      return '+';
    }
    return -1;
  }
  void flush() {oldPos=enc.pos;}
  size_t write(uint8_t v) {oldPos=v;return 1;}
};


