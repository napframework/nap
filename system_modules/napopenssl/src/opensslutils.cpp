#include "opensslutils.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string>


void Base64Encode( const unsigned char* buffer,
                   size_t length,
                   std::string& outString) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    outString= std::string((*bufferPtr).data);
}

size_t calcDecodeLength(const char* b64input)
{
    size_t len = strlen(b64input), padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len-1] == '=') //last char is =
        padding = 1;
    return (len*3)/4 - padding;
}

void Base64Decode(const char* b64message, unsigned char** buffer, size_t* length)
{
    BIO *bio, *b64;

    int decodeLen = calcDecodeLength(b64message);
    *buffer = (unsigned char*)malloc(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    bio = BIO_new_mem_buf(b64message, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    *length = BIO_read(bio, *buffer, strlen(b64message));
    BIO_free_all(bio);
}

constexpr const char* supportedSigningSchemes[] =
{
    "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", ""
};

const evp_md_st* getSigningScheme(const std::string& signingScheme)
{
    if(signingScheme == "SHA1")
    {
        return EVP_sha1();
    }
    else if(signingScheme == "SHA224")
    {
        return EVP_sha224();
    }
    else if(signingScheme == "SHA256")
    {
        return EVP_sha256();
    }
    else if(signingScheme == "SHA384")
    {
        return EVP_sha384();
    }
    else if(signingScheme == "SHA512")
    {
        return EVP_sha512();
    }
    else
    {
        return nullptr;
    }
}

bool nap::openssl::utility::createSignature(const std::string& privkey, const std::string& message, const std::string& signingScheme, std::string& outSignature)
{
    EVP_MD_CTX *mdctx = NULL;
    int ret = 0;
    size_t slen = 0;
    unsigned char *sig = NULL;
    const evp_md_st* scheme = NULL;

    // Read the private key
    BIO* in = BIO_new_mem_buf((unsigned char*)privkey.c_str(), strlen(privkey.c_str()));
    auto* key = PEM_read_bio_PrivateKey(in, NULL, 0, NULL);

    // Get the signing scheme
    if(!(scheme = getSigningScheme(signingScheme))) goto err;

    // Create the Message Digest Context
    if(!(mdctx = EVP_MD_CTX_create())) goto err;

    // Initialise the DigestSign operation
    if(1 != EVP_DigestSignInit(mdctx, NULL, scheme, NULL, key)) goto err;

    // Call update with the message
    if(1 != EVP_DigestSignUpdate(mdctx, message.c_str(), strlen(message.c_str()))) goto err;

    // Finalise the DigestSign operation
    // First call EVP_DigestSignFinal with a NULL sig parameter to obtain the length of the
    // signature. Length is returned in slen
    if(1 != EVP_DigestSignFinal(mdctx, NULL, &slen)) goto err;

    // Allocate memory for the signature based on size in slen
    sig = reinterpret_cast<unsigned char*>(OPENSSL_malloc(sizeof(unsigned char) * (slen)));

    if(sig == NULL) goto err;

    // Obtain the signature
    if(1 != EVP_DigestSignFinal(mdctx, sig, &slen)) goto err;

    // Success
    ret = 1;
    err:

    // If successfull, encode the signature
    if(ret == 1)
    {
        // encode the signature
        Base64Encode(sig, slen, outSignature);
    }

    // Clean up
    if(*sig && !ret) OPENSSL_free(reinterpret_cast<void *>(*sig));
    if(mdctx) EVP_MD_CTX_destroy(mdctx);
    if(in) BIO_free(in);

    return ret == 1;
}


bool nap::openssl::utility::verifyMessage(const std::string &pubkey, const std::string &message, const std::string& signingScheme, const std::string &signature)
{
    EVP_MD_CTX *mdctx = NULL;
    int ret = 0;
    size_t slen = signature.length();
    unsigned char *sig = NULL;
    const evp_md_st* scheme = NULL;

    // Decode signature
    Base64Decode(signature.c_str(), &sig, &slen);

    // Read the public key
    BIO* in = BIO_new_mem_buf((unsigned char*)pubkey.c_str(), strlen(pubkey.c_str()));
    auto* key = PEM_read_bio_PUBKEY(in, NULL, 0, NULL);

    // Get the signing scheme
    if(!(scheme = getSigningScheme(signingScheme))) goto err;

    // Create the Message Digest Context
    if(!(mdctx = EVP_MD_CTX_create())) goto err;

    // Initialize `key` with a public key
    if(1 != EVP_DigestVerifyInit(mdctx, NULL, scheme, NULL, key)) goto err;

    // Initialize `key` with a public key
    if(1 != EVP_DigestVerifyUpdate(mdctx, message.c_str(), strlen(message.c_str()))) goto err;

    if(1 == EVP_DigestVerifyFinal(mdctx, sig, slen))
    {
        // Success
        ret = 1;
    }
    else
    {
        // Failure
        ret = 0;
    }
    err:

    // Clean up
    if(mdctx) EVP_MD_CTX_destroy(mdctx);
    if(in) BIO_free(in);

    return ret == 1;
}