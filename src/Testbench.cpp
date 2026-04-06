#include <systemc.h>
#include <cstdlib>
#include <ctime>

typedef sc_bv<64> keytype;
typedef sc_bv<32> key32_t;
typedef sc_bv<64> row_t;

#include "design.cpp"

SC_MODULE(Testbench) {

    sc_signal<keytype> alice_key, bob_key;
    sc_signal<keytype> alice_basis, bob_basis;
    sc_signal<row_t> privacy_matrix[32];

    sc_signal<keytype> sifted_alice, sifted_bob, mask;
    sc_signal<double> qber_pre;
    sc_signal<keytype> corrected_key;
    sc_signal<double> qber_post;
    sc_signal<key32_t> alice_final, bob_final;

    Top top;

    SC_CTOR(Testbench) : top("top") {

        top.alice_key_sig(alice_key);
        top.bob_key_sig(bob_key);
        top.alice_basis_sig(alice_basis);
        top.bob_basis_sig(bob_basis);

        for (int i = 0; i < 32; i++) {
            top.privacy_matrix[i](privacy_matrix[i]);
        }

        top.sifteda_key_sig(sifted_alice);
        top.siftedb_key_sig(sifted_bob);
        top.mask_sig(mask);
        top.qber_sig(qber_pre);

        top.hamming_corrected_key_sig(corrected_key);
        top.qber_post_hamming_sig(qber_post);

        top.alice_final_key_sig(alice_final);
        top.bob_final_key_sig(bob_final);

        SC_THREAD(run);
    }

    int rand_bit() {
        return rand() % 2;
    }

    void run() {

        srand(time(NULL));

        keytype a_key, b_key, a_basis, b_basis;

        for (int i = 0; i < 64; i++) {

            int bit = rand_bit();
            a_key[i] = bit;

            if ((rand() % 50) == 0)
                b_key[i] = !bit;
            else
                b_key[i] = bit;
        }

        for (int i = 0; i < 64; i++) {
            a_basis[i] = rand_bit();
            b_basis[i] = rand_bit();
        }

        for (int i = 0; i < 32; i++) {
            row_t row;
            for (int j = 0; j < 64; j++) {
                row[j] = rand_bit();
            }
            privacy_matrix[i].write(row);
        }

        alice_key.write(a_key);
        bob_key.write(b_key);
        alice_basis.write(a_basis);
        bob_basis.write(b_basis);

        wait(1, SC_NS);

        cout << "\n========= INPUT =========\n";
        cout << "Alice Key   : " << alice_key.read() << endl;
        cout << "Bob Key     : " << bob_key.read() << endl;
        cout << "Alice Basis : " << alice_basis.read() << endl;
        cout << "Bob Basis   : " << bob_basis.read() << endl;

        cout << "\n========= SIFTING =========\n";
        cout << "Sifted Alice: " << sifted_alice.read() << endl;
        cout << "Sifted Bob  : " << sifted_bob.read() << endl;
        cout << "Mask        : " << mask.read() << endl;

        cout << "\n========= PRE-QBER =========\n";
        cout << "QBER (pre)  : " << qber_pre.read() << endl;

        cout << "\n========= HAMMING =========\n";
        cout << "Corrected Key: " << corrected_key.read() << endl;

        cout << "\n========= POST-QBER =========\n";
        cout << "QBER (post) : " << qber_post.read() << endl;

        cout << "\n========= FINAL KEY =========\n";
        cout << "Alice Final : " << alice_final.read() << endl;
        cout << "Bob Final   : " << bob_final.read() << endl;

        cout << "\n====================================\n";

        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {

    Testbench tb("tb");
    sc_start();

    return 0;
}
