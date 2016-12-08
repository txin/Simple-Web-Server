#include "encrypt.h"

#include <boost/log/trivial.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>

using namespace CryptoPP;
using namespace std;

typedef unsigned char byte;

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
    CryptoPP::GCM<AES>::Decryption dec;
    dec.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
    FileSource(infile, true, new AuthenticatedEncryptionFilter(dec, new FileSink(outfile)));
}

// encrypt with RSA public key
void encrypt_string_RSA(const std::string &plain, const std::string &file,
                        RandomNumberGenerator &rng,
                        const RSA::PublicKey &pk) {
    RSAES<OAEP<SHA>>::Encryptor enc(pk);
    StringSource ss1(plain, true,
                     new PK_EncryptorFilter(rng, enc,
                                            new FileSink(file.c_str())
                         ) 
        );
}

// decrypt with RSA secret key
void decrypt_string_RSA(std::string &recovered, const std::string &file,
                        RandomNumberGenerator &rng,
                        RSA::PrivateKey &sk) {

    RSAES<OAEP<SHA>>::Decryptor dec(sk);
    FileSource ss2(file.c_str(), true,
                   new PK_DecryptorFilter(rng, dec,
                                          new StringSink(recovered)
                       ) 
        ); 
}

void load_private_key(const std::string &file_name, CryptoPP::RSA::PrivateKey &sk) {
    FileSource fs1(file_name.c_str(), true);
    PEM_Load(fs1, sk);

    AutoSeededRandomPool prng;
    bool valid = sk.Validate(prng, 3);
    if(!valid) {
        cerr << "Server: RSA private key is not valid" << endl; 
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Server: RSA private key is valid";
    }
}

void load_public_key(const string &file_name, CryptoPP::RSA::PublicKey &pk) {
    FileSource fs1(file_name.c_str(), true);
    PEM_Load(fs1, pk);

    AutoSeededRandomPool prng;
    bool valid = pk.Validate(prng, 3);
    if(!valid) {
        cerr << "Server: RSA public key is not valid." << endl; 
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Server: RSA public key is valid.";
    }
}

// Dump readable key
void dump_key(SecByteBlock& ekey, string &key_str) {
    // Print them
    HexEncoder encoder(new StringSink(key_str));
    cout << "AES key: ";
    encoder.Put(ekey.data(), ekey.size());
    encoder.MessageEnd(); cout << endl;
}


// encrypt key and store IV as normal binary file.
void generate_key_store(const string &in_file, SecByteBlock &key, SecByteBlock &iv,
                        const RSA::PublicKey &pk) {
    AutoSeededRandomPool rnd;

    rnd.GenerateBlock(key, key.size());
    rnd.GenerateBlock(iv, iv.size());

    // encrypt the secret key with RSA
    const std::string key_out_file = in_file + ".key";
    std::string key_str = std::string((const char*)key.data(), key.size());
    const std::string iv_file = in_file + ".iv";

    std::string iv_str = std::string((const char*)iv.data(), iv.size());
    StringSource ss(iv_str, true /* pump all */, new FileSink(iv_file.c_str(), true));
    BOOST_LOG_TRIVIAL(trace) << "Server: finished writing iv: " << iv_file;
    encrypt_string_RSA(key_str, key_out_file, rnd, pk);
    BOOST_LOG_TRIVIAL(trace) << "Server: finished encrypting key: " << key_out_file;
}

// Confidentiality
void encrypt_file_1(const string &in_file, const RSA::PublicKey &pk) {
    SecByteBlock key(0x00, AES::DEFAULT_KEYLENGTH);
    SecByteBlock iv(0x00, AES::BLOCKSIZE);
    generate_key_store(in_file, key, iv, pk);
    const std::string out_file = in_file + ".bin";
    encrypt_file_CBC_AES(key, iv, in_file.c_str(), out_file.c_str());
    BOOST_LOG_TRIVIAL(trace) << "Server: finished encrypting file (CONFIDENTIALITY): "
                             << out_file;
}

// Integrity
void encrypt_file_2(const string &in_file, const RSA::PublicKey &pk) {
    SecByteBlock key(0x00, AES::DEFAULT_KEYLENGTH);
    SecByteBlock iv(0x00, AES::BLOCKSIZE);
    generate_key_store(in_file, key, iv, pk);
    const std::string out_file = in_file + ".bin";
    encrypt_file_GCM_AES(key, iv, in_file.c_str(), out_file.c_str());
    BOOST_LOG_TRIVIAL(trace) << "Server: finished encrypting file (INTEGRITY): " << out_file;
}

// Write security flag, plain


