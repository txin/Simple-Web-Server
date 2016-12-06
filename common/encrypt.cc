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
    std::ifstream ifs(file_name);

    std::string private_key((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());

    static string HEADER = "-----BEGIN RSA PRIVATE KEY-----";
    static string FOOTER = "-----END RSA PRIVATE KEY-----";
    
    size_t pos1, pos2;
    pos1 = private_key.find(HEADER);
    if(pos1 == string::npos)
        throw runtime_error("PEM header not found");
    
    pos2 = private_key.find(FOOTER, pos1+1);
    if(pos2 == string::npos)
        throw runtime_error("PEM footer not found");
    
// Start position and length
    pos1 = pos1 + HEADER.length();
    pos2 = pos2 - pos1;
    string keystr = private_key.substr(pos1, pos2);

// Base64 decode, place in a ByteQueue    
    ByteQueue queue;
    Base64Decoder decoder;

    decoder.Attach(new Redirector(queue));
    decoder.Put((const byte*)keystr.data(), keystr.length());
    decoder.MessageEnd();

// Write to file for inspection
    FileSink fs("decoded-sk.der");
    queue.CopyTo(fs);
    fs.MessageEnd();

    try
    {
//        CryptoPP::RSA::PrivateKey rsaPrivate;
        sk.BERDecodePrivateKey(queue, false /*paramsPresent*/, queue.MaxRetrievable());

        // BERDecodePrivateKey is a void function. Here's the only check
        // we have regarding the DER bytes consumed.
        assert(queue.IsEmpty());
    
        AutoSeededRandomPool prng;
        bool valid = sk.Validate(prng, 3);
        if(!valid)
            cerr << "RSA private key is not valid" << endl;
    
        cout << "N:" << sk.GetModulus() << endl;
        cout << "E:" << sk.GetPublicExponent() << endl;
        cout << "D:" << sk.GetPrivateExponent() << endl;
    
    }
    catch (const Exception& ex)
    {
        cerr << ex.what() << endl;
        exit (1);
    }
}

void load_public_key(const string &file_name, CryptoPP::RSA::PublicKey &pk) {
    std::ifstream ifs(file_name);
    std::string key_str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());

    static string HEADER = "-----BEGIN RSA PUBLIC KEY-----";
    static string FOOTER = "-----END RSA PUBLIC KEY-----";
    
    size_t pos1, pos2;
    pos1 = key_str.find(HEADER);
    if(pos1 == string::npos)
        throw runtime_error("PEM header not found");
    
    pos2 = key_str.find(FOOTER, pos1+1);
    if(pos2 == string::npos)
        throw runtime_error("PEM footer not found");
    
// Start position and length
    pos1 = pos1 + HEADER.length();
    pos2 = pos2 - pos1;
    string keystr = key_str.substr(pos1, pos2);

// Base64 decode, place in a ByteQueue    
    ByteQueue queue;
    Base64Decoder decoder;

    decoder.Attach(new Redirector(queue));
    decoder.Put((const byte*)keystr.data(), keystr.length());
    decoder.MessageEnd();

// Write to file for inspection
    FileSink fs("decoded-pk.der");
    queue.CopyTo(fs);
    fs.MessageEnd();

    try
    {
        pk.BERDecodePublicKey(queue, false /*paramsPresent*/, queue.MaxRetrievable());
        assert(queue.IsEmpty());
        AutoSeededRandomPool prng;
        bool valid = pk.Validate(prng, 3);
        if(!valid)
            cerr << "RSA private key is not valid" << endl;
        cout << "N:" << pk.GetModulus() << endl;
        cout << "E:" << pk.GetPublicExponent() << endl;
    }
    
    catch (const Exception& ex)
    {
        cerr << ex.what() << endl;
        exit (1);
    }
}
