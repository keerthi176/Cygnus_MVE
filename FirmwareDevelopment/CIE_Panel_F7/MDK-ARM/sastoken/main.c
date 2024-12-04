
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "azure_c_shared_utility/gballoc.h"
//#include "azure_c_shared_utility/sastoken.h"
//#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/sha.h"
#include "azure_c_shared_utility/hmacsha256.h"
//#include "azure_c_shared_utility/azure_base64.h"
//#include "azure_c_shared_utility/agenttime.h"
//#include "azure_c_shared_utility/strings.h"
//#include "azure_c_shared_utility/buffer_.h"
// 
//#include "azure_c_shared_utility/crt_abstractions.h"
int hmac(SHAversion whichSha, const unsigned char *text, int text_len,
    const unsigned char *key, int key_len,
    uint8_t digest[USHAMaxHashSize]);

char* SASToken( const char* key, const char* scope,  size_t expiry);
 
char* URLEnc( char* in );
char* Base64Dec( const char *inp, int inlen );
char* Base64Enc( char * inp, int inlen );
 
 
 
int main( void )
{
	char* x = SASToken( "S/4YuFcbUYoF7gGD7AR82OOg3T3UV3FDBcNFD58yVeE=", "smartnet.azure-devices.net%2Fdevices%2FMATTDEVICE1", 1654110813 );
	
	return (int) x;
}


#ifdef needed

static double getExpiryValue(const char* expiryASCII)
{
    double value = 0;
    size_t i = 0;
    for (i = 0; expiryASCII[i] != '\0'; i++)
    {
        if (expiryASCII[i] >= '0' && expiryASCII[i] <= '9')
        {
            value = value * 10 + (double)(expiryASCII[i] - '0');
        }
        else
        {
            value = 0;
            break;
        }
    }
    return value;
}


bool SASToken_Validate(char* sasToken)
{
    bool result;
    /*Codes_SRS_SASTOKEN_25_025: [**SASToken_Validate shall get the SASToken value by invoking STRING_c_str on the handle.**]***/
    const char* sasTokenArray = (const char*) sasToken;

    /* Codes_SRS_SASTOKEN_25_024: [**If handle is NULL then SASToken_Validate shall return false.**] */
    /* Codes_SRS_SASTOKEN_25_026: [**If STRING_c_str on handle return NULL then SASToken_Validate shall return false.**] */
    if (sasToken == NULL || sasTokenArray == NULL)
    {
        result = false;
    }
    else
    {
        int seStart = -1, seStop = -1;
        int srStart = -1, srStop = -1;
        int sigStart = -1, sigStop = -1;
        int tokenLength = (int)strlen(sasToken);
        int i;
        for (i = 0; i < tokenLength; i++)
        {
            if (sasTokenArray[i] == 's' && sasTokenArray[i + 1] == 'e' && sasTokenArray[i + 2] == '=') // Look for se=
            {
                seStart = i + 3;
                if (srStart > 0 && srStop < 0)
                {
                    if (sasTokenArray[i - 1] != '&' && sasTokenArray[i - 1] == ' ') // look for either & or space
                        srStop = i - 1;
                    else if (sasTokenArray[i - 1] == '&')
                        srStop = i - 2;
                    else
                        seStart = -1; // as the format is not either "&se=" or " se="
                }
                else if (sigStart > 0 && sigStop < 0)
                {
                    if (sasTokenArray[i - 1] != '&' && sasTokenArray[i - 1] == ' ')
                        sigStop = i - 1;
                    else if (sasTokenArray[i - 1] == '&')
                        sigStop = i - 2;
                    else
                        seStart = -1;
                }
            }
            else if (sasTokenArray[i] == 's' && sasTokenArray[i + 1] == 'r' && sasTokenArray[i + 2] == '=') // Look for sr=
            {
                srStart = i + 3;
                if (seStart > 0 && seStop < 0)
                {
                    if (sasTokenArray[i - 1] != '&' && sasTokenArray[i - 1] == ' ')
                        seStop = i - 1;
                    else if (sasTokenArray[i - 1] == '&')
                        seStop = i - 2;
                    else
                        srStart = -1;
                }
                else if (sigStart > 0 && sigStop < 0)
                {
                    if (sasTokenArray[i - 1] != '&' && sasTokenArray[i - 1] == ' ')
                        sigStop = i - 1;
                    else if (sasTokenArray[i - 1] == '&')
                        sigStop = i - 2;
                    else
                        srStart = -1;
                }
            }
            else if (sasTokenArray[i] == 's' && sasTokenArray[i + 1] == 'i' && sasTokenArray[i + 2] == 'g' && sasTokenArray[i + 3] == '=') // Look for sig=
            {
                sigStart = i + 4;
                if (srStart > 0 && srStop < 0)
                {
                    if (sasTokenArray[i - 1] != '&' && sasTokenArray[i - 1] == ' ')
                        srStop = i - 1;
                    else if (sasTokenArray[i - 1] == '&')
                        srStop = i - 2;
                    else
                        sigStart = -1;
                }
                else if (seStart > 0 && seStop < 0)
                {
                    if (sasTokenArray[i - 1] != '&' && sasTokenArray[i - 1] == ' ')
                        seStop = i - 1;
                    else if (sasTokenArray[i - 1] == '&')
                        seStop = i - 2;
                    else
                        sigStart = -1;
                }
            }
        }

        /*Codes_SRS_SASTOKEN_25_027: [**If SASTOKEN does not obey the SASToken format then SASToken_Validate shall return false.**]***/
        /*Codes_SRS_SASTOKEN_25_028: [**SASToken_validate shall check for the presence of sr, se and sig from the token and return false if not found**]***/
        if (seStart < 0 || srStart < 0 || sigStart < 0)
        {
            result = false;
        }
        else
        {
            if (seStop < 0)
            {
                seStop = tokenLength;
            }
            else if (srStop < 0)
            {
                srStop = tokenLength;
            }
            else if (sigStop < 0)
            {
                sigStop = tokenLength;
            }

            if ((seStop <= seStart) ||
                (srStop <= srStart) ||
                (sigStop <= sigStart))
            {
                result = false;
            }
            else
            {
                char* expiryASCII = (char*)malloc(seStop - seStart + 1);
                /*Codes_SRS_SASTOKEN_25_031: [**If malloc fails during validation then SASToken_Validate shall return false.**]***/
                if (expiryASCII == NULL)
                {
                    result = false;
                }
                else
                {
                    double expiry;
                    // Add the Null terminator here
                    memset(expiryASCII, 0, seStop - seStart + 1);
                    for (i = seStart; i < seStop; i++)
                    {
                        // The se contains the expiration values, if a & token is encountered then
                        // the se field is complete.
                        if (sasTokenArray[i] == '&')
                        {
                            break;
                        }
                        expiryASCII[i - seStart] = sasTokenArray[i];
                    }
                    expiry = getExpiryValue(expiryASCII);
                    /*Codes_SRS_SASTOKEN_25_029: [**SASToken_validate shall check for expiry time from token and if token has expired then would return false **]***/
                    if (expiry <= 0)
                    {
                        result = false;
                    }
                    else
                    {
                        double secSinceEpoch = get_difftime(get_time(NULL), (time_t)0);
                        if (expiry < secSinceEpoch)
                        {
                            /*Codes_SRS_SASTOKEN_25_029: [**SASToken_validate shall check for expiry time from token and if token has expired then would return false **]***/
                            result = false;
                        }
                        else
                        {
                            /*Codes_SRS_SASTOKEN_25_030: [**SASToken_validate shall return true only if the format is obeyed and the token has not yet expired **]***/
                            result = true;
                        }
                    }
                    free(expiryASCII);
                }
            }
        }
    }

    return result;
}
#endif



static char* construct_sas_token( const char* key, const char* scope, const char* keyname, size_t expiry )
{
	char* result;

	char tokenExpirationTime[32] = { 0 };

	char* decodedKey;

	/*Codes_SRS_SASTOKEN_06_029: [The key parameter is decoded from base64.]*/
	if ( ( decodedKey = Base64Dec( key, strlen( key ) ) ) == NULL )
	{
		/*Codes_SRS_SASTOKEN_06_030: [If there is an error in the decoding then SASToken_Create shall return NULL.]*/
	 
		result = NULL;
	}
	else
	{
		/*Codes_SRS_SASTOKEN_06_026: [If the conversion to string form fails for any reason then SASToken_Create shall return NULL.]*/
		snprintf( tokenExpirationTime, sizeof(tokenExpirationTime), "%d", expiry);
		 
	 
		char toBeHashed[ 512 ];

		char hash[ 512 ];
		char result_buff[ 512 ];

		result = result_buff;

		/*Codes_SRS_SASTOKEN_06_009: [The scope is the basis for creating a char*.]*/
		/*Codes_SRS_SASTOKEN_06_010: [A "\n" is appended to that string.]*/
		/*Codes_SRS_SASTOKEN_06_011: [tokenExpirationTime is appended to that string.]*/
		strcpy(toBeHashed, scope) ;
		strcat(toBeHashed, "\n");
		strcat(toBeHashed, tokenExpirationTime);

		char* base64Signature = NULL;
		char* urlEncodedSignature = NULL;
		size_t inLen = strlen(toBeHashed);
		const char* inBuf = (const char*) toBeHashed ;
		size_t outLen = strlen(decodedKey);
		char* outBuf = decodedKey;
		/*Codes_SRS_SASTOKEN_06_013: [If an error is returned from the HMAC256 function then NULL is returned from SASToken_Create.]*/
		/*Codes_SRS_SASTOKEN_06_012: [An HMAC256 hash is calculated using the decodedKey, over toBeHashed.]*/
		/*Codes_SRS_SASTOKEN_06_014: [If there are any errors from the following operations then NULL shall be returned.]*/
		/*Codes_SRS_SASTOKEN_06_015: [The hash is base 64 encoded.]*/
		/*Codes_SRS_SASTOKEN_06_028: [base64Signature shall be url encoded.]*/
		/*Codes_SRS_SASTOKEN_06_016: [The string "SharedAccessSignature sr=" is the first part of the result of SASToken_Create.]*/
		/*Codes_SRS_SASTOKEN_06_017: [The scope parameter is appended to result.]*/
		/*Codes_SRS_SASTOKEN_06_018: [The string "&sig=" is appended to result.]*/
		/*Codes_SRS_SASTOKEN_06_019: [The string urlEncodedSignature shall be appended to result.]*/
		/*Codes_SRS_SASTOKEN_06_020: [The string "&se=" shall be appended to result.]*/
		/*Codes_SRS_SASTOKEN_06_021: [tokenExpirationTime is appended to result.]*/
		/*Codes_SRS_SASTOKEN_06_022: [If keyName is non-NULL, the string "&skn=" is appended to result.]*/
		/*Codes_SRS_SASTOKEN_06_023: [If keyName is non-NULL, the argument keyName is appended to result.]*/
		int len = hmac( SHA256, (const unsigned char*)inBuf, inLen,  (const unsigned char*)outBuf, outLen, (uint8_t*) hash ) ;
		
		base64Signature = Base64Enc(hash, 32) ;
		urlEncodedSignature = URLEnc(base64Signature);

		strcpy( result, "SharedAccessSignature sr=" );
		strcat( result, scope);
		strcat( result, "&sig=");
		strcat( result, urlEncodedSignature );
		strcat( result, "&se=");
		strcat( result, tokenExpirationTime );
 
	}

	return result;
}

char* SASToken( const char* string_key, const char* string_scope, size_t expiry )
{
    char* result;

    /*Codes_SRS_SASTOKEN_06_001: [If key is NULL then SASToken_Create shall return NULL.]*/
    /*Codes_SRS_SASTOKEN_06_003: [If scope is NULL then SASToken_Create shall return NULL.]*/
    /*Codes_SRS_SASTOKEN_06_007: [keyName is optional and can be set to NULL.]*/
    if ((string_key == NULL) ||
        (string_scope == NULL))
    {
     //   LogError("Invalid Parameter to SASToken_Create. handle key: %p, handle scope: %p, handle keyName: %p", key, scope, keyName);
        result = NULL;
    }
    else
    {
        result = construct_sas_token(string_key, string_scope, NULL, expiry);
    }
    return result;
}
 