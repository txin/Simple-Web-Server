#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <cryptopp/cryptlib.h>
#include <cryptopp/pem.h>
// prime generator
#include <cryptopp/nbtheory.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/polynomi.h>
#include <cryptopp/modes.h>

#include <cryptopp/files.h>
#include <cryptopp/secblock.h>
#include <cryptopp/gcm.h>
#include <cryptopp/filters.h>
#include <cryptopp/rsa.h>
#include <cryptopp/base64.h>
#include <cryptopp/randpool.h>
#include <cryptopp/hex.h>

#include <assert.h>
#include <string>


#define D_KEY_SIZE 32 // default key size for symmetric key, bytes

void encrypt_file_CBC_AES(CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv,
                          const char *infile,
                          const char *outfile );

void decrypt_file_CBC_AES(CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv,
                          const char *infile,
                          const char* outfile );

void encrypt_file_GCM_AES(CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv,
                          const char *infile,
                          const char* outfile );

void decrypt_file_GCM_AES(CryptoPP::SecByteBlock key, CryptoPP::SecByteBlock iv,
                          const char *infile,
                          const char* outfile );

void load_public_key(const std::string& file, 
                     CryptoPP::RSA::PublicKey& key);

void load_private_key(const std::string& file, 
                      CryptoPP::RSA::PrivateKey& key);

void load_private_key(const std::string &file_name,
                      CryptoPP::RSA::PrivateKey &sk, const char* pwd,
                      size_t length);

// decrypt with RSA
void decrypt_string_RSA(std::string &recovered, const std::string &file,
                      CryptoPP::RandomNumberGenerator &rng,
                      CryptoPP::RSA::PrivateKey &sk);

void encrypt_string_RSA(const std::string &plain, const std::string &file,
                      CryptoPP::RandomNumberGenerator &rng,
                      CryptoPP::RSA::PublicKey &pk);

void encrypt_file_1(const std::string &in_file);

void encrypt_file_2(const std::string &in_file);

#endif
