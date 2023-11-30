#include "opensslapi.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <stdio.h>
#include <string>
#include <sstream>


// Generates an RSA public-private key pair and returns it.
// The number of bits is specified by the bits argument.
//
// This uses the long way of generating an RSA key.
//
EVP_PKEY *generate_rsa_key_long(OSSL_LIB_CTX *libctx, unsigned int bits)
{
    // A property query used for selecting algorithm implementations.
    const char *propq = NULL;
    EVP_PKEY_CTX *genctx = NULL;
    EVP_PKEY *pkey = NULL;
    unsigned int primes = 2;

    // Create context using RSA algorithm. "RSA-PSS" could also be used here.
    genctx = EVP_PKEY_CTX_new_from_name(libctx, "RSA", propq);
    if (genctx == NULL)
    {
        goto cleanup;
    }

    // Initialize context for key generation purposes.
    if (EVP_PKEY_keygen_init(genctx) <= 0)
    {
        goto cleanup;
    }

    // Here we set the number of bits to use in the RSA key.
    // See comment at top of file for information on appropriate values.
    //
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(genctx, bits) <= 0)
    {
        goto cleanup;
    }

    // It is possible to create an RSA key using more than two primes.
    // Do not do this unless you know why you need this.
    // You ordinarily do not need to specify this, as the default is two.
    //
    // Both of these parameters can also be set via EVP_PKEY_CTX_set_params, but
    // these functions provide a more concise way to do so.
    if (EVP_PKEY_CTX_set_rsa_keygen_primes(genctx, primes) <= 0)
    {
        goto cleanup;
    }

    // Generating an RSA key with a number of bits large enough to be secure for
    // modern applications can take a fairly substantial amount of time (e.g.
    // one second). If you require fast key generation, consider using an EC key
    // instead.
    //
    // If you require progress information during the key generation process,
    // you can set a progress callback using EVP_PKEY_set_cb; see the example in
    // EVP_PKEY_generate(3).
    if (EVP_PKEY_generate(genctx, &pkey) <= 0)
    {
        goto cleanup;
    }

    // pkey is now set to an object representing the generated key pair.
    cleanup:
    EVP_PKEY_CTX_free(genctx);
    return pkey;
}


void base64Encode(const unsigned char* buffer,
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
    BUF_MEM_free(bufferPtr);
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

void base64Decode(const char* b64message, std::string& out)
{
    BIO *bio, *b64;

    int decodeLen = calcDecodeLength(b64message);
    unsigned char *buffer = (unsigned char*)malloc(decodeLen + 1);
    buffer[decodeLen] = '\0';

    bio = BIO_new_mem_buf(b64message, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    int length = BIO_read(bio, buffer, strlen(b64message));
    BIO_free_all(bio);

    out = std::string(reinterpret_cast<char*>(buffer), length);
    OPENSSL_free(buffer);
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

namespace nap
{
    namespace opensslapi
    {
        bool generateRSAKey(unsigned int bits, std::string& outPrivKey, std::string& outPubKey)
        {
            int ret = EXIT_FAILURE;
            OSSL_LIB_CTX *libctx = NULL;
            EVP_PKEY *pkey = NULL;
            BIO* pub_bio = BIO_new(BIO_s_mem());
            BIO* priv_bio = BIO_new(BIO_s_mem());
            unsigned char *data;
            int len;

            // Generate RSA key.
            pkey = generate_rsa_key_long(libctx, bits);

            if (pkey == NULL)
                goto cleanup;

            // Output a PEM encoding of the public key.
            if (PEM_write_bio_PUBKEY(pub_bio, pkey) == 0)
            {
                goto cleanup;
            }
            len = BIO_get_mem_data(pub_bio, &data);
            outPubKey = std::string(reinterpret_cast<char*>(data), len);

            // Output a PEM encoding of the private key. Please note that this output is
            // not encrypted. You may wish to use the arguments to specify encryption of
            // the key if you are storing it on disk. See PEM_write_PrivateKey(3).
            if (PEM_write_bio_PrivateKey(priv_bio, pkey, NULL, NULL, 0, NULL, NULL) == 0)
            {
                goto cleanup;
            }
            len = BIO_get_mem_data(priv_bio, &data);
            outPrivKey = std::string(reinterpret_cast<char*>(data), len);

            ret = EXIT_SUCCESS;
            cleanup:
            EVP_PKEY_free(pkey);
            OSSL_LIB_CTX_free(libctx);
            BIO_free(pub_bio);
            BIO_free(priv_bio);
            return ret == EXIT_SUCCESS;
        }


        bool createSignature(const std::string &privkey, const std::string &message, const std::string &signingScheme, std::string &outSignature)
        {
            EVP_MD_CTX *mdctx = NULL;
            int ret = 0;
            size_t slen = 0;
            unsigned char *sig = NULL;
            const evp_md_st *scheme = NULL;

            // Read the private key
            BIO *in = BIO_new_mem_buf((unsigned char *) privkey.c_str(), strlen(privkey.c_str()));
            auto *key = PEM_read_bio_PrivateKey(in, NULL, 0, NULL);

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
            sig = reinterpret_cast<unsigned char *>(OPENSSL_malloc(sizeof(unsigned char) * (slen)));

            if(sig == NULL) goto err;

            // Obtain the signature
            if(1 != EVP_DigestSignFinal(mdctx, sig, &slen)) goto err;

            // Success
            ret = 1;
            err:

            // If successful, encode the signature
            if(ret == 1)
            {
                // encode the signature
                base64Encode(sig, slen, outSignature);
            }

            // Clean up
            OPENSSL_free(sig);
            EVP_MD_CTX_destroy(mdctx);
            BIO_free(in);

            return ret == 1;
        }


        bool verifyMessage(const std::string &pubkey, const std::string &message, const std::string &signingScheme, const std::string &signature)
        {
            EVP_MD_CTX *mdctx = NULL;
            int ret = 0;
            size_t slen = signature.length();
            unsigned char *sig = NULL;
            const evp_md_st *scheme = NULL;

            // Decode signature
            std::string decodedSignature;
            base64Decode(signature.c_str(), decodedSignature);
            slen = decodedSignature.length();
            sig = reinterpret_cast<unsigned char *>(OPENSSL_malloc(sizeof(unsigned char) * (slen)));
            memcpy(sig, decodedSignature.c_str(), slen);

            // Read the public key
            BIO *in = BIO_new_mem_buf((unsigned char *) pubkey.c_str(), strlen(pubkey.c_str()));
            auto *key = PEM_read_bio_PUBKEY(in, NULL, 0, NULL);

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
            } else
            {
                // Failure
                ret = 0;
            }
            err:

            // Clean up
            EVP_MD_CTX_destroy(mdctx);
            BIO_free(in);
            OPENSSL_free(sig);

            return ret == 1;
        }


        std::string sha256(const std::string& str)
        {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            return { reinterpret_cast<const char *>(SHA256(reinterpret_cast<const unsigned char *>(str.c_str()), str.length(), hash)) };
        }


        std::string encode64(const std::string& str)
        {
            std::string out;
            base64Encode(reinterpret_cast<const unsigned char *>(str.c_str()), str.length(), out);
            return out;
        }


        std::string decode64(const std::string& str)
        {
            std::string out;
            base64Decode(str.c_str(), out);
            return out;
        }
    }
}