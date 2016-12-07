#include "encrypt.h"
#include <boost/log/trivial.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>

using namespace CryptoPP;
using namespace std;

void encrypt_file_CBC_AES(SecByteBlock key, SecByteBlock iv, const char *infile,
                          const char* outfile ) {
    CBC_Mode<AES>::Encryption aes(key, key.size(), iv);
    FileSource(infile, true, new StreamTransformationFilter(aes, new FileSink(outfile)));
}

void decrypt_file_CBC_AES(SecByteBlock key, SecByteBlock iv, const char *infile,
                          const char* outfile ) {
    CBC_Mode<AES>::Decryption aes(key, key.size(), iv);
    FileSource(infile, true, new StreamTransformationFilter(aes, new FileSink(outfile)));
}

void encrypt_file_GCM_AES(SecByteBlock key, SecByteBlock iv, const char *infile,
                          const char* outfile ) {
    GCM<AES>::Encryption enc;
    enc.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
    FileSource(infile, true, new AuthenticatedEncryptionFilter(enc, new FileSink(outfile)));
}

void decrypt_file_GCM_AES(SecByteBlock key, SecByteBlock iv, const char *infile,
                          const char* outfile ) {
    GCM<AES>::Decryption dec;
    dec.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
    FileSource(infile, true, new AuthenticatedEncryptionFilter(dec, new FileSink(outfile)));
}

// encrypt with RSA public key
void encrypt_file_RSA(std::string &plain, std::string &file,
                      RandomNumberGenerator &rng,
                      RSA::PublicKey &pk) {
    RSAES<OAEP<SHA>>::Encryptor enc(pk);
    StringSource ss1(plain, true,
                     new PK_EncryptorFilter(rng, enc,
                                            new FileSink(file.c_str())
                         ) 
        );
}

// decrypt with RSA
void decrypt_file_RSA(std::string &recovered, std::string &file,
                      RandomNumberGenerator &rng,
                      RSA::PrivateKey &sk) {

    RSAES<OAEP<SHA>>::Decryptor dec(sk);
    FileSource ss2(file.c_str(), true,
                   new PK_DecryptorFilter(rng, dec,
                                          new StringSink(recovered)
                       ) 
        ); 
}

void load_private_key(const std::string &file_name, RSA::PrivateKey &sk) {
    FileSource fs1(file_name.c_str(), true);
    PEM_Load(fs1, sk);

    AutoSeededRandomPool prng;
    bool valid = sk.Validate(prng, 3);
    if(!valid) {
        cerr << "RSA private key is not valid" << endl; 
    } else {
        BOOST_LOG_TRIVIAL(trace) << "RSA public key is valid";
    }
    cout << "N:" << sk.GetModulus() << endl;
    cout << "E:" << sk.GetPublicExponent() << endl;
}

void load_public_key(const string &file_name, CryptoPP::RSA::PublicKey &pk) {
    FileSource fs1(file_name.c_str(), true);
    PEM_Load(fs1, pk);

    AutoSeededRandomPool prng;
    bool valid = pk.Validate(prng, 3);
    if(!valid) {
        cerr << "RSA public key is not valid" << endl; 
    } else {
        BOOST_LOG_TRIVIAL(trace) << "RSA public key is valid";
    }
    cout << "N:" << pk.GetModulus() << endl;
    cout << "E:" << pk.GetPublicExponent() << endl;
}
