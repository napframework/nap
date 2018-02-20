/*********************************************************************
 *
 * $Id: yjson.c 29354 2017-11-29 17:09:16Z seb $
 *
 * Simple JSON parser (actually a slightly enhanced lexer)
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
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
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
#define __FILE_ID__  "yjson"
#include <string.h>
#include "yjson.h"

#ifdef MICROCHIP_API
#define atoi      parseUInt
#else
#include <stdio.h>
#include <stdlib.h>
#ifdef WINDOWS_API
#if defined(__BORLANDC__)
#pragma warn -8019
#include <windows.h>
#pragma warn +8019
#else
#include <windows.h>
#endif
#endif
#endif

#ifdef DEBUG_JSON_PARSE
const char* yJsonStateStr[] = {
    "YJSON_HTTP_START",       // about to parse HTTP header, up to first space before return code
    "YJSON_HTTP_READ_CODE",   // reading HTTP return code
    "YJSON_HTTP_READ_MSG",    // reading HTTP return message
    "YJSON_HTTP_SKIP",        // skipping rest of HTTP header until double-CRLF
    "YJSON_START",            // about to parse JSON reply
    "YJSON_PARSE_ANY",        // parsing anything to come
    "YJSON_PARSE_SYMBOL",     // parsing a symbol (boolean)
    "YJSON_PARSE_NUM",        // parsing a number
    "YJSON_PARSE_STRING",     // parsing a quoted string
    "YJSON_PARSE_STRINGQ",    // parsing a quoted string, within quoted character
    "YJSON_PARSE_STRINGCONT", // parsing the continuation of a quoted string
    "YJSON_PARSE_STRINGCONTQ",// parsing the continuation of a quoted string, within quoted character
    "YJSON_PARSE_ARRAY",      // parsing an unnamed array
    "YJSON_PARSE_STRUCT",     // parsing a named structure
    "YJSON_PARSE_MEMBSTART",  // parsing a structure member (before name)
    "YJSON_PARSE_MEMBNAME",   // parsing a structure member name
    "YJSON_PARSE_MEMBCOL",    // parsing the colon between member name and value
    "YJSON_PARSE_DONE",       // parse completed, end of input data (or end of container)
    "YJSON_PARSE_ERROR"       // dead end, parse error encountered
};
#endif

yJsonRetCode yJsonParse(yJsonStateMachine *j)
{
    yJsonRetCode    res;
    yJsonState      st = j->st;
    _FAR const char *src = j->src;
    _FAR const char *end = j->end;
    char            *pt = j->pt;
    char            *ept = j->token + sizeof(j->token) - 1;
    unsigned char   c=0;

skip:
    res = YJSON_NEED_INPUT;
    if(st == YJSON_HTTP_START || st == YJSON_START || st == YJSON_PARSE_ANY) {
        j->next = YJSON_PARSE_SPECIAL;
    } else if(j->next != YJSON_PARSE_SPECIAL) {
        st = j->next;
        j->next = YJSON_PARSE_SPECIAL;
    }

    while(1) {
        switch(st) {
            case YJSON_HTTP_START:       // about to parse HTTP header, up to first space before return code
                while(src < end && (c = *src) != ' ' && c != 'O' && c != '\r') src++;
                if(src >= end) goto done;
                if(c == ' ') src++; // skip space
                pt = j->token;
                st = YJSON_HTTP_READ_CODE;
                // fall through
            case YJSON_HTTP_READ_CODE:   // reading HTTP return code
                while(src < end && pt < ept && (c = *src) != ' ' && c != 'O' && c != '\r') {
                    if(c < '0' || c > '9') goto push_error;
                    *pt++ = c;
                    src++;
                }
                if(src >= end) goto done;
                if(pt >= ept) goto push_error;
                if(c == ' ') src++; // skip space
                else if(c == 'O' && pt == j->token) {
                    // Handle short status "OK" as "HTTP/1.1 200 OK"
                    *pt++ = '2';*pt++ = '0';*pt++ = '0';
                }
                *pt = 0;
                pt = j->token;
                j->next = YJSON_HTTP_READ_MSG;
                res = YJSON_PARSE_AVAIL;
                goto done;
            case YJSON_HTTP_READ_MSG:    // reading HTTP return message
                while(src < end && pt < ept && (c = *src) != '\r' && c != '\n') {
                    *pt++ = c;
                    src++;
                }
                if(src >= end) goto done;
                *pt = 0;
                pt = j->token;
                j->next = YJSON_HTTP_SKIP;
                res = YJSON_PARSE_AVAIL;
                goto done;
            case YJSON_HTTP_SKIP:        // skipping rest of HTTP header until double-CRLF
                while(src < end && pt < j->token+2) {
                    c = *src++;
                    if(c == '\n') *pt++ = '\n';
                    else if(c != '\r') pt = j->token;
                }
                if(src >= end) goto done;
                st = YJSON_START;
                // fall through
            case YJSON_START:            // about to parse JSON reply
                j->depth = 0;
                j->skipcnt = 0;
                j->skipdepth = YJSON_MAX_DEPTH;
                // fall through
            case YJSON_PARSE_ANY:        // parsing anything to come
                while(src < end && ((c = *src) == ' ' || c == '\r' || c == '\n')) src++;
                if(src >= end) goto done;
                pt = j->token;
                if(c == '{') {
#ifndef YAPI_IN_YDEVICE
                    j->state_start = src;
#endif
                    src++; st = YJSON_PARSE_STRUCT;

                }
                else if(c == '[') {
#ifndef YAPI_IN_YDEVICE
                    j->state_start = src;
#endif
                    src++; st = YJSON_PARSE_ARRAY;
                }
                else if(c == '"') {
#ifndef YAPI_IN_YDEVICE
                    j->state_start = src;
#endif
                    src++; st = YJSON_PARSE_STRING;
                }
                else if(c=='-' || (c >= '0' && c <= '9')) {
#ifndef YAPI_IN_YDEVICE
                    j->state_start = src;
#endif
                    st = YJSON_PARSE_NUM;
                }
                else if(c >= 'A' && c <= 'z') {
#ifndef YAPI_IN_YDEVICE
                    j->state_start = src;
#endif
                    st = YJSON_PARSE_SYMBOL;
                }
                else if(j->depth > 0 && c == ']') { st = YJSON_PARSE_DONE; }
                else goto push_error;
                // continue into the selected state
                continue;
            case YJSON_PARSE_SYMBOL:     // parsing a symbol (boolean)
                while(src < end && pt < ept && (c = *src) >= 'A' && c <= 'z') {
                    *pt++ = c;
                    src++;
                }
                if(src >= end) goto done;
                if(pt >= ept) goto push_error;
            token_done:
                *pt = 0;
                j->next = YJSON_PARSE_DONE;
#ifndef YAPI_IN_YDEVICE
                j->state_end = src;
#endif
                res = YJSON_PARSE_AVAIL;
                goto done;
            case YJSON_PARSE_NUM:        // parsing a number
                while(src < end && pt < ept && ( (c = *src)=='-' || (c >= '0' && c <= '9') ))  {
                    *pt++ = c;
                    src++;
                }
                if(src >= end) goto done;
                if(pt >= ept) goto push_error;
                goto token_done;
            case YJSON_PARSE_STRING:     // parsing a quoted string
            case YJSON_PARSE_STRINGCONT: // parsing the continuation of a quoted string
                while(src < end && pt < ept && (c = *src) != '"' && c != '\\') {
                    *pt++ = c;
                    src++;
                }
                if(src >= end) goto done;
                if(pt >= ept) {
                    *pt = 0;
                    pt = j->token;
                    j->next = YJSON_PARSE_STRINGCONT;
                    res = YJSON_PARSE_AVAIL;
                    goto done;
                }
                src++; // skip double-quote or backslash
                if(c == '"') goto token_done;
                if (st == YJSON_PARSE_STRING) {
                    st = YJSON_PARSE_STRINGQ;
                } else {
                    st = YJSON_PARSE_STRINGCONTQ;
                }
                // fall through
            case YJSON_PARSE_STRINGQ:    // parsing a quoted string, within quoted character
            case YJSON_PARSE_STRINGCONTQ:// parsing the continuation of a quoted string, within quoted character
                if(src >= end) goto done;
                c = *src++;
                switch(c) {
                    case 'r': *pt++ = '\r'; break;
                    case 'n': *pt++ = '\n'; break;
                    case 't': *pt++ = '\t'; break;
                    default: *pt++ = c;
                }
                if (st == YJSON_PARSE_STRINGQ) {
                    st = YJSON_PARSE_STRING;
                } else {
                    st = YJSON_PARSE_STRINGCONT;
                }
                // continue string parsing;
                continue;
            case YJSON_PARSE_ARRAY:      // parsing an unnamed array
                while(src < end && (*src == ' ' || *src == '\r' || *src == '\n')) src++;
                if(src >= end) goto done;
                if(*src == ']') {
                    j->next = YJSON_PARSE_DONE;
                }else{
                    j->next = YJSON_PARSE_ANY;
                }
                c = '[';
                goto nest;
            case YJSON_PARSE_STRUCT:     // parsing a named structure
                j->next = YJSON_PARSE_MEMBSTART;
                c = '{';
            nest:
                if(j->depth >= YJSON_MAX_DEPTH) goto push_error;
                j->stack[j->depth++] = st;
                *pt++ = c;
                *pt = 0;
                res = YJSON_PARSE_AVAIL;
                goto done;
            case YJSON_PARSE_MEMBSTART:  // parsing a structure member (before name)
                while(src < end && ((c = *src) == ' ' || c == '\r' || c == '\n')) src++;
                if(src >= end) goto done;
                if(c == '}') {
                    st = YJSON_PARSE_DONE;
                    continue;
                }
                if(c != '"') goto push_error;
                src++;
                pt = j->token;
                st = YJSON_PARSE_MEMBNAME;
                // fall through
            case YJSON_PARSE_MEMBNAME:   // parsing a structure member name
                while(src < end && pt < ept && (c = *src) != '"') {
                    *pt++ = c;
                    src++;
                }
                if(src >= end) goto done;
                if(pt >= ept) goto push_error;
                src++;
                *pt = 0;
                j->next = YJSON_PARSE_MEMBCOL;
                res = YJSON_PARSE_AVAIL;
                goto done;
            case YJSON_PARSE_MEMBCOL:    // parsing the colon between member name and value
                while(src < end && ((c = *src) == ' ' || c == '\r' || c == '\n')) src++;
                if(src >= end) goto done;
                if(c != ':') goto push_error;
                src++;
                st = YJSON_PARSE_ANY;
                continue; // continue parse
            case YJSON_PARSE_DONE:       // parse completed, end of input data
                while(src < end && ((c = *src) == ' ' || c == '\r' || c == '\n')) src++;
                pt = j->token;
                if(j->depth > 0) {
                    if(src >= end) goto done;
                    if(j->stack[j->depth-1] == YJSON_PARSE_STRUCT) {
                        if(c == ',') { src++; st = YJSON_PARSE_MEMBSTART; }
                        else if(c == '}') goto un_nest;
                        else goto push_error;
                    } else { // YJSON_PARSE_ARRAY
                        if(c == ',') { src++; st = YJSON_PARSE_ANY; }
                        else if(c == ']') {
            un_nest:
                            *pt++ = *src++;
                            st = j->stack[--(j->depth)];
                            goto token_done;
                        }
                        else goto push_error;
                    }
                    continue; // continue to parse nested block
                }
                if(src < end) goto push_error; // unexpected content at end of JSON data
                *pt = 0;
                res = YJSON_SUCCESS;
                goto done;
                // save current state if possible, and trigger an error
            push_error:
                if(j->depth < YJSON_MAX_DEPTH)
                    j->stack[j->depth++] = st;
                st = YJSON_PARSE_ERROR;
                // fall through
            case YJSON_PARSE_ERROR:      // dead end, parse error encountered
                res = YJSON_FAILED;
                goto done;
            case YJSON_PARSE_SPECIAL:    // should never happen
                res = YJSON_FAILED;
                goto done;
        }
    }
done:
    if(st >= YJSON_START  && res == YJSON_PARSE_AVAIL) {
        if(j->skipdepth <= j->depth) {
            if(j->skipdepth == j->depth) {
                j->skipdepth = YJSON_MAX_DEPTH;
            }
            goto skip;
        }
        if(j->skipcnt > 0) {
            if(st == YJSON_PARSE_STRUCT || st == YJSON_PARSE_ARRAY) {
                j->skipdepth = j->depth-1;
            }
            j->skipcnt--;
            goto skip;
        }
    }
    j->st = st;
    j->src = src;
    j->pt = pt;
    return res;
}

void yJsonSkip(yJsonStateMachine *j, int nitems)
{
    j->skipcnt += nitems;
}

#if 0
void yJsonInitEx(yJsonStateMachineEx *j, const char *jzon, int jzon_len, const char *ref, int ref_len)
{

    memset(j, 0, sizeof(yJsonStateMachineEx));
    j->jzon.src = jzon;
    j->jzon.end = jzon + jzon_len;
    j->jzon.st = YJSON_START;
    if (ref) {
        j->ref.src = ref;
        j->ref.end = ref + ref_len;
        j->ref.st = YJSON_START;
    }
    j->sst = JZON_PARSE_SYNCRO;
}


yJsonRetCode yJsonParseEx(yJsonStateMachineEx *j)
{
    yJsonRetCode  ref_res;
    if (j->ref.src == NULL) {
        ref_res = yJsonParse(&j->jzon);
        j->st = j->jzon.st;
        j->next = j->jzon.next;
        memcpy(j->token, j->jzon.token, sizeof(j->token));
        return ref_res;
    }

    switch (j->sst) {
    case JZON_PARSE_SYNCRO:
        ref_res = yJsonParse(&j->ref);
        if (ref_res != YJSON_PARSE_AVAIL) {
            return ref_res;
        }
        ref_res = yJsonParse(&j->jzon);
        if (ref_res != YJSON_PARSE_AVAIL) {
            return ref_res;
        }
        switch (j->ref.st) {
        case YJSON_PARSE_STRUCT:
            if (j->jzon.st == YJSON_PARSE_ARRAY) {
                j->sst = JZON_PARSE_ONLY_REF;
            }
            j->sst_stack[j->depth++] = j->sst;
            // no break on purpose
        case YJSON_PARSE_MEMBNAME:
            j->st = j->ref.st;
            j->next = j->ref.next;
            memcpy(j->token, j->ref.token, sizeof(j->token));
            break;
        case YJSON_PARSE_STRING:
            // skip value with potential continuation
            while (j->ref.next == YJSON_PARSE_STRINGCONT && yJsonParse(&j->ref) == YJSON_PARSE_AVAIL);
            if (j->jzon.next == YJSON_PARSE_STRINGCONT) {
                j->sst = JZON_PARSE_ONLY_YZON;
            }
            //no break on purpose
        default:
            j->st = j->jzon.st;
            j->next = j->jzon.next;
            memcpy(j->token, j->jzon.token, sizeof(j->token));
            if (j->next == YJSON_PARSE_DONE) {
                j->sst = j->sst_stack[j->depth - 1];
            }
            break;
        }
        break;
    case JZON_PARSE_ONLY_REF:
        ref_res = yJsonParse(&j->ref);
        if (ref_res != YJSON_PARSE_AVAIL) {
            return -1;
        }
        j->st = j->ref.st;
        j->next = j->ref.next;
        memcpy(j->token, j->ref.token, sizeof(j->token));
        if (j->ref.st == YJSON_PARSE_MEMBNAME) {
            j->sst = JZON_PARSE_SYNCRO;
        }
        break;
    case JZON_PARSE_ONLY_YZON:
        ref_res = yJsonParse(&j->jzon);
        if (ref_res != YJSON_PARSE_AVAIL) {
            return -1;
        }
        j->st = j->jzon.st;
        j->next = j->jzon.next;
        memcpy(j->token, j->jzon.token, sizeof(j->token));
        if (j->jzon.next != YJSON_PARSE_STRINGCONT) {
            j->sst = JZON_PARSE_SYNCRO;
        }
        break;
    }

    return YJSON_PARSE_AVAIL;
}


void yJsonSkipEx(yJsonStateMachineEx *j, int nitems)
{
    if (j->ref.src == NULL) {
        j->jzon.skipcnt += nitems;
        return;
    }
    switch (j->sst) {
    case JZON_PARSE_SYNCRO:
        j->ref.skipcnt += nitems;
        j->jzon.skipcnt += nitems;
        break;

    case JZON_PARSE_ONLY_REF:
        j->ref.skipcnt += nitems;
        break;
    case JZON_PARSE_ONLY_YZON:
        j->jzon.skipcnt += nitems;
        break;
    }
}
#endif

