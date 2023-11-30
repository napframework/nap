#pragma once

#include <string>

namespace nap
{
    namespace opensslapi
    {
        /**
         * @brief Creates signature for a given message using the given signing scheme and private key
         * @param privkey the private key to use
         * @param message the message to sign
         * @param signingScheme the signing scheme to use
         * @param outSignature the created signature
         * @return true if the signature was created successfully
         */
        bool createSignature(const std::string &privkey, const std::string &message, const std::string &signingScheme, std::string &outSignature);

        /**
         * @brief Verifies a message against a signature using the given signing scheme and public key
         * @param pubkey the public key to use
         * @param message the message to verify
         * @param signingScheme the signing scheme to use
         * @param signature the signature to verify against
         * @return true if the message was verified successfully
         */
        bool verifyMessage(const std::string &pubkey, const std::string &message, const std::string &signingScheme, const std::string &signature);

        /**
         * @brief Generates a RSA key pair
         * @param bits number of bits
         * @param outPrivKey the private key generated
         * @param outPubKey the public key generated
         * @return true if the key pair was generated successfully
         */
        bool generateRSAKey(unsigned int bits, std::string& outPrivKey, std::string& outPubKey);

        /**
         * @brief Generates a SHA256 hash of a string
         * @param str the string to hash
         * @return the hash
         */
        std::string sha256(const std::string& str);

        /**
         * @brief Encodes a string to base64
         * @param str the string to encode
         * @return the encoded string
         */
        std::string encode64(const std::string& str);

        /**
         * @brief Decodes a base64 string
         * @param str the string to decode
         * @return the decoded string
         */
         std::string decode64(const std::string& str);
    }
}
