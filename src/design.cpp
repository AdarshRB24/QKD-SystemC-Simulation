#include <systemc.h>
using namespace sc_core;

typedef sc_bv<66> key66_t;
typedef sc_bv<16> block_t;
typedef sc_bv<64> keytype;
typedef sc_bv<32> key32_t;
typedef sc_bv<64> row_t;

inline int b(sc_logic x) {
    return x.to_bool();
}

SC_MODULE(Sifting) {
    sc_in<keytype> alice_key, bob_key;
    sc_in<keytype> alice_basis, bob_basis;
    sc_out<keytype> sifted_alice_key, sifted_bob_key, Mask;

    void process() {
        keytype resultA, resultB, mask;

        for (int i = 0; i < 64; i++) {
            if (alice_basis.read()[i] == bob_basis.read()[i]) {
                resultA[i] = alice_key.read()[i];
                resultB[i] = bob_key.read()[i];
                mask[i] = 1;
            } else {
                resultA[i] = 0;
                resultB[i] = 0;
                mask[i] = 0;
            }
        }

        sifted_alice_key.write(resultA);
        sifted_bob_key.write(resultB);
        Mask.write(mask);
    }

    SC_CTOR(Sifting) {
        SC_METHOD(process);
        sensitive << alice_key << bob_key << alice_basis << bob_basis;
    }
};

SC_MODULE(ErrorEstimation) {
    sc_in<keytype> sifted_key_alice, sifted_key_bob, Mask;
    sc_out<double> qber;

    void compute() {
        int errors = 0;
        int total = 0;

        for (int i = 0; i < 64; i++) {
            if (Mask.read()[i] == 1) {
                total++;
                if (sifted_key_alice.read()[i] != sifted_key_bob.read()[i])
                    errors++;
            }
        }

        qber.write((double)errors / total);  
    }

    SC_CTOR(ErrorEstimation) {
        SC_METHOD(compute);
        sensitive << sifted_key_alice << sifted_key_bob << Mask;
    }
};


SC_MODULE(HammingCorrection) {

    sc_in<keytype> alice_sifted_key;
    sc_in<keytype> bob_sifted_key;

    sc_out<keytype> corrected_key;
  	

	
   
    block_t build_block(sc_bv<11> data, int p1, int p2, int p3, int p4, int p0) {

        block_t block;

        block[0] = p0;
        block[1] = p1;
        block[2] = p2;
        block[4] = p3;
        block[8] = p4;

        int d = 0;
        for (int i = 0; i < 16; i++) {
            if (i != 0 && i != 1 && i != 2 && i != 4 && i != 8) {
                block[i] = data[d++];
            }
        }

        return block;
    }

   
    void compute_parity(sc_bv<11> data, int &p1, int &p2, int &p3, int &p4, int &p0) {

    
        block_t temp;
        int d = 0;

        for (int i = 0; i < 16; i++) {
            if (i != 0 && i != 1 && i != 2 && i != 4 && i != 8) {
                temp[i] = data[d++];
            } else {
                temp[i] = 0;
            }
        }

        
        p1 = b(temp[1]^temp[3])^b(temp[5]^temp[7])^b(temp[9]^temp[11])^b(temp[13]^temp[15]);
        p2 = b(temp[2]^temp[3])^b(temp[6]^temp[7])^b(temp[10]^temp[11])^b(temp[14]^temp[15]);
        p3 = b(temp[4]^temp[5])^b(temp[6]^temp[7])^b(temp[12]^temp[13])^b(temp[14]^temp[15]);
        p4 = b(temp[8]^temp[9])^b(temp[10]^temp[11])^b(temp[12]^temp[13])^b(temp[14]^temp[15]);

        
        p0 = 0;
        for (int i = 1; i < 16; i++)
            p0 ^= b(temp[i]);

        p0 ^= p1 ^ p2 ^ p3 ^ p4;
    }

    
    void correct_block(block_t &block) {

        int s1 = b(block[1]^block[3])^b(block[5]^block[7])^b(block[9]^block[11])^b(block[13]^block[15]);
        int s2 = b(block[2]^block[3])^b(block[6]^block[7])^b(block[10]^block[11])^b(block[14]^block[15]);
        int s3 = b(block[4]^block[5])^b(block[6]^block[7])^b(block[12]^block[13])^b(block[14]^block[15]);
        int s4 = b(block[8]^block[9])^b(block[10]^block[11])^b(block[12]^block[13])^b(block[14]^block[15]);

        int error_pos = s1 + (s2<<1) + (s3<<2) + (s4<<3);

        if (error_pos != 0) {
            block[error_pos] = !block[error_pos];
        }
    }

    void process() {

        key66_t alice = 0;
        key66_t bob   = 0;
		
      
      	for (int i=0;i<64;i++){
    		alice[i]=alice_sifted_key.read()[i];
      		bob[i]=bob_sifted_key.read()[i];
 		}
      
        key66_t result=0;

        for (int g = 0; g < 6; g++) {

            sc_bv<11> a_data, b_data;

            for (int i = 0; i < 11; i++) {
                a_data[i] = alice[g*11 + i];
                b_data[i] = bob[g*11 + i];
            }

            
            int p1,p2,p3,p4,p0;
            compute_parity(a_data, p1,p2,p3,p4,p0);
            
            block_t block = build_block(b_data, p1,p2,p3,p4,p0);

            correct_block(block);

            int d = 0;
            for (int i = 0; i < 16; i++) {
                if (i != 0 && i != 1 && i != 2 && i != 4 && i != 8) {
                    result[g*11 + d++] = block[i];
                }
            }
        }

        corrected_key.write(result.range(63,0));
    }

    SC_CTOR(HammingCorrection) {
        SC_METHOD(process);
        sensitive << alice_sifted_key << bob_sifted_key;
    }
};



SC_MODULE(PrivacyAmplification) {

    sc_in<keytype> alice_key_in;
    sc_in<keytype> bob_key_in;

    sc_in<row_t> M[32];

    sc_out<key32_t> alice_final_key;
    sc_out<key32_t> bob_final_key;

    void process() {

        keytype alice = alice_key_in.read();
        keytype bob   = bob_key_in.read();

        key32_t out_alice=0;
        key32_t out_bob=0;

        for (int i = 0; i < 32; i++) {

            bool sum_a = 0;
            bool sum_b = 0;

            row_t row = M[i].read();

            for (int j = 0; j < 64; j++) {

                if (row[j] == '1') {
                    sum_a ^= (alice[j] == '1');
                    sum_b ^= (bob[j] == '1');
                }
            }

            out_alice[i] = sum_a;
            out_bob[i]   = sum_b;
        }

        alice_final_key.write(out_alice);
        bob_final_key.write(out_bob);
    }

    SC_CTOR(PrivacyAmplification) {
        SC_METHOD(process);

        sensitive << alice_key_in << bob_key_in;

        for (int i = 0; i < 32; i++)
            sensitive << M[i];
    }
};


SC_MODULE(Top) {

    sc_out<keytype> sifteda_key_sig;
    sc_out<keytype> siftedb_key_sig;
    sc_out<keytype> mask_sig;
	  sc_out<double> qber_sig;
  	sc_out<keytype> hamming_corrected_key_sig;
  	sc_out<double> qber_post_hamming_sig;
  	sc_out<key32_t> alice_final_key_sig;
  	sc_out<key32_t> bob_final_key_sig;

    sc_in<keytype> alice_key_sig, bob_key_sig;
    sc_in<keytype> alice_basis_sig, bob_basis_sig;
  	sc_in<row_t>privacy_matrix[32];


    Sifting sift;
    ErrorEstimation pre_err;
  	HammingCorrection ham;
  	ErrorEstimation post_err;
  	PrivacyAmplification pri_amp;

    SC_CTOR(Top) : sift("Sifting"), pre_err("ErrorEstimation"), ham("HammingCorrection"), post_err("ErrorEstimation"), pri_amp("PrivacyAmplification"){

        sift.alice_key(alice_key_sig);
        sift.bob_key(bob_key_sig);
        sift.alice_basis(alice_basis_sig);
        sift.bob_basis(bob_basis_sig);

        sift.sifted_alice_key(sifteda_key_sig);
        sift.sifted_bob_key(siftedb_key_sig);
        sift.Mask(mask_sig);


        pre_err.sifted_key_alice(sifteda_key_sig);
        pre_err.sifted_key_bob(siftedb_key_sig);
        pre_err.Mask(mask_sig);
      
        pre_err.qber(qber_sig);
      
      
      	ham.alice_sifted_key(sifteda_key_sig);
      	ham.bob_sifted_key(siftedb_key_sig);
      
      	ham.corrected_key(hamming_corrected_key_sig);
      
      
      	post_err.sifted_key_alice(sifteda_key_sig);
        post_err.sifted_key_bob(hamming_corrected_key_sig);
        post_err.Mask(mask_sig);
      
        post_err.qber(qber_post_hamming_sig);
      
      
      
      	pri_amp.alice_key_in(sifteda_key_sig);
      	pri_amp.bob_key_in(hamming_corrected_key_sig);
      	for (int i = 0; i < 32; i++) {
    		pri_amp.M[i](privacy_matrix[i]);
		}
      	
      	pri_amp.alice_final_key(alice_final_key_sig);
      	pri_amp.bob_final_key(bob_final_key_sig);
    }
};
