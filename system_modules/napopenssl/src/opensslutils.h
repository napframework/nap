#pragma once

#include <string>

namespace nap
{
namespace openssl
{
    namespace utility
    {
        /**
         * @brief Create a signature for a given message
         * @param privkey The private key to use
         * @param message The message to sign
         * @param signingScheme The signing scheme to use
         * @param outSignature The signature
         * @return True if the signature was created successfully
         */
        bool createSignature(const std::string& privkey, const std::string& message, const std::string& signingScheme, std::string& outSignature);

        /**
         * @brief Verify a message against a signature
         * @param pubkey The public key to use
         * @param message The message to verify
         * @param signingScheme The signing scheme to use
         * @param signature The signature
         * @return True if the message was verified successfully
         */
        bool verifyMessage(const std::string& pubkey, const std::string& message, const std::string& signingScheme, const std::string& signature);
    }
}
}