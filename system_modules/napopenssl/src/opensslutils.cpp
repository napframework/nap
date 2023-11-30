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


bool nap::utility::createSignature(const std::string& privkey, const std::string& message, ESigningScheme scheme, std::string& outSignature)
{
    std::string signing_scheme;
    if(!convertSigningScheme(scheme, signing_scheme))
    {
        return false;
    }
    return opensslapi::createSignature(privkey, message, signing_scheme, outSignature);
}


bool nap::utility::verifyMessage(const std::string &pubkey, const std::string &message, ESigningScheme scheme, const std::string &signature)
{
    std::string signing_scheme;
    if(!convertSigningScheme(scheme, signing_scheme))
    {
        return false;
    }
    return opensslapi::verifyMessage(pubkey, message, signing_scheme, signature);
}


bool nap::utility::generateRSAKey(unsigned int bits, std::string &outPrivKey, std::string &outPubKey)
{
    return opensslapi::generateRSAKey(bits, outPrivKey, outPubKey);
}


std::string nap::utility::sha256(const std::string& str)
{
    return opensslapi::sha256(str);
}


std::string nap::utility::encode64(const std::string& str)
{
    return opensslapi::encode64(str);
}


std::string nap::utility::decode64(const std::string& str)
{
    return opensslapi::decode64(str);
}
