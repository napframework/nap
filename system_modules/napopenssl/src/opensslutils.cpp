/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "opensslutils.h"
#include "opensslapi.h"

using namespace nap;

bool convertSigningScheme(ESigningScheme scheme, std::string& out)
{
    switch(scheme)
    {
        case ESigningScheme::SHA1:
            out = "SHA1";
            break;
        case ESigningScheme::SHA224:
            out = "SHA224";
            break;
        case ESigningScheme::SHA256:
            out = "SHA256";
            break;
        case ESigningScheme::SHA384:
            out = "SHA384";
            break;
        case ESigningScheme::SHA512:
            out = "SHA512";
            break;
        default:
            return false;
    }

    return true;
}

namespace nap
{
namespace utility
{
    bool createSignature(const std::string& privkey, const std::string& message, ESigningScheme scheme, std::string& outSignature)
    {
        std::string signing_scheme;
        if(!convertSigningScheme(scheme, signing_scheme))
        {
            return false;
        }
        return opensslapi::createSignature(privkey, message, signing_scheme, outSignature);
    }


    bool verifyMessage(const std::string &pubkey, const std::string &message, ESigningScheme scheme, const std::string &signature)
    {
        std::string signing_scheme;
        if(!convertSigningScheme(scheme, signing_scheme))
        {
            return false;
        }
        return opensslapi::verifyMessage(pubkey, message, signing_scheme, signature);
    }


    bool generateRSAKey(unsigned int bits, std::string &outPrivKey, std::string &outPubKey)
    {
        return opensslapi::generateRSAKey(bits, outPrivKey, outPubKey);
    }


    std::string sha256(const std::string& str)
    {
        return opensslapi::sha256(str);
    }


    std::string encode64(const std::string& str)
    {
        return opensslapi::encode64(str);
    }


    std::string decode64(const std::string& str)
    {
        return opensslapi::decode64(str);
    }
}
}

