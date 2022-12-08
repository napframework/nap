/*********************************************************************
 *
 * $Id: yocto_messagebox.cpp 28753 2017-10-03 11:23:38Z seb $
 *
 * Implements yFindMessageBox(), the high-level API for MessageBox functions
 *
 * - - - - - - - - - License information: - - - - - - - - -
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing
 *  with Yoctopuce products.
 *
 *  You may reproduce and distribute copies of this file in
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED 'AS IS' WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/


#define _CRT_SECURE_NO_DEPRECATE //do not use windows secure crt
#include "yocto_messagebox.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#define  __FILE_ID__  "messagebox"


YSms::YSms(void):
//--- (generated code: YSms initialization)
    _mbox(NULL)
    ,_slot(0)
    ,_deliv(0)
    ,_mref(0)
    ,_pid(0)
    ,_alphab(0)
    ,_mclass(0)
    ,_npdu(0)
    ,_aggIdx(0)
    ,_aggCnt(0)
//--- (end of generated code: YSms initialization)
{ }

YSms::YSms(YMessageBox *mbox) :
//--- (generated code: YSms initialization)
    _mbox(NULL)
    ,_slot(0)
    ,_deliv(0)
    ,_mref(0)
    ,_pid(0)
    ,_alphab(0)
    ,_mclass(0)
    ,_npdu(0)
    ,_aggIdx(0)
    ,_aggCnt(0)
//--- (end of generated code: YSms initialization)
{
    _mbox = mbox;
}

//--- (generated code: YSms implementation)
// static attributes


int YSms::get_slot(void)
{
    return _slot;
}

string YSms::get_smsc(void)
{
    return _smsc;
}

int YSms::get_msgRef(void)
{
    return _mref;
}

string YSms::get_sender(void)
{
    return _orig;
}

string YSms::get_recipient(void)
{
    return _dest;
}

int YSms::get_protocolId(void)
{
    return _pid;
}

bool YSms::isReceived(void)
{
    return _deliv;
}

int YSms::get_alphabet(void)
{
    return _alphab;
}

int YSms::get_msgClass(void)
{
    if (((_mclass) & (16)) == 0) {
        return -1;
    }
    return ((_mclass) & (3));
}

int YSms::get_dcs(void)
{
    return ((_mclass) | ((((_alphab) << (2)))));
}

string YSms::get_timestamp(void)
{
    return _stamp;
}

string YSms::get_userDataHeader(void)
{
    return _udh;
}

string YSms::get_userData(void)
{
    return _udata;
}

string YSms::get_textData(void)
{
    string isolatin;
    int isosize = 0;
    int i = 0;
    if (_alphab == 0) {
        // using GSM standard 7-bit alphabet
        return _mbox->gsm2str(_udata);
    }
    if (_alphab == 2) {
        // using UCS-2 alphabet
        isosize = (((int)(_udata).size()) >> (1));
        isolatin = string(isosize, (char)0);
        i = 0;
        while (i < isosize) {
            isolatin[i] = (char)(((u8)_udata[2*i+1]));
            i = i + 1;
        }
        return isolatin;
    }
    // default: convert 8 bit to string as-is
    return _udata;
}

vector<int> YSms::get_unicodeData(void)
{
    vector<int> res;
    int unisize = 0;
    int unival = 0;
    int i = 0;
    if (_alphab == 0) {
        // using GSM standard 7-bit alphabet
        return _mbox->gsm2unicode(_udata);
    }
    if (_alphab == 2) {
        // using UCS-2 alphabet
        unisize = (((int)(_udata).size()) >> (1));
        res.clear();
        i = 0;
        while (i < unisize) {
            unival = 256*((u8)_udata[2*i])+((u8)_udata[2*i+1]);
            res.push_back(unival);
            i = i + 1;
        }
    } else {
        // return straight 8-bit values
        unisize = (int)(_udata).size();
        res.clear();
        i = 0;
        while (i < unisize) {
            res.push_back(((u8)_udata[i])+0);
            i = i + 1;
        }
    }
    return res;
}

int YSms::get_partCount(void)
{
    if (_npdu == 0) {
        this->generatePdu();
    }
    return _npdu;
}

string YSms::get_pdu(void)
{
    if (_npdu == 0) {
        this->generatePdu();
    }
    return _pdu;
}

vector<YSms> YSms::get_parts(void)
{
    if (_npdu == 0) {
        this->generatePdu();
    }
    return _parts;
}

string YSms::get_concatSignature(void)
{
    if (_npdu == 0) {
        this->generatePdu();
    }
    return _aggSig;
}

int YSms::get_concatIndex(void)
{
    if (_npdu == 0) {
        this->generatePdu();
    }
    return _aggIdx;
}

int YSms::get_concatCount(void)
{
    if (_npdu == 0) {
        this->generatePdu();
    }
    return _aggCnt;
}

int YSms::set_slot(int val)
{
    _slot = val;
    return YAPI_SUCCESS;
}

int YSms::set_received(bool val)
{
    _deliv = val;
    return YAPI_SUCCESS;
}

int YSms::set_smsc(string val)
{
    _smsc = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_msgRef(int val)
{
    _mref = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_sender(string val)
{
    _orig = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_recipient(string val)
{
    _dest = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_protocolId(int val)
{
    _pid = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_alphabet(int val)
{
    _alphab = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_msgClass(int val)
{
    if (val == -1) {
        _mclass = 0;
    } else {
        _mclass = 16+val;
    }
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_dcs(int val)
{
    _alphab = (((((val) >> (2)))) & (3));
    _mclass = ((val) & (16+3));
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_timestamp(string val)
{
    _stamp = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::set_userDataHeader(string val)
{
    _udh = val;
    _npdu = 0;
    this->parseUserDataHeader();
    return YAPI_SUCCESS;
}

int YSms::set_userData(string val)
{
    _udata = val;
    _npdu = 0;
    return YAPI_SUCCESS;
}

int YSms::convertToUnicode(void)
{
    vector<int> ucs2;
    int udatalen = 0;
    int i = 0;
    int uni = 0;
    if (_alphab == 2) {
        return YAPI_SUCCESS;
    }
    if (_alphab == 0) {
        ucs2 = _mbox->gsm2unicode(_udata);
    } else {
        udatalen = (int)(_udata).size();
        ucs2.clear();
        i = 0;
        while (i < udatalen) {
            uni = ((u8)_udata[i]);
            ucs2.push_back(uni);
            i = i + 1;
        }
    }
    _alphab = 2;
    _udata = string(0, (char)0);
    this->addUnicodeData(ucs2);
    return YAPI_SUCCESS;
}

int YSms::addText(string val)
{
    string udata;
    int udatalen = 0;
    string newdata;
    int newdatalen = 0;
    int i = 0;
    if ((int)(val).length() == 0) {
        return YAPI_SUCCESS;
    }
    if (_alphab == 0) {
        // Try to append using GSM 7-bit alphabet
        newdata = _mbox->str2gsm(val);
        newdatalen = (int)(newdata).size();
        if (newdatalen == 0) {
            // 7-bit not possible, switch to unicode
            this->convertToUnicode();
            newdata = val;
            newdatalen = (int)(newdata).size();
        }
    } else {
        newdata = val;
        newdatalen = (int)(newdata).size();
    }
    udatalen = (int)(_udata).size();
    if (_alphab == 2) {
        // Append in unicode directly
        udata = string(udatalen + 2*newdatalen, (char)0);
        i = 0;
        while (i < udatalen) {
            udata[i] = (char)(((u8)_udata[i]));
            i = i + 1;
        }
        i = 0;
        while (i < newdatalen) {
            udata[udatalen+1] = (char)(((u8)newdata[i]));
            udatalen = udatalen + 2;
            i = i + 1;
        }
    } else {
        // Append binary buffers
        udata = string(udatalen+newdatalen, (char)0);
        i = 0;
        while (i < udatalen) {
            udata[i] = (char)(((u8)_udata[i]));
            i = i + 1;
        }
        i = 0;
        while (i < newdatalen) {
            udata[udatalen] = (char)(((u8)newdata[i]));
            udatalen = udatalen + 1;
            i = i + 1;
        }
    }
    return this->set_userData(udata);
}

int YSms::addUnicodeData(vector<int> val)
{
    int arrlen = 0;
    int newdatalen = 0;
    int i = 0;
    int uni = 0;
    string udata;
    int udatalen = 0;
    int surrogate = 0;
    if (_alphab != 2) {
        this->convertToUnicode();
    }
    // compute number of 16-bit code units
    arrlen = (int)val.size();
    newdatalen = arrlen;
    i = 0;
    while (i < arrlen) {
        uni = val[i];
        if (uni > 65535) {
            newdatalen = newdatalen + 1;
        }
        i = i + 1;
    }
    // now build utf-16 buffer
    udatalen = (int)(_udata).size();
    udata = string(udatalen+2*newdatalen, (char)0);
    i = 0;
    while (i < udatalen) {
        udata[i] = (char)(((u8)_udata[i]));
        i = i + 1;
    }
    i = 0;
    while (i < arrlen) {
        uni = val[i];
        if (uni >= 65536) {
            surrogate = uni - 65536;
            uni = (((((surrogate) >> (10))) & (1023))) + 55296;
            udata[udatalen] = (char)(((uni) >> (8)));
            udata[udatalen+1] = (char)(((uni) & (255)));
            udatalen = udatalen + 2;
            uni = (((surrogate) & (1023))) + 56320;
        }
        udata[udatalen] = (char)(((uni) >> (8)));
        udata[udatalen+1] = (char)(((uni) & (255)));
        udatalen = udatalen + 2;
        i = i + 1;
    }
    return this->set_userData(udata);
}

int YSms::set_pdu(string pdu)
{
    _pdu = pdu;
    _npdu = 1;
    return this->parsePdu(pdu);
}

int YSms::set_parts(vector<YSms> parts)
{
    vector<YSms> sorted;
    int partno = 0;
    int initpartno = 0;
    int i = 0;
    int retcode = 0;
    int totsize = 0;
    YSms subsms;
    string subdata;
    string res;
    _npdu = (int)parts.size();
    if (_npdu == 0) {
        return YAPI_INVALID_ARGUMENT;
    }
    sorted.clear();
    partno = 0;
    while (partno < _npdu) {
        initpartno = partno;
        i = 0;
        while (i < _npdu) {
            subsms = parts[i];
            if (subsms.get_concatIndex() == partno) {
                sorted.push_back(subsms);
                partno = partno + 1;
            }
            i = i + 1;
        }
        if (initpartno == partno) {
            partno = partno + 1;
        }
    }
    _parts = sorted;
    _npdu = (int)sorted.size();
    // inherit header fields from first part
    subsms = _parts[0];
    retcode = this->parsePdu(subsms.get_pdu());
    if (retcode != YAPI_SUCCESS) {
        return retcode;
    }
    // concatenate user data from all parts
    totsize = 0;
    partno = 0;
    while (partno < (int)_parts.size()) {
        subsms = _parts[partno];
        subdata = subsms.get_userData();
        totsize = totsize + (int)(subdata).size();
        partno = partno + 1;
    }
    res = string(totsize, (char)0);
    totsize = 0;
    partno = 0;
    while (partno < (int)_parts.size()) {
        subsms = _parts[partno];
        subdata = subsms.get_userData();
        i = 0;
        while (i < (int)(subdata).size()) {
            res[totsize] = (char)(((u8)subdata[i]));
            totsize = totsize + 1;
            i = i + 1;
        }
        partno = partno + 1;
    }
    _udata = res;
    return YAPI_SUCCESS;
}

string YSms::encodeAddress(string addr)
{
    string bytes;
    int srclen = 0;
    int numlen = 0;
    int i = 0;
    int val = 0;
    int digit = 0;
    string res;
    bytes = addr;
    srclen = (int)(bytes).size();
    numlen = 0;
    i = 0;
    while (i < srclen) {
        val = ((u8)bytes[i]);
        if ((val >= 48) && (val < 58)) {
            numlen = numlen + 1;
        }
        i = i + 1;
    }
    if (numlen == 0) {
        res = string(1, (char)0);
        res[0] = (char)(0);
        return res;
    }
    res = string(2+((numlen+1) >> (1)), (char)0);
    res[0] = (char)(numlen);
    if (((u8)bytes[0]) == 43) {
        res[1] = (char)(145);
    } else {
        res[1] = (char)(129);
    }
    numlen = 4;
    digit = 0;
    i = 0;
    while (i < srclen) {
        val = ((u8)bytes[i]);
        if ((val >= 48) && (val < 58)) {
            if (((numlen) & (1)) == 0) {
                digit = val - 48;
            } else {
                res[((numlen) >> (1))] = (char)(digit + 16*(val-48));
            }
            numlen = numlen + 1;
        }
        i = i + 1;
    }
    // pad with F if needed
    if (((numlen) & (1)) != 0) {
        res[((numlen) >> (1))] = (char)(digit + 240);
    }
    return res;
}

string YSms::decodeAddress(string addr,int ofs,int siz)
{
    int addrType = 0;
    string gsm7;
    string res;
    int i = 0;
    int rpos = 0;
    int carry = 0;
    int nbits = 0;
    int byt = 0;
    if (siz == 0) {
        return "";
    }
    res = "";
    addrType = ((((u8)addr[ofs])) & (112));
    if (addrType == 80) {
        // alphanumeric number
        siz = ((4*siz) / (7));
        gsm7 = string(siz, (char)0);
        rpos = 1;
        carry = 0;
        nbits = 0;
        i = 0;
        while (i < siz) {
            if (nbits == 7) {
                gsm7[i] = (char)(carry);
                carry = 0;
                nbits = 0;
            } else {
                byt = ((u8)addr[ofs+rpos]);
                rpos = rpos + 1;
                gsm7[i] = (char)(((carry) | ((((((byt) << (nbits)))) & (127)))));
                carry = ((byt) >> ((7 - nbits)));
                nbits = nbits + 1;
            }
            i = i + 1;
        }
        return _mbox->gsm2str(gsm7);
    } else {
        // standard phone number
        if (addrType == 16) {
            res = "+";
        }
        siz = (((siz+1)) >> (1));
        i = 0;
        while (i < siz) {
            byt = ((u8)addr[ofs+i+1]);
            res = YapiWrapper::ysprintf("%s%x%x", res.c_str(), ((byt) & (15)),((byt) >> (4)));
            i = i + 1;
        }
        // remove padding digit if needed
        if (((((u8)addr[ofs+siz])) >> (4)) == 15) {
            res = (res).substr( 0, (int)(res).length()-1);
        }
        return res;
    }
}

string YSms::encodeTimeStamp(string exp)
{
    int explen = 0;
    int i = 0;
    string res;
    int n = 0;
    string expasc;
    int v1 = 0;
    int v2 = 0;
    explen = (int)(exp).length();
    if (explen == 0) {
        res = string(0, (char)0);
        return res;
    }
    if ((exp).substr(0, 1) == "+") {
        n = atoi(((exp).substr(1, explen-1)).c_str());
        res = string(1, (char)0);
        if (n > 30*86400) {
            n = 192+(((n+6*86400)) / ((7*86400)));
        } else {
            if (n > 86400) {
                n = 166+(((n+86399)) / (86400));
            } else {
                if (n > 43200) {
                    n = 143+(((n-43200+1799)) / (1800));
                } else {
                    n = -1+(((n+299)) / (300));
                }
            }
        }
        if (n < 0) {
            n = 0;
        }
        res[0] = (char)(n);
        return res;
    }
    if ((exp).substr(4, 1) == "-" || (exp).substr(4, 1) == "/") {
        // ignore century
        exp = (exp).substr( 2, explen-2);
        explen = (int)(exp).length();
    }
    expasc = exp;
    res = string(7, (char)0);
    n = 0;
    i = 0;
    while ((i+1 < explen) && (n < 7)) {
        v1 = ((u8)expasc[i]);
        if ((v1 >= 48) && (v1 < 58)) {
            v2 = ((u8)expasc[i+1]);
            if ((v2 >= 48) && (v2 < 58)) {
                v1 = v1 - 48;
                v2 = v2 - 48;
                res[n] = (char)((((v2) << (4))) + v1);
                n = n + 1;
                i = i + 1;
            }
        }
        i = i + 1;
    }
    while (n < 7) {
        res[n] = (char)(0);
        n = n + 1;
    }
    if (i+2 < explen) {
        // convert for timezone in cleartext ISO format +/-nn:nn
        v1 = ((u8)expasc[i-3]);
        v2 = ((u8)expasc[i]);
        if (((v1 == 43) || (v1 == 45)) && (v2 == 58)) {
            v1 = ((u8)expasc[i+1]);
            v2 = ((u8)expasc[i+2]);
            if ((v1 >= 48) && (v1 < 58) && (v1 >= 48) && (v1 < 58)) {
                v1 = (((10*(v1 - 48)+(v2 - 48))) / (15));
                n = n - 1;
                v2 = 4 * ((u8)res[n]) + v1;
                if (((u8)expasc[i-3]) == 45) {
                    v2 += 128;
                }
                res[n] = (char)(v2);
            }
        }
    }
    return res;
}

string YSms::decodeTimeStamp(string exp,int ofs,int siz)
{
    int n = 0;
    string res;
    int i = 0;
    int byt = 0;
    string sign;
    string hh;
    string ss;
    if (siz < 1) {
        return "";
    }
    if (siz == 1) {
        n = ((u8)exp[ofs]);
        if (n < 144) {
            n = n * 300;
        } else {
            if (n < 168) {
                n = (n-143) * 1800;
            } else {
                if (n < 197) {
                    n = (n-166) * 86400;
                } else {
                    n = (n-192) * 7 * 86400;
                }
            }
        }
        return YapiWrapper::ysprintf("+%d",n);
    }
    res = "20";
    i = 0;
    while ((i < siz) && (i < 6)) {
        byt = ((u8)exp[ofs+i]);
        res = YapiWrapper::ysprintf("%s%x%x", res.c_str(), ((byt) & (15)),((byt) >> (4)));
        if (i < 3) {
            if (i < 2) {
                res = YapiWrapper::ysprintf("%s-",res.c_str());
            } else {
                res = YapiWrapper::ysprintf("%s ",res.c_str());
            }
        } else {
            if (i < 5) {
                res = YapiWrapper::ysprintf("%s:",res.c_str());
            }
        }
        i = i + 1;
    }
    if (siz == 7) {
        byt = ((u8)exp[ofs+i]);
        sign = "+";
        if (((byt) & (8)) != 0) {
            byt = byt - 8;
            sign = "-";
        }
        byt = (10*(((byt) & (15)))) + (((byt) >> (4)));
        hh = YapiWrapper::ysprintf("%d",((byt) >> (2)));
        ss = YapiWrapper::ysprintf("%d",15*(((byt) & (3))));
        if ((int)(hh).length()<2) {
            hh = YapiWrapper::ysprintf("0%s",hh.c_str());
        }
        if ((int)(ss).length()<2) {
            ss = YapiWrapper::ysprintf("0%s",ss.c_str());
        }
        res = YapiWrapper::ysprintf("%s%s%s:%s", res.c_str(), sign.c_str(), hh.c_str(),ss.c_str());
    }
    return res;
}

int YSms::udataSize(void)
{
    int res = 0;
    int udhsize = 0;
    udhsize = (int)(_udh).size();
    res = (int)(_udata).size();
    if (_alphab == 0) {
        if (udhsize > 0) {
            res = res + (((8 + 8*udhsize + 6)) / (7));
        }
        res = (((res * 7 + 7)) / (8));
    } else {
        if (udhsize > 0) {
            res = res + 1 + udhsize;
        }
    }
    return res;
}

string YSms::encodeUserData(void)
{
    int udsize = 0;
    int udlen = 0;
    int udhsize = 0;
    int udhlen = 0;
    string res;
    int i = 0;
    int wpos = 0;
    int carry = 0;
    int nbits = 0;
    int thi_b = 0;
    // nbits = number of bits in carry
    udsize = this->udataSize();
    udhsize = (int)(_udh).size();
    udlen = (int)(_udata).size();
    res = string(1+udsize, (char)0);
    udhlen = 0;
    nbits = 0;
    carry = 0;
    // 1. Encode UDL
    if (_alphab == 0) {
        // 7-bit encoding
        if (udhsize > 0) {
            udhlen = (((8 + 8*udhsize + 6)) / (7));
            nbits = 7*udhlen - 8 - 8*udhsize;
        }
        res[0] = (char)(udhlen+udlen);
    } else {
        // 8-bit encoding
        res[0] = (char)(udsize);
    }
    // 2. Encode UDHL and UDL
    wpos = 1;
    if (udhsize > 0) {
        res[wpos] = (char)(udhsize);
        wpos = wpos + 1;
        i = 0;
        while (i < udhsize) {
            res[wpos] = (char)(((u8)_udh[i]));
            wpos = wpos + 1;
            i = i + 1;
        }
    }
    // 3. Encode UD
    if (_alphab == 0) {
        // 7-bit encoding
        i = 0;
        while (i < udlen) {
            if (nbits == 0) {
                carry = ((u8)_udata[i]);
                nbits = 7;
            } else {
                thi_b = ((u8)_udata[i]);
                res[wpos] = (char)(((carry) | ((((((thi_b) << (nbits)))) & (255)))));
                wpos = wpos + 1;
                nbits = nbits - 1;
                carry = ((thi_b) >> ((7 - nbits)));
            }
            i = i + 1;
        }
        if (nbits > 0) {
            res[wpos] = (char)(carry);
        }
    } else {
        // 8-bit encoding
        i = 0;
        while (i < udlen) {
            res[wpos] = (char)(((u8)_udata[i]));
            wpos = wpos + 1;
            i = i + 1;
        }
    }
    return res;
}

int YSms::generateParts(void)
{
    int udhsize = 0;
    int udlen = 0;
    int mss = 0;
    int partno = 0;
    int partlen = 0;
    string newud;
    string newudh;
    YSms newpdu;
    int i = 0;
    int wpos = 0;
    udhsize = (int)(_udh).size();
    udlen = (int)(_udata).size();
    mss = 140 - 1 - 5 - udhsize;
    if (_alphab == 0) {
        mss = (((mss * 8 - 6)) / (7));
    }
    _npdu = (((udlen+mss-1)) / (mss));
    _parts.clear();
    partno = 0;
    wpos = 0;
    while (wpos < udlen) {
        partno = partno + 1;
        newudh = string(5+udhsize, (char)0);
        newudh[0] = (char)(0);           // IEI: concatenated message
        newudh[1] = (char)(3);           // IEDL: 3 bytes
        newudh[2] = (char)(_mref);
        newudh[3] = (char)(_npdu);
        newudh[4] = (char)(partno);
        i = 0;
        while (i < udhsize) {
            newudh[5+i] = (char)(((u8)_udh[i]));
            i = i + 1;
        }
        if (wpos+mss < udlen) {
            partlen = mss;
        } else {
            partlen = udlen-wpos;
        }
        newud = string(partlen, (char)0);
        i = 0;
        while (i < partlen) {
            newud[i] = (char)(((u8)_udata[wpos]));
            wpos = wpos + 1;
            i = i + 1;
        }
        newpdu = YSms(_mbox);
        newpdu.set_received(this->isReceived());
        newpdu.set_smsc(this->get_smsc());
        newpdu.set_msgRef(this->get_msgRef());
        newpdu.set_sender(this->get_sender());
        newpdu.set_recipient(this->get_recipient());
        newpdu.set_protocolId(this->get_protocolId());
        newpdu.set_dcs(this->get_dcs());
        newpdu.set_timestamp(this->get_timestamp());
        newpdu.set_userDataHeader(newudh);
        newpdu.set_userData(newud);
        _parts.push_back(newpdu);
    }
    return YAPI_SUCCESS;
}

int YSms::generatePdu(void)
{
    string sca;
    string hdr;
    string addr;
    string stamp;
    string udata;
    int pdutyp = 0;
    int pdulen = 0;
    int i = 0;
    // Determine if the message can fit within a single PDU
    _parts.clear();
    if (this->udataSize() > 140) {
        // multiple PDU are needed
        _pdu = string(0, (char)0);
        return this->generateParts();
    }
    sca = this->encodeAddress(_smsc);
    if ((int)(sca).size() > 0) {
        sca[0] = (char)((int)(sca).size()-1);
    }
    stamp = this->encodeTimeStamp(_stamp);
    udata = this->encodeUserData();
    if (_deliv) {
        addr = this->encodeAddress(_orig);
        hdr = string(1, (char)0);
        pdutyp = 0;
    } else {
        addr = this->encodeAddress(_dest);
        _mref = _mbox->nextMsgRef();
        hdr = string(2, (char)0);
        hdr[1] = (char)(_mref);
        pdutyp = 1;
        if ((int)(stamp).size() > 0) {
            pdutyp = pdutyp + 16;
        }
        if ((int)(stamp).size() == 7) {
            pdutyp = pdutyp + 8;
        }
    }
    if ((int)(_udh).size() > 0) {
        pdutyp = pdutyp + 64;
    }
    hdr[0] = (char)(pdutyp);
    pdulen = (int)(sca).size()+(int)(hdr).size()+(int)(addr).size()+2+(int)(stamp).size()+(int)(udata).size();
    _pdu = string(pdulen, (char)0);
    pdulen = 0;
    i = 0;
    while (i < (int)(sca).size()) {
        _pdu[pdulen] = (char)(((u8)sca[i]));
        pdulen = pdulen + 1;
        i = i + 1;
    }
    i = 0;
    while (i < (int)(hdr).size()) {
        _pdu[pdulen] = (char)(((u8)hdr[i]));
        pdulen = pdulen + 1;
        i = i + 1;
    }
    i = 0;
    while (i < (int)(addr).size()) {
        _pdu[pdulen] = (char)(((u8)addr[i]));
        pdulen = pdulen + 1;
        i = i + 1;
    }
    _pdu[pdulen] = (char)(_pid);
    pdulen = pdulen + 1;
    _pdu[pdulen] = (char)(this->get_dcs());
    pdulen = pdulen + 1;
    i = 0;
    while (i < (int)(stamp).size()) {
        _pdu[pdulen] = (char)(((u8)stamp[i]));
        pdulen = pdulen + 1;
        i = i + 1;
    }
    i = 0;
    while (i < (int)(udata).size()) {
        _pdu[pdulen] = (char)(((u8)udata[i]));
        pdulen = pdulen + 1;
        i = i + 1;
    }
    _npdu = 1;
    return YAPI_SUCCESS;
}

int YSms::parseUserDataHeader(void)
{
    int udhlen = 0;
    int i = 0;
    int iei = 0;
    int ielen = 0;
    string sig;
    _aggSig = "";
    _aggIdx = 0;
    _aggCnt = 0;
    udhlen = (int)(_udh).size();
    i = 0;
    while (i+1 < udhlen) {
        iei = ((u8)_udh[i]);
        ielen = ((u8)_udh[i+1]);
        i = i + 2;
        if (i + ielen <= udhlen) {
            if ((iei == 0) && (ielen == 3)) {
                // concatenated SMS, 8-bit ref
                sig = YapiWrapper::ysprintf("%s-%s-%02x-%02x", _orig.c_str(), _dest.c_str(),
                _mref,((u8)_udh[i]));
                _aggSig = sig;
                _aggCnt = ((u8)_udh[i+1]);
                _aggIdx = ((u8)_udh[i+2]);
            }
            if ((iei == 8) && (ielen == 4)) {
                // concatenated SMS, 16-bit ref
                sig = YapiWrapper::ysprintf("%s-%s-%02x-%02x%02x", _orig.c_str(), _dest.c_str(),
                _mref, ((u8)_udh[i]),((u8)_udh[i+1]));
                _aggSig = sig;
                _aggCnt = ((u8)_udh[i+2]);
                _aggIdx = ((u8)_udh[i+3]);
            }
        }
        i = i + ielen;
    }
    return YAPI_SUCCESS;
}

int YSms::parsePdu(string pdu)
{
    int rpos = 0;
    int addrlen = 0;
    int pdutyp = 0;
    int tslen = 0;
    int dcs = 0;
    int udlen = 0;
    int udhsize = 0;
    int udhlen = 0;
    int i = 0;
    int carry = 0;
    int nbits = 0;
    int thi_b = 0;
    _pdu = pdu;
    _npdu = 1;
    // parse meta-data
    _smsc = this->decodeAddress(pdu, 1, 2*(((u8)pdu[0])-1));
    rpos = 1+((u8)pdu[0]);
    pdutyp = ((u8)pdu[rpos]);
    rpos = rpos + 1;
    _deliv = (((pdutyp) & (3)) == 0);
    if (_deliv) {
        addrlen = ((u8)pdu[rpos]);
        rpos = rpos + 1;
        _orig = this->decodeAddress(pdu, rpos, addrlen);
        _dest = "";
        tslen = 7;
    } else {
        _mref = ((u8)pdu[rpos]);
        rpos = rpos + 1;
        addrlen = ((u8)pdu[rpos]);
        rpos = rpos + 1;
        _dest = this->decodeAddress(pdu, rpos, addrlen);
        _orig = "";
        if ((((pdutyp) & (16))) != 0) {
            if ((((pdutyp) & (8))) != 0) {
                tslen = 7;
            } else {
                tslen= 1;
            }
        } else {
            tslen = 0;
        }
    }
    rpos = rpos + ((((addrlen+3)) >> (1)));
    _pid = ((u8)pdu[rpos]);
    rpos = rpos + 1;
    dcs = ((u8)pdu[rpos]);
    rpos = rpos + 1;
    _alphab = (((((dcs) >> (2)))) & (3));
    _mclass = ((dcs) & (16+3));
    _stamp = this->decodeTimeStamp(pdu, rpos, tslen);
    rpos = rpos + tslen;
    // parse user data (including udh)
    nbits = 0;
    carry = 0;
    udlen = ((u8)pdu[rpos]);
    rpos = rpos + 1;
    if (((pdutyp) & (64)) != 0) {
        udhsize = ((u8)pdu[rpos]);
        rpos = rpos + 1;
        _udh = string(udhsize, (char)0);
        i = 0;
        while (i < udhsize) {
            _udh[i] = (char)(((u8)pdu[rpos]));
            rpos = rpos + 1;
            i = i + 1;
        }
        if (_alphab == 0) {
            // 7-bit encoding
            udhlen = (((8 + 8*udhsize + 6)) / (7));
            nbits = 7*udhlen - 8 - 8*udhsize;
            if (nbits > 0) {
                thi_b = ((u8)pdu[rpos]);
                rpos = rpos + 1;
                carry = ((thi_b) >> (nbits));
                nbits = 8 - nbits;
            }
        } else {
            // byte encoding
            udhlen = 1+udhsize;
        }
        udlen = udlen - udhlen;
    } else {
        udhsize = 0;
        _udh = string(0, (char)0);
    }
    _udata = string(udlen, (char)0);
    if (_alphab == 0) {
        // 7-bit encoding
        i = 0;
        while (i < udlen) {
            if (nbits == 7) {
                _udata[i] = (char)(carry);
                carry = 0;
                nbits = 0;
            } else {
                thi_b = ((u8)pdu[rpos]);
                rpos = rpos + 1;
                _udata[i] = (char)(((carry) | ((((((thi_b) << (nbits)))) & (127)))));
                carry = ((thi_b) >> ((7 - nbits)));
                nbits = nbits + 1;
            }
            i = i + 1;
        }
    } else {
        // 8-bit encoding
        i = 0;
        while (i < udlen) {
            _udata[i] = (char)(((u8)pdu[rpos]));
            rpos = rpos + 1;
            i = i + 1;
        }
    }
    this->parseUserDataHeader();
    return YAPI_SUCCESS;
}

int YSms::send(void)
{
    int i = 0;
    int retcode = 0;
    YSms pdu;

    if (_npdu == 0) {
        this->generatePdu();
    }
    if (_npdu == 1) {
        return _mbox->_upload("sendSMS", _pdu);
    }
    retcode = YAPI_SUCCESS;
    i = 0;
    while ((i < _npdu) && (retcode == YAPI_SUCCESS)) {
        pdu = _parts[i];
        retcode= pdu.send();
        i = i + 1;
    }
    return retcode;
}

int YSms::deleteFromSIM(void)
{
    int i = 0;
    int retcode = 0;
    YSms pdu;

    if (_slot > 0) {
        return _mbox->clearSIMSlot(_slot);
    }
    retcode = YAPI_SUCCESS;
    i = 0;
    while ((i < _npdu) && (retcode == YAPI_SUCCESS)) {
        pdu = _parts[i];
        retcode= pdu.deleteFromSIM();
        i = i + 1;
    }
    return retcode;
}
//--- (end of generated code: YSms implementation)


YMessageBox::YMessageBox(const string& func): YFunction(func)
//--- (generated code: YMessageBox initialization)
    ,_slotsInUse(SLOTSINUSE_INVALID)
    ,_slotsCount(SLOTSCOUNT_INVALID)
    ,_slotsBitmap(SLOTSBITMAP_INVALID)
    ,_pduSent(PDUSENT_INVALID)
    ,_pduReceived(PDURECEIVED_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackMessageBox(NULL)
    ,_nextMsgRef(0)
    ,_gsm2unicodeReady(0)
//--- (end of generated code: YMessageBox initialization)
{
    _className="MessageBox";
}

YMessageBox::~YMessageBox()
{
//--- (generated code: YMessageBox cleanup)
//--- (end of generated code: YMessageBox cleanup)
}
//--- (generated code: YMessageBox implementation)
// static attributes
const string YMessageBox::SLOTSBITMAP_INVALID = YAPI_INVALID_STRING;
const string YMessageBox::COMMAND_INVALID = YAPI_INVALID_STRING;

int YMessageBox::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("slotsInUse")) {
        _slotsInUse =  json_val->getInt("slotsInUse");
    }
    if(json_val->has("slotsCount")) {
        _slotsCount =  json_val->getInt("slotsCount");
    }
    if(json_val->has("slotsBitmap")) {
        _slotsBitmap =  json_val->getString("slotsBitmap");
    }
    if(json_val->has("pduSent")) {
        _pduSent =  json_val->getInt("pduSent");
    }
    if(json_val->has("pduReceived")) {
        _pduReceived =  json_val->getInt("pduReceived");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Returns the number of message storage slots currently in use.
 *
 * @return an integer corresponding to the number of message storage slots currently in use
 *
 * On failure, throws an exception or returns Y_SLOTSINUSE_INVALID.
 */
int YMessageBox::get_slotsInUse(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMessageBox::SLOTSINUSE_INVALID;
                }
            }
        }
        res = _slotsInUse;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the total number of message storage slots on the SIM card.
 *
 * @return an integer corresponding to the total number of message storage slots on the SIM card
 *
 * On failure, throws an exception or returns Y_SLOTSCOUNT_INVALID.
 */
int YMessageBox::get_slotsCount(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMessageBox::SLOTSCOUNT_INVALID;
                }
            }
        }
        res = _slotsCount;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YMessageBox::get_slotsBitmap(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMessageBox::SLOTSBITMAP_INVALID;
                }
            }
        }
        res = _slotsBitmap;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of SMS units sent so far.
 *
 * @return an integer corresponding to the number of SMS units sent so far
 *
 * On failure, throws an exception or returns Y_PDUSENT_INVALID.
 */
int YMessageBox::get_pduSent(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMessageBox::PDUSENT_INVALID;
                }
            }
        }
        res = _pduSent;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the value of the outgoing SMS units counter.
 *
 * @param newval : an integer corresponding to the value of the outgoing SMS units counter
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YMessageBox::set_pduSent(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("pduSent", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the number of SMS units received so far.
 *
 * @return an integer corresponding to the number of SMS units received so far
 *
 * On failure, throws an exception or returns Y_PDURECEIVED_INVALID.
 */
int YMessageBox::get_pduReceived(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMessageBox::PDURECEIVED_INVALID;
                }
            }
        }
        res = _pduReceived;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the value of the incoming SMS units counter.
 *
 * @param newval : an integer corresponding to the value of the incoming SMS units counter
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YMessageBox::set_pduReceived(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("pduReceived", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YMessageBox::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YMessageBox::COMMAND_INVALID;
                }
            }
        }
        res = _command;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YMessageBox::set_command(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("command", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Retrieves a MessageBox interface for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the MessageBox interface is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YMessageBox.isOnline() to test if the MessageBox interface is
 * indeed online at a given time. In case of ambiguity when looking for
 * a MessageBox interface by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the MessageBox interface
 *
 * @return a YMessageBox object allowing you to drive the MessageBox interface.
 */
YMessageBox* YMessageBox::FindMessageBox(string func)
{
    YMessageBox* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YMessageBox*) YFunction::_FindFromCache("MessageBox", func);
        if (obj == NULL) {
            obj = new YMessageBox(func);
            YFunction::_AddToCache("MessageBox", func, obj);
        }
    } catch (std::exception) {
        if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
        throw;
    }
    if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
    return obj;
}

/**
 * Registers the callback function that is invoked on every change of advertised value.
 * The callback is invoked only during the execution of ySleep or yHandleEvents.
 * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
 * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
 *
 * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
 *         arguments: the function object of which the value has changed, and the character string describing
 *         the new advertised value.
 * @noreturn
 */
int YMessageBox::registerValueCallback(YMessageBoxValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackMessageBox = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YMessageBox::_invokeValueCallback(string value)
{
    if (_valueCallbackMessageBox != NULL) {
        _valueCallbackMessageBox(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YMessageBox::nextMsgRef(void)
{
    _nextMsgRef = _nextMsgRef + 1;
    return _nextMsgRef;
}

int YMessageBox::clearSIMSlot(int slot)
{
    _prevBitmapStr = "";
    return this->set_command(YapiWrapper::ysprintf("DS%d",slot));
}

YSms YMessageBox::fetchPdu(int slot)
{
    string binPdu;
    vector<string> arrPdu;
    string hexPdu;
    YSms sms;

    binPdu = this->_download(YapiWrapper::ysprintf("sms.json?pos=%d&len=1",slot));
    arrPdu = this->_json_get_array(binPdu);
    hexPdu = this->_decode_json_string(arrPdu[0]);
    sms = YSms(this);
    sms.set_slot(slot);
    sms.parsePdu(YAPI::_hexStr2Bin(hexPdu));
    return sms;
}

int YMessageBox::initGsm2Unicode(void)
{
    int i = 0;
    int uni = 0;
    _gsm2unicode.clear();
    // 00-07
    _gsm2unicode.push_back(64);
    _gsm2unicode.push_back(163);
    _gsm2unicode.push_back(36);
    _gsm2unicode.push_back(165);
    _gsm2unicode.push_back(232);
    _gsm2unicode.push_back(233);
    _gsm2unicode.push_back(249);
    _gsm2unicode.push_back(236);
    // 08-0F
    _gsm2unicode.push_back(242);
    _gsm2unicode.push_back(199);
    _gsm2unicode.push_back(10);
    _gsm2unicode.push_back(216);
    _gsm2unicode.push_back(248);
    _gsm2unicode.push_back(13);
    _gsm2unicode.push_back(197);
    _gsm2unicode.push_back(229);
    // 10-17
    _gsm2unicode.push_back(916);
    _gsm2unicode.push_back(95);
    _gsm2unicode.push_back(934);
    _gsm2unicode.push_back(915);
    _gsm2unicode.push_back(923);
    _gsm2unicode.push_back(937);
    _gsm2unicode.push_back(928);
    _gsm2unicode.push_back(936);
    // 18-1F
    _gsm2unicode.push_back(931);
    _gsm2unicode.push_back(920);
    _gsm2unicode.push_back(926);
    _gsm2unicode.push_back(27);
    _gsm2unicode.push_back(198);
    _gsm2unicode.push_back(230);
    _gsm2unicode.push_back(223);
    _gsm2unicode.push_back(201);
    // 20-7A
    i = 32;
    while (i <= 122) {
        _gsm2unicode.push_back(i);
        i = i + 1;
    }
    // exceptions in range 20-7A
    _gsm2unicode[36] = 164;
    _gsm2unicode[64] = 161;
    _gsm2unicode[91] = 196;
    _gsm2unicode[92] = 214;
    _gsm2unicode[93] = 209;
    _gsm2unicode[94] = 220;
    _gsm2unicode[95] = 167;
    _gsm2unicode[96] = 191;
    // 7B-7F
    _gsm2unicode.push_back(228);
    _gsm2unicode.push_back(246);
    _gsm2unicode.push_back(241);
    _gsm2unicode.push_back(252);
    _gsm2unicode.push_back(224);
    // Invert table as well wherever possible
    _iso2gsm = string(256, (char)0);
    i = 0;
    while (i <= 127) {
        uni = _gsm2unicode[i];
        if (uni <= 255) {
            _iso2gsm[uni] = (char)(i);
        }
        i = i + 1;
    }
    i = 0;
    while (i < 4) {
        // mark escape sequences
        _iso2gsm[91+i] = (char)(27);
        _iso2gsm[123+i] = (char)(27);
        i = i + 1;
    }
    // Done
    _gsm2unicodeReady = true;
    return YAPI_SUCCESS;
}

vector<int> YMessageBox::gsm2unicode(string gsm)
{
    int i = 0;
    int gsmlen = 0;
    int reslen = 0;
    vector<int> res;
    int uni = 0;
    if (!(_gsm2unicodeReady)) {
        this->initGsm2Unicode();
    }
    gsmlen = (int)(gsm).size();
    reslen = gsmlen;
    i = 0;
    while (i < gsmlen) {
        if (((u8)gsm[i]) == 27) {
            reslen = reslen - 1;
        }
        i = i + 1;
    }
    res.clear();
    i = 0;
    while (i < gsmlen) {
        uni = _gsm2unicode[((u8)gsm[i])];
        if ((uni == 27) && (i+1 < gsmlen)) {
            i = i + 1;
            uni = ((u8)gsm[i]);
            if (uni < 60) {
                if (uni < 41) {
                    if (uni==20) {
                        uni=94;
                    } else {
                        if (uni==40) {
                            uni=123;
                        } else {
                            uni=0;
                        }
                    }
                } else {
                    if (uni==41) {
                        uni=125;
                    } else {
                        if (uni==47) {
                            uni=92;
                        } else {
                            uni=0;
                        }
                    }
                }
            } else {
                if (uni < 62) {
                    if (uni==60) {
                        uni=91;
                    } else {
                        if (uni==61) {
                            uni=126;
                        } else {
                            uni=0;
                        }
                    }
                } else {
                    if (uni==62) {
                        uni=93;
                    } else {
                        if (uni==64) {
                            uni=124;
                        } else {
                            if (uni==101) {
                                uni=164;
                            } else {
                                uni=0;
                            }
                        }
                    }
                }
            }
        }
        if (uni > 0) {
            res.push_back(uni);
        }
        i = i + 1;
    }
    return res;
}

string YMessageBox::gsm2str(string gsm)
{
    int i = 0;
    int gsmlen = 0;
    int reslen = 0;
    string resbin;
    string resstr;
    int uni = 0;
    if (!(_gsm2unicodeReady)) {
        this->initGsm2Unicode();
    }
    gsmlen = (int)(gsm).size();
    reslen = gsmlen;
    i = 0;
    while (i < gsmlen) {
        if (((u8)gsm[i]) == 27) {
            reslen = reslen - 1;
        }
        i = i + 1;
    }
    resbin = string(reslen, (char)0);
    i = 0;
    reslen = 0;
    while (i < gsmlen) {
        uni = _gsm2unicode[((u8)gsm[i])];
        if ((uni == 27) && (i+1 < gsmlen)) {
            i = i + 1;
            uni = ((u8)gsm[i]);
            if (uni < 60) {
                if (uni < 41) {
                    if (uni==20) {
                        uni=94;
                    } else {
                        if (uni==40) {
                            uni=123;
                        } else {
                            uni=0;
                        }
                    }
                } else {
                    if (uni==41) {
                        uni=125;
                    } else {
                        if (uni==47) {
                            uni=92;
                        } else {
                            uni=0;
                        }
                    }
                }
            } else {
                if (uni < 62) {
                    if (uni==60) {
                        uni=91;
                    } else {
                        if (uni==61) {
                            uni=126;
                        } else {
                            uni=0;
                        }
                    }
                } else {
                    if (uni==62) {
                        uni=93;
                    } else {
                        if (uni==64) {
                            uni=124;
                        } else {
                            if (uni==101) {
                                uni=164;
                            } else {
                                uni=0;
                            }
                        }
                    }
                }
            }
        }
        if ((uni > 0) && (uni < 256)) {
            resbin[reslen] = (char)(uni);
            reslen = reslen + 1;
        }
        i = i + 1;
    }
    resstr = resbin;
    if ((int)(resstr).length() > reslen) {
        resstr = (resstr).substr(0, reslen);
    }
    return resstr;
}

string YMessageBox::str2gsm(string msg)
{
    string asc;
    int asclen = 0;
    int i = 0;
    int ch = 0;
    int gsm7 = 0;
    int extra = 0;
    string res;
    int wpos = 0;
    if (!(_gsm2unicodeReady)) {
        this->initGsm2Unicode();
    }
    asc = msg;
    asclen = (int)(asc).size();
    extra = 0;
    i = 0;
    while (i < asclen) {
        ch = ((u8)asc[i]);
        gsm7 = ((u8)_iso2gsm[ch]);
        if (gsm7 == 27) {
            extra = extra + 1;
        }
        if (gsm7 == 0) {
            // cannot use standard GSM encoding
            res = string(0, (char)0);
            return res;
        }
        i = i + 1;
    }
    res = string(asclen+extra, (char)0);
    wpos = 0;
    i = 0;
    while (i < asclen) {
        ch = ((u8)asc[i]);
        gsm7 = ((u8)_iso2gsm[ch]);
        res[wpos] = (char)(gsm7);
        wpos = wpos + 1;
        if (gsm7 == 27) {
            if (ch < 100) {
                if (ch<93) {
                    if (ch<92) {
                        gsm7=60;
                    } else {
                        gsm7=47;
                    }
                } else {
                    if (ch<94) {
                        gsm7=62;
                    } else {
                        gsm7=20;
                    }
                }
            } else {
                if (ch<125) {
                    if (ch<124) {
                        gsm7=40;
                    } else {
                        gsm7=64;
                    }
                } else {
                    if (ch<126) {
                        gsm7=41;
                    } else {
                        gsm7=61;
                    }
                }
            }
            res[wpos] = (char)(gsm7);
            wpos = wpos + 1;
        }
        i = i + 1;
    }
    return res;
}

int YMessageBox::checkNewMessages(void)
{
    string bitmapStr;
    string prevBitmap;
    string newBitmap;
    int slot = 0;
    int nslots = 0;
    int pduIdx = 0;
    int idx = 0;
    int bitVal = 0;
    int prevBit = 0;
    int i = 0;
    int nsig = 0;
    int cnt = 0;
    string sig;
    vector<YSms> newArr;
    vector<YSms> newMsg;
    vector<YSms> newAgg;
    vector<string> signatures;
    YSms sms;

    bitmapStr = this->get_slotsBitmap();
    if (bitmapStr == _prevBitmapStr) {
        return YAPI_SUCCESS;
    }
    prevBitmap = YAPI::_hexStr2Bin(_prevBitmapStr);
    newBitmap = YAPI::_hexStr2Bin(bitmapStr);
    _prevBitmapStr = bitmapStr;
    nslots = 8*(int)(newBitmap).size();
    newArr.clear();
    newMsg.clear();
    signatures.clear();
    nsig = 0;
    // copy known messages
    pduIdx = 0;
    while (pduIdx < (int)_pdus.size()) {
        sms = _pdus[pduIdx];
        slot = sms.get_slot();
        idx = ((slot) >> (3));
        if (idx < (int)(newBitmap).size()) {
            bitVal = ((1) << ((((slot) & (7)))));
            if ((((((u8)newBitmap[idx])) & (bitVal))) != 0) {
                newArr.push_back(sms);
                if (sms.get_concatCount() == 0) {
                    newMsg.push_back(sms);
                } else {
                    sig = sms.get_concatSignature();
                    i = 0;
                    while ((i < nsig) && ((int)(sig).length() > 0)) {
                        if (signatures[i] == sig) {
                            sig = "";
                        }
                        i = i + 1;
                    }
                    if ((int)(sig).length() > 0) {
                        signatures.push_back(sig);
                        nsig = nsig + 1;
                    }
                }
            }
        }
        pduIdx = pduIdx + 1;
    }
    // receive new messages
    slot = 0;
    while (slot < nslots) {
        idx = ((slot) >> (3));
        bitVal = ((1) << ((((slot) & (7)))));
        prevBit = 0;
        if (idx < (int)(prevBitmap).size()) {
            prevBit = ((((u8)prevBitmap[idx])) & (bitVal));
        }
        if ((((((u8)newBitmap[idx])) & (bitVal))) != 0) {
            if (prevBit == 0) {
                sms = this->fetchPdu(slot);
                newArr.push_back(sms);
                if (sms.get_concatCount() == 0) {
                    newMsg.push_back(sms);
                } else {
                    sig = sms.get_concatSignature();
                    i = 0;
                    while ((i < nsig) && ((int)(sig).length() > 0)) {
                        if (signatures[i] == sig) {
                            sig = "";
                        }
                        i = i + 1;
                    }
                    if ((int)(sig).length() > 0) {
                        signatures.push_back(sig);
                        nsig = nsig + 1;
                    }
                }
            }
        }
        slot = slot + 1;
    }
    _pdus = newArr;
    // append complete concatenated messages
    i = 0;
    while (i < nsig) {
        sig = signatures[i];
        cnt = 0;
        pduIdx = 0;
        while (pduIdx < (int)_pdus.size()) {
            sms = _pdus[pduIdx];
            if (sms.get_concatCount() > 0) {
                if (sms.get_concatSignature() == sig) {
                    if (cnt == 0) {
                        cnt = sms.get_concatCount();
                        newAgg.clear();
                    }
                    newAgg.push_back(sms);
                }
            }
            pduIdx = pduIdx + 1;
        }
        if ((cnt > 0) && ((int)newAgg.size() == cnt)) {
            sms = YSms(this);
            sms.set_parts(newAgg);
            newMsg.push_back(sms);
        }
        i = i + 1;
    }
    _messages = newMsg;
    return YAPI_SUCCESS;
}

vector<YSms> YMessageBox::get_pdus(void)
{
    this->checkNewMessages();
    return _pdus;
}

/**
 * Clear the SMS units counters.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YMessageBox::clearPduCounters(void)
{
    int retcode = 0;

    retcode = this->set_pduReceived(0);
    if (retcode != YAPI_SUCCESS) {
        return retcode;
    }
    retcode = this->set_pduSent(0);
    return retcode;
}

/**
 * Sends a regular text SMS, with standard parameters. This function can send messages
 * of more than 160 characters, using SMS concatenation. ISO-latin accented characters
 * are supported. For sending messages with special unicode characters such as asian
 * characters and emoticons, use newMessage to create a new message and define
 * the content of using methods addText and addUnicodeData.
 *
 * @param recipient : a text string with the recipient phone number, either as a
 *         national number, or in international format starting with a plus sign
 * @param message : the text to be sent in the message
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YMessageBox::sendTextMessage(string recipient,string message)
{
    YSms sms;

    sms = YSms(this);
    sms.set_recipient(recipient);
    sms.addText(message);
    return sms.send();
}

/**
 * Sends a Flash SMS (class 0 message). Flash messages are displayed on the handset
 * immediately and are usually not saved on the SIM card. This function can send messages
 * of more than 160 characters, using SMS concatenation. ISO-latin accented characters
 * are supported. For sending messages with special unicode characters such as asian
 * characters and emoticons, use newMessage to create a new message and define
 * the content of using methods addText et addUnicodeData.
 *
 * @param recipient : a text string with the recipient phone number, either as a
 *         national number, or in international format starting with a plus sign
 * @param message : the text to be sent in the message
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YMessageBox::sendFlashMessage(string recipient,string message)
{
    YSms sms;

    sms = YSms(this);
    sms.set_recipient(recipient);
    sms.set_msgClass(0);
    sms.addText(message);
    return sms.send();
}

/**
 * Creates a new empty SMS message, to be configured and sent later on.
 *
 * @param recipient : a text string with the recipient phone number, either as a
 *         national number, or in international format starting with a plus sign
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
YSms YMessageBox::newMessage(string recipient)
{
    YSms sms;
    sms = YSms(this);
    sms.set_recipient(recipient);
    return sms;
}

/**
 * Returns the list of messages received and not deleted. This function
 * will automatically decode concatenated SMS.
 *
 * @return an YSms object list.
 *
 * On failure, throws an exception or returns an empty list.
 */
vector<YSms> YMessageBox::get_messages(void)
{
    this->checkNewMessages();
    return _messages;
}

YMessageBox *YMessageBox::nextMessageBox(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YMessageBox::FindMessageBox(hwid);
}

YMessageBox* YMessageBox::FirstMessageBox(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("MessageBox", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YMessageBox::FindMessageBox(serial+"."+funcId);
}

//--- (end of generated code: YMessageBox implementation)

//--- (generated code: YMessageBox functions)
//--- (end of generated code: YMessageBox functions)
