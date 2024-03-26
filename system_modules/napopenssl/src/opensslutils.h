/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "signingscheme.h"

// External Includes
#include <string>
#include <utility/dllexport.h>


namespace nap
{
    namespace utility
    {
        /**
         * Create a signature for a given message. outSignature is encoded in base64.
         * @param privkey The private key to use
         * @param message The message to sign
         * @param signingScheme The signing scheme to use
         * @param outSignature The signature encoded in base64
         * @return True if the signature was created successfully
         */
        bool NAPAPI createSignature(const std::string& privkey, const std::string& message, ESigningScheme scheme, std::string& outSignature);

        /**
         * Verify a message against a signature. Signature is expected to be encoded in base64.
         * @param pubkey The public key to use
         * @param message The message to verify
         * @param signingScheme The signing scheme to use
         * @param signature The signature encoded in base64
         * @return True if the message was verified successfully
         */
        bool NAPAPI verifyMessage(const std::string& pubkey, const std::string& message, ESigningScheme scheme, const std::string& signature);

        /**
         * Generate a RSA public-private key pair.
         * @param bits number of bits
         * @param outPrivKey string will be filled with the private key
         * @param outPubKey string will be filled with the public key
         * @return True if the key pair was generated successfully
         */
        bool NAPAPI generateRSAKey(unsigned int bits, std::string& outPrivKey, std::string& outPubKey);

        /**
         * @brief Generate a SHA256 hash of a string
         * @param str The string to hash
         * @return The hash
         */
        std::string NAPAPI sha256(const std::string& str);

        /**
         * @brief Encode a string to base64
         * @param str The string to encode
         * @return The encoded string
         */
        std::string NAPAPI encode64(const std::string& str);

        /**
         * @brief Decode a base64 string
         * @param str The string to decode
         * @return The decoded string
         */
        std::string NAPAPI decode64(const std::string& str);
    }
}
